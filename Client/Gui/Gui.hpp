#pragma once
#include "../Network/Network.hpp"
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx9.h>
#include <d3d9.h>

namespace Gui
{
	const ImVec2 ScreenSize = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };
	const ImVec2 AppSize = { 450.f, 380.f };

	// Window data
	inline WNDCLASSEX Class;
	inline HWND Hwnd;
	inline bool FinishLoop = false;

	// Dx Data
	static LPDIRECT3D9 DxD3D;
	static LPDIRECT3DDEVICE9 DxDevice;
	static D3DPRESENT_PARAMETERS DxPP;

	// Custom functions
    void Render();
    void Uninitialize();
    bool Run();

	void ResetDeviceD3D();
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
}