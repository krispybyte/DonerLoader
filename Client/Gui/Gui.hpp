#pragma once
#include "../Network/Network.hpp"
#include <imgui.h>
#include <d3d9.h>

namespace Gui
{
	// Loader ui data
	extern bool ShouldRun;

	extern bool Streamed;

	extern char Username[17]; // Maximum of 16 characters
	extern char Password[33]; // Maximum of 32 characters

	extern bool RememberMe;

	extern int SelectedModule;

	// Screen data
	extern const ImVec2 ScreenSize;

	// Window data
	extern HWND Hwnd;
	extern WNDCLASSEX Class;
	extern ImVec2 Size;
	extern ImVec2 Position;

	// Dx Data
	extern PDIRECT3D9 DxD3D;
	extern LPDIRECT3DDEVICE9 DxDevice;
	extern D3DPRESENT_PARAMETERS DxPresentParameters;

	void Run();

	extern inline void CreateWnd(const char* WindowName);
	void ClearWnd();

	bool CreateDevice();
	void ResetDevice();
	void ClearDevice();

	void CreateImGui();
	void ClearImGui();

	void BeginRender();
	void EndRender();
	extern inline void Render();
}