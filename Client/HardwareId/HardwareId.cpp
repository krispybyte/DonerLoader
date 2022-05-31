#include "HardwareId.hpp"
#include "WmiQuery/WmiQuery.hpp"

/*
	Cpu
*/

std::string HardwareId::Cpu::Name()
{
	return Wmi::Query(L"SELECT Name FROM Win32_Processor", L"Name");
}

std::string HardwareId::Cpu::Id()
{
	return Wmi::Query(L"SELECT ProcessorId FROM Win32_Processor", L"ProcessorId");
}

unsigned long HardwareId::Cpu::CoreCount()
{
	SYSTEM_INFO SystemInformation;
	GetSystemInfo(&SystemInformation);
	return SystemInformation.dwNumberOfProcessors;
}

/*
	Gpu
*/

std::string HardwareId::Gpu::Name()
{
	return Wmi::Query(L"SELECT Name FROM Win32_VideoController", L"Name");
}

/*
	Ram
*/

std::string HardwareId::Ram::Serial()
{
	return Wmi::Query(L"SELECT SerialNumber FROM Win32_PhysicalMemory", L"SerialNumber");
}

std::string HardwareId::Ram::Manufacturer()
{
	return Wmi::Query(L"SELECT Manufacturer FROM Win32_PhysicalMemory", L"Manufacturer");
}

/*
	Disk
*/

std::string HardwareId::Disk::Serial()
{
	return Wmi::Query(L"SELECT VolumeSerialNumber FROM Win32_LogicalDisk", L"VolumeSerialNumber");
}