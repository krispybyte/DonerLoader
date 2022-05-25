#include "Gui.hpp"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx9.h>

bool LoggedIn = false;
bool Streamed = false;

void Gui::Render()
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize(Size);

	ImGui::Begin("Client", &ShouldRun, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
	{
		switch (Network::ClientState)
		{
			case Network::ClientStates::IdleState:
			{
				if (!LoggedIn)
				{
					if (ImGui::Button("Login", { 80, 32 }))
					{
						Network::ClientState = Network::ClientStates::LoginState;
						LoggedIn = true;
					}

					break;
				}

				ImGui::Text("Logged in, welcome!");

				if (!Streamed)
				{
					if (ImGui::Button("Stream", { 80, 32 }))
					{
						Network::ClientState = Network::ClientStates::ModuleState;
						Streamed = true;
					}

					break;
				}

				ImGui::Text("Successfully streamed!");

				if (ImGui::Button("Exit", { 80, 32 }))
				{
					ShouldRun = false;
				}

				break;
			}
			case Network::ClientStates::InitializeState:
			{
				ImGui::Text("Exchanging keys...");
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
				ShouldRun = false;
				return;
			}
		}

		ImGui::End();
	}
}

bool Gui::Run()
{
	// Create
	CreateWnd("Client");
	CreateDevice();
	CreateImGui();

	// Run
	while (ShouldRun)
	{
		BeginRender();
		Render();
		EndRender();
	}

	// Destroy
	ClearImGui();
	ClearDevice();
	ClearWnd();

	return true;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND Hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

long __stdcall WindowProcess(HWND window, UINT message, WPARAM wideParameter, LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
		case WM_SIZE:
		{
			if (Gui::DxDevice && wideParameter != SIZE_MINIMIZED)
			{
				Gui::DxPresentParameters.BackBufferWidth = LOWORD(longParameter);
				Gui::DxPresentParameters.BackBufferHeight = HIWORD(longParameter);
				Gui::ResetDevice();
			}

			return 0;
		}
		case WM_SYSCOMMAND:
		{
			if ((wideParameter & 0xfff0) == SC_KEYMENU)
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
		case WM_LBUTTONDOWN:
		{
			Gui::Position = MAKEPOINTS(longParameter);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			if (wideParameter == MK_LBUTTON)
			{
				const POINTS Points = MAKEPOINTS(longParameter);
				RECT Rect;

				GetWindowRect(Gui::Hwnd, &Rect);

				Rect.left += Points.x - Gui::Position.x;
				Rect.top += Points.y - Gui::Position.y;

				if (Gui::Position.x >= 0 && Gui::Position.x <= Gui::Size.x && Gui::Position.y >= 0 && Gui::Position.y <= 18)
				{
					SetWindowPos(Gui::Hwnd, HWND_TOPMOST, Rect.left, Rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			return 0;
		}
	}

	return DefWindowProcA(window, message, wideParameter, longParameter);
}

void Gui::CreateWnd(const char* windowName)
{
	Class = { sizeof(WNDCLASSEX), CS_CLASSDC, reinterpret_cast<WNDPROC>(WindowProcess), 0, 0, GetModuleHandleA(0), 0, 0, 0, 0, "ClientWnd", 0 };

	RegisterClassExA(&Class);

	Hwnd = CreateWindowExA(0, "ClientWnd", windowName, WS_POPUP, Position.x, Position.y, Size.x, Size.y, 0, 0, Class.hInstance, 0);

	ShowWindow(Hwnd, SW_SHOWDEFAULT);
	UpdateWindow(Hwnd);
}

void Gui::ClearWnd()
{
	DestroyWindow(Hwnd);
	UnregisterClassA(Class.lpszClassName, Class.hInstance);
}

bool Gui::CreateDevice()
{
	DxD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!DxD3D)
	{
		return false;
	}

	RtlZeroMemory(&DxPresentParameters, sizeof(DxPresentParameters));

	DxPresentParameters.Windowed = true;
	DxPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	DxPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	DxPresentParameters.EnableAutoDepthStencil = true;
	DxPresentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	DxPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	const bool Success = DxD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &DxPresentParameters, &DxDevice) < 0;
	return Success;
}

void Gui::ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const HRESULT Result = DxDevice->Reset(&DxPresentParameters);

	if (Result == D3DERR_INVALIDCALL)
	{
		IM_ASSERT(0);
	}

	ImGui_ImplDX9_CreateDeviceObjects();
}

void Gui::ClearDevice()
{
	if (DxDevice)
	{
		DxDevice->Release();
		DxDevice = nullptr;
	}

	if (DxD3D)
	{
		DxD3D->Release();
		DxD3D = nullptr;
	}
}

void Gui::CreateImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& Io = ImGui::GetIO();

	Io.IniFilename = nullptr;

	ImGui::StyleColorsClassic();

	ImGui_ImplWin32_Init(Hwnd);
	ImGui_ImplDX9_Init(DxDevice);
}

void Gui::ClearImGui()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Gui::BeginRender()
{
	MSG Msg;
	while (PeekMessageA(&Msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);

		if (Msg.message == WM_QUIT)
		{
			ShouldRun = false;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Gui::EndRender()
{
	ImGui::EndFrame();

	DxDevice->SetRenderState(D3DRS_ZENABLE, false);
	DxDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	DxDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	DxDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.f, 0);

	if (DxDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		DxDevice->EndScene();
	}

	const HRESULT Result = DxDevice->Present(0, 0, 0, 0);

	// Handle resetting of the DX device
	if (Result == D3DERR_DEVICELOST && DxDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ResetDevice();
	}
}