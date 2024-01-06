#include "../../SDK/SDK.h"

#include "../Features/CFG.h"
#include "../Features/VisualUtils/VisualUtils.h"
#include "../Features/ProjectileSim//ProjectileSim.h"
#include "../Features/SpyCamera/SpyCamera.h"

void RenderLine(const Vector& v1, const Vector& v2, Color_t c, bool bZBuffer)
{
	reinterpret_cast<void(__cdecl *)(const Vector&, const Vector&, Color_t, bool)>
		(Signatures::RenderLine.Get())(v1, v2, c, bZBuffer);
}

void RenderBox(const Vector& origin, const QAngle& angles, const Vector& mins, const Vector& maxs, Color_t c, bool bZBuffer, bool bInsideOut = false)
{
	reinterpret_cast<void(__cdecl *)(const Vector&, const QAngle&, const Vector&, const Vector&, Color_t c, bool, bool)>
		(Memory::RelToAbs(Signatures::RenderBox.Get()))(origin, angles, mins, maxs, c, bZBuffer, bInsideOut);
}

void RenderWireframeBox(const Vector& vOrigin, const QAngle& angles, const Vector& vMins, const Vector& vMaxs, Color_t c, bool bZBuffer)
{
	reinterpret_cast<void(__cdecl *)(const Vector&, const QAngle&, const Vector&, const Vector&, Color_t c, bool)>
		(Signatures::RenderWireframeBox.Get())(vOrigin, angles, vMins, vMaxs, c, bZBuffer);
}

