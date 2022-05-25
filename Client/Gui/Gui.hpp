#pragma once
#include "../Network/Network.hpp"
#include <imgui.h>
#include <d3d9.h>

namespace Gui
{
	inline bool ShouldRun = true;

	// Screen data
	inline ImVec2 ScreenSize = { static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) };

	// Window data
	inline HWND Hwnd;
	inline WNDCLASSEX Class;
	inline ImVec2 Size = { 220, 105 };
	inline POINTS Position = { ScreenSize.x / 2.f - Size.x / 2.f, ScreenSize.y / 2.f - Size.y / 2.f };

	// Dx Data
	inline PDIRECT3D9 DxD3D;
	inline LPDIRECT3DDEVICE9 DxDevice;
	inline D3DPRESENT_PARAMETERS DxPresentParameters;

	bool Run();

	void CreateWnd(const char* Name);
	void ClearWnd();

	bool CreateDevice();
	void ResetDevice();
	void ClearDevice();

	void CreateImGui();
	void ClearImGui();

	void BeginRender();
	void EndRender();
	void Render();
}