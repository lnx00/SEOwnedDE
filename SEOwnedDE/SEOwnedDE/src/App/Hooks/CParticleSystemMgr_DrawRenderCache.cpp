#include "../../SDK/SDK.h"

#include "../Features/Materials/Materials.h"
#include "../Features/Outlines/Outlines.h"

bool isDrawingWorld = false;

MAKE_HOOK(
	CBaseWorldView_DrawExecute, Signatures::CBaseWorldView_DrawExecute.Get(),
	void, __fastcall, void* ecx, void* edx, float waterHeight, view_id_t viewID, float waterZAdjust)
{
	isDrawingWorld = true;
	CALL_ORIGINAL(ecx, edx, waterHeight, viewID, waterZAdjust);
	isDrawingWorld = false;
}

MAKE_HOOK(
	CParticleSystemMgr_DrawRenderCache, Signatures::CParticleSystemMgr_DrawRenderCache.Get(),
	void, __fastcall, void* ecx, void* edx, bool bShadowDepth)
{
	if (isDrawingWorld)
	{
		if (const auto rc = I::MaterialSystem->GetRenderContext())
		{
			rc->ClearBuffers(false, false, true);
		}

		F::Materials->Run();
		F::Outlines->RunModels();
	}

	CALL_ORIGINAL(ecx, edx, bShadowDepth);
}
