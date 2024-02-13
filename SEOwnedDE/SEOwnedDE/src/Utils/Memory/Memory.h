#pragma once

#include <Windows.h>

#include <cstdint>

namespace Memory
{
	std::uintptr_t FindSignature(const char *szModule, const char *szPattern);
	PVOID FindInterface(const char *szModule, const char *szObject);

	inline void *GetVFunc(void *pBaseClass, unsigned int unIndex)
	{
		return reinterpret_cast<void *>((*static_cast<int **>(pBaseClass))[unIndex]);
	}

	inline std::uintptr_t RelToAbs(const std::uintptr_t address)
	{
		return *reinterpret_cast<std::int32_t*>(address + 0x3) + address + 0x7;
	}
}