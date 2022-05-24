#pragma once
#include <unordered_map>

namespace Modules
{
	typedef std::vector<uint8_t> Module;

	// Module Id (int), Module data (vector)
	inline std::unordered_map<int, Module> List;

	bool Load(const int Id, const std::string_view& FilePath);
}