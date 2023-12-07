#include "StickyJump.h"

#include "../CFG.h"
#include "../MovementSimulation/MovementSimulation.h"

void CStickyJump::Run(CUserCmd* cmd)
{
	// Should run?
	if (!H::Input->IsDown(CFG::Misc_Auto_Air_Pogo_Key)
		|| I::EngineVGui->IsGameUIVisible()
		|| I::MatSystemSurface->IsCursorVisible()
		|| SDKUtils::BInEndOfMatch())
	{
		return;
	}

	// Valid player?
	const auto local = H::Entities->GetLocal();
	if (!local || local->deadflag() || (local->m_fFlags() & FL_ONGROUND) || !local->InCond(TF_COND_BLASTJUMPING))
		return;

	// Sticky launcher equipped?
	const auto weapon = H::Entities->GetWeapon();
	if (!weapon || weapon->GetWeaponID() != TF_WEAPON_PIPEBOMBLAUNCHER)
		return;

	/* Begin: Movement simulation */
	if (!F::MovementSimulation->Initialize(local))
		return;

	const int simTicks = TIME_TO_TICKS(SDKUtils::AttribHookValue(0.8f, "sticky_arm_time", local));
	for (int n = 0; n < simTicks; n++)
	{
		F::MovementSimulation->RunTick();
	}

	const auto end = F::MovementSimulation->GetOrigin();
	F::MovementSimulation->Restore();
	/* End: Movement simulation */

	if (G::bFiring)
	{
		G::bSilentAngles = true;

		float pitchOffset = Math::RemapValClamped(I::EngineClient->GetViewAngles().x, 0.0f, -25.0f, 0.0f, -4.0f);
		if (!(local->m_fFlags() & FL_DUCKING))
		{
			pitchOffset = -3.0f;
		}

		cmd->viewangles = Math::CalcAngle(local->GetShootPos(), end) + Vec3(pitchOffset, 0.0f, 0.0f);
	}

	// Shoot a sticky
	if (H::Entities->GetGroup(EEntGroup::PROJECTILES_LOCAL_STICKIES).empty() && local->m_vecVelocity().z < 500.0f)
	{
		if (weapon->As<C_TFPipebombLauncher>()->m_flChargeBeginTime() > 0.0f)
		{
			cmd->buttons &= ~IN_ATTACK;

			//I::EngineClient->ClientCmd_Unrestricted("impulse 101");
		}

		else
		{
			cmd->buttons |= IN_ATTACK;
		}
	}

	for (const auto ent : H::Entities->GetGroup(EEntGroup::PROJECTILES_LOCAL_STICKIES))
	{
		if (!ent)
			continue;

		const auto sticky = ent->As<C_TFGrenadePipebombProjectile>();
		if (!sticky || sticky->m_bTouched())
			continue;

		cmd->forwardmove *= 0.05f;
		cmd->sidemove *= 0.05f;

		Vec3 stickyVel{};
		sticky->EstimateAbsVelocity(stickyVel);

		// We're near the sticky, trigger it!
		if (sticky->m_vecOrigin().DistTo(local->m_vecOrigin()) < 350.0f && local->m_vecVelocity().Length2D() > (stickyVel.Length2D() * 1.05f))
		{
			cmd->buttons |= IN_ATTACK2;
		}

		auto slowdownSpeed = Math::RemapValClamped(I::EngineClient->GetViewAngles().x, 0.0f, -25.0f, 880.0f, 840.0f);

		if (!(local->m_fFlags() & FL_DUCKING))
		{
			slowdownSpeed *= 1.01f;
		}

		// Move to the sticky
		if (local->m_vecVelocity().Length2D() > slowdownSpeed)
		{
			Vec3 forward{};
			Math::AngleVectors(Math::VelocityToAngles({ local->m_vecVelocity().x, local->m_vecVelocity().y, 0.0f }), &forward);

			SDKUtils::WalkTo(cmd, local->m_vecOrigin(), local->m_vecOrigin() + ((forward * -1.0f) * 100.0f), 1.0f);
		}

		break;
	}
}
