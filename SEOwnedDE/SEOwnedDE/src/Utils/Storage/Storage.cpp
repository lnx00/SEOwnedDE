#include "Storage.h"
#include <Windows.h>
#include <shlobj.h>

#include "../Assert/Assert.h"

std::optional<std::filesystem::path> GetLocalAppdata()
{
	PWSTR buffer = nullptr;
	const auto result = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &buffer);

	if (SUCCEEDED(result))
	{
		std::filesystem::path folderPath(buffer);
		if (exists(folderPath))
		{
			CoTaskMemFree(buffer);
			return folderPath;
		}
	}

	CoTaskMemFree(buffer);
	return std::nullopt;
}

bool AssureDirectory(const std::filesystem::path& path)
{
	if (!exists(path))
	{
		return create_directories(path);
	}

	return true;
}

void CStorage::Init(const std::string& folderName)
{
	const auto rootPath = GetLocalAppdata();
	if (!rootPath.has_value())
	{
		MessageBoxA(nullptr, "Failed to initialize storage path!", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	m_WorkFolder = *rootPath / folderName;
	Assert(!AssureDirectory(m_WorkFolder))

	m_ConfigFolder = m_WorkFolder / "Configs";
	Assert(!AssureDirectory(m_ConfigFolder))
}
