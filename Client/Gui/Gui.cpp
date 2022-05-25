#include "Gui.hpp"

bool LoggedIn = false;
bool Streamed = false;

void Gui::Render()
{
    ImGui::SetNextWindowPos({ -1, -1 });
    ImGui::SetNextWindowSize(AppSize);
    if (ImGui::Begin("Client", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
    {
        switch (Network::ClientState)
        {
            case Network::ClientStates::IdleState:
            {
                if (!LoggedIn)
                {
                    if (ImGui::Button("Login", { AppSize.x - 31, 22 }))
                    {
                        Network::ClientState = Network::ClientStates::LoginState;
                        LoggedIn = true;
                    }

                    break;
                }

                ImGui::Text("Logged in, welcome!");

                if (!Streamed)
                {
                    if (ImGui::Button("Stream content", { AppSize.x - 31, 22 }))
                    {
                        Network::ClientState = Network::ClientStates::ModuleState;
                        Streamed = true;
                    }

                    break;
                }

                ImGui::Text("Successfully streamed!");

                if (ImGui::Button("Exit", { AppSize.x - 31, 22 }))
                {
                    FinishLoop = true;
                }

                break;
            }
            case Network::ClientStates::InitializeState:
            {
                ImGui::Text("Initializing...");
                break;
            }
            case Network::ClientStates::LoginState:
            {
                ImGui::Text("Logging you in...");
                break;
            }
            case Network::ClientStates::ModuleState:
            {
                ImGui::Text("Streaming module...");
                break;
            }
            default:
            {
                MessageBoxA(nullptr, "Invalid state.", "Error (55)", MB_ICONERROR | MB_OK);
                FinishLoop = true;
                return;
            }
        }

        ImGui::End();
    }
}

bool Gui::Run()
{
    Class = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr, "Client", nullptr };
    RegisterClassExA(&Class);
    Hwnd = CreateWindowA(Class.lpszClassName, "Client", WS_OVERLAPPEDWINDOW, ScreenSize.x / 2 - AppSize.x / 2, ScreenSize.y / 2 - AppSize.y / 2, AppSize.x, AppSize.y, nullptr, nullptr, Class.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(Hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassA(Class.lpszClassName, Class.hInstance);
        return false;
    }

    // Show the window
    ShowWindow(Hwnd, SW_SHOWDEFAULT);
    UpdateWindow(Hwnd);

    // Setup Dear ImGui context
    ImGui::CreateContext();
    ImGuiIO& Io = ImGui::GetIO(); (void)Io;

    // Enable Keyboard Controls
    Io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(Hwnd);
    ImGui_ImplDX9_Init(DxDevice);

    // Our state
    ImVec4 ClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    if (FinishLoop)
    {
        return true;
    }

    // Main loop
    while (!FinishLoop)
    {
        MSG Msg;

        while (PeekMessageA(&Msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessageA(&Msg);

            // Handle application exitting
            if (Msg.message == WM_QUIT)
            {
                FinishLoop = true;
            }
        }

        // Begin the ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Render our GUI
        Gui::Render();

        // Frame rendering
        ImGui::EndFrame();

        DxDevice->SetRenderState(D3DRS_ZENABLE, false);
        DxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
        DxDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

        const D3DCOLOR ClearColorDx = D3DCOLOR_RGBA((int)(ClearColor.x * ClearColor.w * 255.0f), (int)(ClearColor.y * ClearColor.w * 255.0f), (int)(ClearColor.z * ClearColor.w * 255.0f), (int)(ClearColor.w * 255.0f));
        DxDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, ClearColorDx, 1.0f, 0);

        if (DxDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            DxDevice->EndScene();
        }

        const HRESULT Result = DxDevice->Present(0, 0, 0, 0);

        // Handle loss of D3D9 device
        if (Result == D3DERR_DEVICELOST && DxDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        {
            ResetDeviceD3D();
        }
    }

    Gui::Uninitialize();

    return false;
}
void Gui::Uninitialize()
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(Hwnd);
    UnregisterClassA(Class.lpszClassName, Class.hInstance);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI Gui::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    switch (msg)
    {
        case WM_SIZE:
        {
            if (DxDevice && wParam != SIZE_MINIMIZED)
            {
                DxPP.BackBufferWidth = LOWORD(lParam);
                DxPP.BackBufferHeight = HIWORD(lParam);
                ResetDeviceD3D();
            }

            return 0;
        }
        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            {
                return 0;
            }

            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}
bool Gui::CreateDeviceD3D(HWND hWnd)
{
    if ((DxD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
    {
        return false;
    }

    // Create the D3DDevice
    RtlZeroMemory(&DxPP, sizeof(DxPP));
    DxPP.Windowed = TRUE;
    DxPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
    DxPP.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    DxPP.EnableAutoDepthStencil = TRUE;
    DxPP.AutoDepthStencilFormat = D3DFMT_D16;
    DxPP.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (DxD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &DxPP, &DxDevice) < 0)
    {
        return false;
    }

    return true;
}
void Gui::CleanupDeviceD3D()
{
    if (DxDevice) { DxDevice->Release(); DxDevice = nullptr; }
    if (DxD3D) { DxD3D->Release(); DxD3D = nullptr; }
}
void Gui::ResetDeviceD3D()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();

    const HRESULT Result = DxDevice->Reset(&DxPP);
    if (Result == D3DERR_INVALIDCALL)
    {
        IM_ASSERT(0);
    }

    ImGui_ImplDX9_CreateDeviceObjects();
}