#pragma once

#include "../../SDK/SDK.h"

namespace Hooks::WINAPI_WndProc
{
	inline HWND hwWindow;
	inline WNDPROC Original;
	LRESULT __stdcall Func(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Init();
	void Release();
}
