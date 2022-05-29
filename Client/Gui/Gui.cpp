#include "Gui.hpp"
#include "Fonts/Fonts.hpp"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx9.h>

void Gui::Render()
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize(Size);

	ImGui::Begin("Client", &ShouldRun, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	{
		ImGui::GetWindowDrawList()->AddRectFilled({ 0, 0 }, { Size.x, Size.y }, ImGui::GetColorU32(ImGuiCol_TitleBg));

		switch (Network::ClientState)
		{
			case Network::ClientStates::IdleState:
			{
				if (!Network::SuccessfulLogin)
				{
					ImGui::PushItemWidth(160);

					ImGui::SetCursorPos({ Size.x / 2 - 160 / 2, Size.y / 3 });
					ImGui::InputText("Username", Username, IM_ARRAYSIZE(Username));

					ImGui::SetCursorPos({ Size.x / 2 - 160 / 2, Size.y / 2.4f });
					ImGui::InputText("Password", Password, IM_ARRAYSIZE(Password), ImGuiInputTextFlags_Password);

					ImGui::SetCursorPos({ Size.x / 2 - 160 / 2, Size.y / 2 });
					ImGui::Checkbox("Remember Me", &RememberMe);

					ImGui::SetCursorPos({ Size.x / 2 - 160 / 2, Size.y / 1.7f });
					if (ImGui::Button("Login", { 160, 46 }))
					{
						Network::ClientState = Network::ClientStates::LoginState;
					}

					ImGui::PopItemWidth();
					break;
				}

				if (!Streamed)
				{
					static const std::string WelcomeMessage = "Logged in, welcome " + std::string(Username) + "!";
					ImGui::SetCursorPos({ Size.x / 2 - ImGui::CalcTextSize(WelcomeMessage.c_str()).x / 2, Size.y / 5 });
					ImGui::Text(WelcomeMessage.c_str());

					ImGui::SetCursorPos({ Size.x / 2 - ImGui::CalcTextSize("Please choose a module to load").x / 2, Size.y / 4 });
					ImGui::Text("Please choose a module to load");

					static const char* AvailableModules[] { "CS:GO", "Rust", "Escape from Tarkov" };
					ImGui::SetCursorPos({ Size.x / 2 - 160 / 2, Size.y / 3.2f });
					ImGui::SetNextItemWidth(160);
					ImGui::ListBox("##Modules", &SelectedModule, AvailableModules, IM_ARRAYSIZE(AvailableModules), 4);

					ImGui::SetCursorPos({ Size.x / 2 - 160 / 2, Size.y / 1.7f });
					if (ImGui::Button("Load", { 160, 46 }))
					{
						Network::ClientState = Network::ClientStates::ModuleState;
						Streamed = true;
					}

					break;
				}

				ImGui::SetCursorPos({ Size.x / 2 - ImGui::CalcTextSize("Successfully loaded!").x / 2, Size.y / 2.4f });
				ImGui::Text("Successfully loaded!");

				ImGui::SetCursorPos({ Size.x / 2 - 160 / 2, Size.y / 2 });
				if (ImGui::Button("Exit", { 160, 46 }))
				{
					ShouldRun = false;
				}

				break;
			}
			case Network::ClientStates::InitializeState:
			{
				ImGui::SetCursorPos({ Size.x / 2 - ImGui::CalcTextSize("Initializing resources...").x / 2, Size.y / 2.4f });
				ImGui::Text("Initializing resources...");
				break;
			}
			case Network::ClientStates::LoginState:
			{
				ImGui::SetCursorPos({ Size.x / 2 - ImGui::CalcTextSize("Logging in...").x / 2, Size.y / 2.4f });
				ImGui::Text("Logging in...");
				break;
			}
			case Network::ClientStates::ModuleState:
			{
				ImGui::SetCursorPos({ Size.x / 2 - ImGui::CalcTextSize("Loading module...").x / 2, Size.y / 2.4f });
				ImGui::Text("Loading module...");
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

void Gui::Run()
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
			const POINTS Points = MAKEPOINTS(longParameter);
			Gui::Position = { (float)Points.x, (float)Points.y };
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

				if (Gui::Position.x >= 0 && Gui::Position.x <= Gui::Size.x && Gui::Position.y >= 0 && Gui::Position.y <= 20)
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

	// Disalbe .ini file generation
	Io.IniFilename = nullptr;

	// Setup ImGui style
	ImGui::StyleColorsDark();

	// Setup custom fonts
	if (!Fonts::Setup())
	{
		ShouldRun = false;
	}

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