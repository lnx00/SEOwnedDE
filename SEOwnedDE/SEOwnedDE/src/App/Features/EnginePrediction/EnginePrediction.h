#pragma once

#include "../../../SDK/SDK.h"

class CEnginePrediction
{
	CMoveData m_MoveData = {};
	float m_fOldCurrentTime = 0.0f;
	float m_fOldFrameTime = 0.0f;
	int m_nOldTickCount = 0;

	int GetTickbase(CUserCmd* pCmd, C_TFPlayer* pLocal);

public:
	int flags{};

	void Start(CUserCmd* pCmd);
	void End();
};

MAKE_SINGLETON_SCOPED(CEnginePrediction, EnginePrediction, F)
