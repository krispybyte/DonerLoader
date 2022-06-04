#include "Modules.hpp"
#include "../Utilities/Utilities.hpp"

bool Modules::Load(const std::size_t Id, const std::string_view& FilePath)
{
	return Utilities::ReadFile(FilePath, Modules::List[Id]);
}