void SniperLines()
{
	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	auto getMaxViewOffsetZ = [](C_TFPlayer* pPlayer)
	{
		if (pPlayer->m_fFlags() & FL_DUCKING)
			return 45.0f;

		switch (pPlayer->m_iClass())
		{
			case TF_CLASS_SCOUT: return 65.0f;
			case TF_CLASS_SOLDIER: return 68.0f;
			case TF_CLASS_PYRO: return 68.0f;
			case TF_CLASS_DEMOMAN: return 68.0f;
			case TF_CLASS_HEAVYWEAPONS: return 75.0f;
			case TF_CLASS_ENGINEER: return 68.0f;
			case TF_CLASS_MEDIC: return 75.0f;
			case TF_CLASS_SNIPER: return 75.0f;
			case TF_CLASS_SPY: return 75.0f;
			default: return 0.0f;
		}
	};

	if (!CFG::ESP_Active || !CFG::ESP_Players_Active || !CFG::ESP_Players_Sniper_Lines
		|| I::EngineVGui->IsGameUIVisible() || SDKUtils::BInEndOfMatch() || F::SpyCamera->IsRendering())
		return;

	const auto pLocal = H::Entities->GetLocal();

	if (!pLocal)
		return;

	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ALL))
	{
		if (!pEntity)
			continue;

		const auto pPlayer = pEntity->As<C_TFPlayer>();

		if (!pPlayer || pPlayer == pLocal || pPlayer->deadflag() || pPlayer->m_iClass() != TF_CLASS_SNIPER)
			continue;

		const auto pWeapon = pPlayer->m_hActiveWeapon().Get();

		if (!pWeapon)
		{
			continue;
		}

		const bool classicCharging = pWeapon->As<C_TFWeaponBase>()->m_iItemDefinitionIndex() == Sniper_m_TheClassic && pWeapon->As<C_TFSniperRifleClassic>()->m_bCharging();

		if (!pPlayer->InCond(TF_COND_ZOOMED) && !classicCharging)
		{
			continue;
		}

		const bool bIsFriend = pPlayer->IsPlayerOnSteamFriendsList();

		if (CFG::ESP_Players_Ignore_Friends && bIsFriend)
			continue;

		if (!bIsFriend)
		{
			if (CFG::ESP_Players_Ignore_Teammates && pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			if (CFG::ESP_Players_Ignore_Enemies && pPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
				continue;
		}

		Vec3 vForward = {};
		Math::AngleVectors(pPlayer->GetEyeAngles(), &vForward);

		Vec3 vStart = pPlayer->m_vecOrigin() + Vec3(0.0f, 0.0f, getMaxViewOffsetZ(pPlayer));
		Vec3 vEnd = vStart + (vForward * 8192.0f);

		CTraceFilterWorldCustom traceFilter = {};
		trace_t trace = {};

		H::AimUtils->Trace(vStart, vEnd, MASK_SOLID, &traceFilter, &trace);

		vEnd = trace.endpos;

		RenderLine(vStart, vEnd, F::VisualUtils->GetEntityColor(pLocal, pPlayer), true);
	}
}

void ProjectileArc()
{
	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	if (I::Input->CAM_IsThirdPerson())
	{
		return;
	}

	if (!CFG::Visuals_Draw_Projectile_Arc || !G::CHudTFCrosshair_ShouldDraw_Result)
	{
		return;
	}

	const auto pLocal = H::Entities->GetLocal();

	if (!pLocal)
	{
		return;
	}

	const auto pWeapon = H::Entities->GetWeapon();

	if (!pWeapon)
	{
		return;
	}

	ProjectileInfo info{};
	const auto backupOrigin = pLocal->m_vecOrigin();

	pLocal->m_vecOrigin() = pLocal->GetAbsOrigin();

	if (!F::ProjectileSim->GetInfo(pLocal, pWeapon, I::EngineClient->GetViewAngles(), info))
	{
		pLocal->m_vecOrigin() = backupOrigin;

		return;
	}

	pLocal->m_vecOrigin() = backupOrigin;

	if (!F::ProjectileSim->Init(info))
	{
		return;
	}

	CTraceFilterArc filter{};
	auto maxTime = 5.0f;

	if (info.m_type == TF_PROJECTILE_PIPEBOMB)
	{
		maxTime = pWeapon->m_iItemDefinitionIndex() == Demoman_m_TheIronBomber ? 1.54f : 2.2f;
	}

	if (info.m_type == TF_PROJECTILE_CANNONBALL)
	{
		maxTime = 1.0f;
	}

	for (auto n{ 0 }; n < TIME_TO_TICKS(maxTime); n++)
	{
		auto pre{ F::ProjectileSim->GetOrigin() };

		F::ProjectileSim->RunTick();

		auto post{ F::ProjectileSim->GetOrigin() };

		auto clr{ F::VisualUtils->RainbowTickOffset(n) };

		if (CFG::Visuals_Draw_Projectile_Arc_Color_Mode == 0)
		{
			clr = CFG::Color_Projectile_Arc;
		}

		trace_t trace{};

		H::AimUtils->TraceHull(pre, post, { -2.0f, -2.0f, -2.0f }, { 2.0f, 2.0f, 2.0f }, MASK_SOLID, &filter, &trace);

		if (trace.DidHit())
		{
			Vec3 angles{};

			Math::VectorAngles(trace.plane.normal, angles);

			RenderBox(trace.endpos, angles, { -1.0f, -10.0f, -10.0f }, { 1.0f, 10.0f, 10.0f }, { clr.r, clr.g, clr.b, 50 }, false);
			RenderWireframeBox(trace.endpos, angles, { -1.0f, -10.0f, -10.0f }, { 1.0f, 10.0f, 10.0f }, clr, true);

			break;
		}

		RenderLine(pre, post, clr, false);
	}
}

MAKE_HOOK(
	ClientModeShared_DoPostScreenSpaceEffects, Memory::GetVFunc(I::ClientModeShared, 39),
	bool, __fastcall, CClientModeShared* ecx, void* edx, const CViewSetup* pSetup)
{
	const auto original = CALL_ORIGINAL(ecx, edx, pSetup);

	SniperLines();
	ProjectileArc();

	return original;
}
