#pragma once
#include <filesystem>

#include "../Singleton/Singleton.h"

class CStorage
{
	std::filesystem::path m_WorkFolder;
	std::filesystem::path m_ConfigFolder;

public:
	void Init(const std::string& folderName);

	const std::filesystem::path& GetWorkFolder()
	{
		return m_WorkFolder;
	}

	const std::filesystem::path& GetConfigFolder()
	{
		return m_ConfigFolder;
	}
};

MAKE_SINGLETON_SCOPED(CStorage, Storage, U);