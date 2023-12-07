#pragma once

#include "../../../SDK/SDK.h"

class CWorldModulation
{
	bool m_bWorldWasModulated = false;

	void ApplyModulation(const Color_t& clr, bool world, bool sky);

public:
	void UpdateWorldModulation();
	void RestoreWorldModulation();

	void LevelShutdown() { m_bWorldWasModulated = false; }
};

MAKE_SINGLETON_SCOPED(CWorldModulation, WorldModulation, F)
