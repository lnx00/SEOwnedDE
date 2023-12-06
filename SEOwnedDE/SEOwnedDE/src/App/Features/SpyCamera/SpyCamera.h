#pragma once

#include "../../../SDK/SDK.h"

class CSpyCamera
{
	bool m_IsRendering = false;
	CViewSetup m_ViewSetup = {};

	void Drag();

public:
	void Run();

	bool IsRendering() { return m_IsRendering; }
	void UpdateViewSetup(const CViewSetup& viewSetup) { m_ViewSetup = viewSetup; }
};

MAKE_SINGLETON_SCOPED(CSpyCamera, SpyCamera, F);
