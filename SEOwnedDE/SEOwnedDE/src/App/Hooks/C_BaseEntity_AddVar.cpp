#include "../../SDK/SDK.h"

#include "../Features/CFG.h"

MAKE_HOOK(
	C_BaseEntity_AddVar, Signatures::C_BaseEntity_AddVar.Get(),
	void, __fastcall, C_BaseEntity* ecx, void* edx, void* data, IInterpolatedVar* watcher, int type, bool bSetup)
{
	if (CFG::Misc_Accuracy_Improvements && watcher)
	{
		const auto hash = HASH_RT(watcher->GetDebugName());

		static constexpr auto m_iv_vecVelocity = HASH_CT("C_BaseEntity::m_iv_vecVelocity");
		static constexpr auto m_iv_angEyeAngles = HASH_CT("C_TFPlayer::m_iv_angEyeAngles");
		static constexpr auto m_iv_flPoseParameter = HASH_CT("C_BaseAnimating::m_iv_flPoseParameter");
		static constexpr auto m_iv_flCycle = HASH_CT("C_BaseAnimating::m_iv_flCycle");
		static constexpr auto m_iv_flMaxGroundSpeed = HASH_CT("CMultiPlayerAnimState::m_iv_flMaxGroundSpeed");

		if (hash == m_iv_vecVelocity
			|| hash == m_iv_flPoseParameter
			|| hash == m_iv_flCycle
			|| hash == m_iv_flMaxGroundSpeed)
			return;

		if (ecx != H::Entities->GetLocal())
		{
			if (hash == m_iv_angEyeAngles)
				return;
		}
	}

	CALL_ORIGINAL(ecx, edx, data, watcher, type, bSetup);
}

MAKE_HOOK(
	C_BaseEntity_EstimateAbsVelocity, Signatures::C_BaseEntity_EstimateAbsVelocity.Get(),
	void, __fastcall, C_BaseEntity* ecx, void* edx, Vector& vel)
{
	if (CFG::Misc_Accuracy_Improvements && ecx)
	{
		if (ecx->GetClassId() == ETFClassIds::CTFPlayer)
		{
			if (const auto pPlayer = ecx->As<C_TFPlayer>())
			{
				vel = pPlayer->m_vecVelocity();
				return;
			}
		}
	}

	CALL_ORIGINAL(ecx, edx, vel);
}
