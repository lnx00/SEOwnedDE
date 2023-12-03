#pragma once

#include "../../../SDK/SDK.h"

class CPaint
{
	IMaterial *m_pMatGlowColor = nullptr, *m_pMatHaloAddToScreen = nullptr;
	ITexture *m_pRtFullFrame = nullptr, *m_pRenderBuffer0 = nullptr, *m_pRenderBuffer1 = nullptr;
	IMaterial *m_pMatBlurX = nullptr, *m_pMatBlurY = nullptr;
	IMaterialVar* m_pBloomAmount = nullptr;

	void Initialize();

	struct PaintRecord_t
	{
		Vec3 Position = {};
		float TimeAdded = 0.0f;
	};

	std::map<int, std::vector<PaintRecord_t>> m_mapPositions = {};

public:
	void Run();
	void CleanUp();
};

MAKE_SINGLETON_SCOPED(CPaint, Paint, F);
