#include "App/App.h"

DWORD WINAPI MainThread(LPVOID lpParam)
{
	while (!GetModuleHandleA("XAudio2_7.dll"))
	{
		Sleep(500);
	}

	App->Start();

	while (!GetAsyncKeyState(VK_F11))
	{
		Sleep(500);
	}

	App->Shutdown();

	Sleep(500);

	FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_SUCCESS);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (const auto hMainThread = CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr))
		{
			CloseHandle(hMainThread);
		}
	}

	return TRUE;
}