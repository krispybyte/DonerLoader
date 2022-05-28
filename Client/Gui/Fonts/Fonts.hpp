#pragma once
#include "SegoeUi.hpp"
#include <imgui.h>

namespace Fonts
{
	ImFont* SegoeUi = nullptr;

	bool Setup()
	{
		// ImGui font instance, we can use this
		// to add new fonts and use them.
		ImFontAtlas* Fonts = ImGui::GetIO().Fonts;

		// Disable ImGui from managing the memory.
		// Letting ImGui manage the memory causes
		// a crash when freeing the font's memory.
		ImFontConfig FontConfiguration;
		FontConfiguration.FontDataOwnedByAtlas = false;

		// Add all fonts
		if (!SegoeUi) SegoeUi = Fonts->AddFontFromMemoryTTF(&Fonts::Memory::SegoeUi, sizeof(Fonts::Memory::SegoeUi), 17.5f, &FontConfiguration);

		return SegoeUi;
	}
}