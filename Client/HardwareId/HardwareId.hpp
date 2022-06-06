#pragma once
#include <string>

namespace HardwareId::Cpu
{
	std::string Name();
	std::string Id();
	unsigned long CoreCount();
}

namespace HardwareId::Gpu
{
	std::string Name();
}

namespace HardwareId::Ram
{
	std::string Serial();
	std::string Manufacturer();
}

namespace HardwareId::Disk
{
	std::string Serial();
}