#include "../../SDK/SDK.h"

#include "../Features/CFG.h"

MAKE_HOOK(
	CPrediction_RunCommand, Memory::GetVFunc(I::Prediction, 17),
	void, __fastcall, CPrediction* ecx, void* edx, C_BasePlayer* player, CUserCmd* pCmd, IMoveHelper* moveHelper)
{
	if (Shifting::bRecharging)
	{
		if (const auto pLocal = H::Entities->GetLocal())
		{
			if (player == pLocal)
				return;
		}
	}

	CALL_ORIGINAL(ecx, edx, player, pCmd, moveHelper);

	I::MoveHelper = moveHelper;

	if (const auto pLocal = H::Entities->GetLocal())
	{
		//credits: KGB
		if (CFG::Misc_Accuracy_Improvements && !pLocal->InCond(TF_COND_HALLOWEEN_KART) && !Shifting::bRecharging)
		{
			if (!pCmd->hasbeenpredicted && player == pLocal)
			{
				if (const auto pAnimState = pLocal->GetAnimState())
				{
					const float flOldFrameTime = I::GlobalVars->frametime;
					I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;
					pAnimState->Update(G::bStartedFakeTaunt ? G::flFakeTauntStartYaw : pCmd->viewangles.y, pCmd->viewangles.x);
					pLocal->FrameAdvance(I::GlobalVars->frametime);
					I::GlobalVars->frametime = flOldFrameTime;
				}
			}
		}
	}
}
