#include "../../SDK/SDK.h"

#include "../Features/CFG.h"
#include "../Features/SeedPred/SeedPred.h"

MAKE_HOOK(
	FX_FireBullets, Signatures::FX_FireBullets.Get(),
	void, __cdecl, void* pWpn, int iPlayer, const Vector& vecOrigin, const QAngle& vecAngles,
	int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
{
	if (CFG::Exploits_SeedPred_Active)
	{
		iSeed = F::SeedPred->GetSeed();
	}

	CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
}
