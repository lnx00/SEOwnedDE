#include "../../SDK/SDK.h"

#include "../Features/CFG.h"

//Hello I am TF2 developer and today we're going to shitcode

MAKE_HOOK(
	CTFPlayer_MaybeDrawRailgunBeam, Signatures::CTFPlayer_MaybeDrawRailgunBeam.Get(),
	void, __fastcall, C_TFPlayer* ecx, void* edx, void* pFilter, C_TFWeaponBase* pWeapon, const Vector& vStartPos, const Vector& vEndPos)
{
	if (CFG::Visuals_Tracer_Type)
	{
		if (const auto pLocal = H::Entities->GetLocal())
		{
			if (ecx == pLocal)
				return;
		}
	}

	CALL_ORIGINAL(ecx, edx, pFilter, pWeapon, vStartPos, vEndPos);
}
