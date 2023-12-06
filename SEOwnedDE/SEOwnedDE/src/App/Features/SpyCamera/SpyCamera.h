#pragma once

#include "../../../SDK/SDK.h"

class CSpyCamera
{
	bool m_IsRendering = false;

	void Drag();

public:
	void Run();
	bool IsRendering() { return m_IsRendering; }
};

MAKE_SINGLETON_SCOPED(CSpyCamera, SpyCamera, F);
