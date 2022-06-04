#pragma once
#include <unordered_map>

namespace Modules
{
	typedef std::vector<uint8_t> Module;

	// Module Id (int), Module data (vector)
	inline std::unordered_map<std::size_t, Module> List;

	bool Load(const std::size_t Id, const std::string_view& FilePath);
}