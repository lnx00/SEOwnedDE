#include "MiscVisuals.h"

#include "../CFG.h"
#include "../RapidFire/RapidFire.h"

#include "../VisualUtils/VisualUtils.h"
#include "../SpyCamera/SpyCamera.h"
#include "../ProjectileSim/ProjectileSim.h"

void CMiscVisuals::AimbotFOVCircle()
{
	if (I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	if (!CFG::Visuals_Aimbot_FOV_Circle
		|| I::EngineVGui->IsGameUIVisible()
		|| !G::CHudTFCrosshair_ShouldDraw_Result
		|| I::Input->CAM_IsThirdPerson())
		return;

	if (const auto pLocal = H::Entities->GetLocal())
	{
		if (const auto flAimFOV = G::flAimbotFOV)
		{
			const float flRadius = tanf(DEG2RAD(flAimFOV) / 2.0f) / tanf(DEG2RAD(static_cast<float>(pLocal->m_iFOV())) / 2.0f) * H::Draw->GetScreenW();
			H::Draw->OutlinedCircle(H::Draw->GetScreenW() / 2, H::Draw->GetScreenH() / 2, static_cast<int>(flRadius), 70, {255, 255, 255, static_cast<byte>(255.0f * CFG::Visuals_Aimbot_FOV_Circle_Alpha)});
		}
	}
}

void CMiscVisuals::ViewModelSway()
{
	static ConVar* cl_wpn_sway_interp = I::CVar->FindVar("cl_wpn_sway_interp");

	if (!cl_wpn_sway_interp)
		return;

	const auto pLocal = H::Entities->GetLocal();

	if (!pLocal)
		return;

	if (CFG::Visuals_ViewModel_Active && CFG::Visuals_ViewModel_Sway && !pLocal->deadflag())
	{
		if (const auto pWeapon = H::Entities->GetWeapon())
		{
			const float flBaseValue = pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW ? 0.02f : 0.05f;

			cl_wpn_sway_interp->SetValue(flBaseValue * CFG::Visuals_ViewModel_Sway_Scale);
		}
	}
	else
	{
		if (cl_wpn_sway_interp->GetFloat() != 0.f)
		{
			cl_wpn_sway_interp->SetValue(0.0f);
		}
	}
}

void CMiscVisuals::DetailProps()
{
	if (!CFG::Visuals_Disable_Detail_Props)
		return;

	static ConVar* r_drawdetailprops = I::CVar->FindVar("r_drawdetailprops");

	if (r_drawdetailprops && r_drawdetailprops->GetInt())
		r_drawdetailprops->SetValue(0);
}

void CMiscVisuals::ShiftBar()
{
	if (!CFG::Exploits_Shifting_Draw_Indicator)
		return;

	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	if (I::EngineVGui->IsGameUIVisible() || !G::CHudTFCrosshair_ShouldDraw_Result || SDKUtils::BInEndOfMatch())
		return;

	const auto pLocal = H::Entities->GetLocal();

	if (!pLocal || pLocal->deadflag())
		return;

	const auto pWeapon = H::Entities->GetWeapon();

	if (!pWeapon)
		return;

	static int nBarW = 80;
	static int nBarH = 4;

	const int nBarX = (H::Draw->GetScreenW() / 2) - (nBarW / 2);
	const int nBarY = (H::Draw->GetScreenH() / 2) + 100;
	const int circleX = H::Draw->GetScreenW() / 2;

	if (CFG::Exploits_Shifting_Indicator_Style == 0)
	{
		H::Draw->Rect(nBarX - 1, nBarY - 1, nBarW + 2, nBarH + 2, CFG::Menu_Background);

		if (Shifting::nAvailableTicks > 0)
		{
			const Color_t color = CFG::Menu_Accent_Secondary;
			const Color_t colorDim = {color.r, color.g, color.b, 25};

			const int nFillWidth = static_cast<int>(Math::RemapValClamped(
				static_cast<float>(Shifting::nAvailableTicks),
				0.0f, static_cast<float>(MAX_COMMANDS),
				0.0f, static_cast<float>(nBarW)
			));

			H::Draw->GradientRect(nBarX, nBarY, nFillWidth, nBarH, colorDim, color, false);
			H::Draw->OutlinedRect(nBarX, nBarY, nFillWidth, nBarH, color);
		}
	}

	if (CFG::Exploits_Shifting_Indicator_Style == 1)
	{
		const float end{Math::RemapValClamped(static_cast<float>(Shifting::nAvailableTicks), 0.0f, MAX_COMMANDS, -90.0f, 359.0f)};

		H::Draw->Arc(circleX, nBarY, 21, 6.0f, -90.0f, 359.0f, CFG::Menu_Background);
		H::Draw->Arc(circleX, nBarY, 20, 4.0f, -90.0f, end, CFG::Menu_Accent_Secondary);
	}

	if (G::nTicksSinceCanFire < 30 && F::RapidFire->IsWeaponSupported(pWeapon))
	{
		if (CFG::Exploits_Shifting_Indicator_Style == 0)
		{
			H::Draw->Rect(nBarX - 1, (nBarY + nBarH + 4) - 1, nBarW + 2, nBarH + 2, CFG::Menu_Background);

			if (G::nTicksSinceCanFire > 0)
			{
				constexpr Color_t color = {241, 196, 15, 255};
				constexpr Color_t colorDim = {color.r, color.g, color.b, 25};

				const int nFillWidth = static_cast<int>(Math::RemapValClamped(
					static_cast<float>(G::nTicksSinceCanFire),
					0.0f, 24.0f,
					0.0f, static_cast<float>(nBarW)
				));

				H::Draw->GradientRect(nBarX, nBarY + nBarH + 4, nFillWidth, nBarH, colorDim, color, false);
				H::Draw->OutlinedRect(nBarX, nBarY + nBarH + 4, nFillWidth, nBarH, color);
			}
		}

		if (CFG::Exploits_Shifting_Indicator_Style == 1)
		{
			const float end{Math::RemapValClamped(static_cast<float>(G::nTicksSinceCanFire), 0.0f, 24.0f, -90.0f, 359.0f)};

			H::Draw->Arc(circleX, nBarY, 24, 2.0f, -90.0f, 359.0f, CFG::Menu_Background);
			H::Draw->Arc(circleX, nBarY, 24, 2.0f, -90.0f, end, {241, 196, 15, 255});
		}
	}
}

void CMiscVisuals::SniperLines()
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

		RenderUtils::RenderLine(vStart, vEnd, F::VisualUtils->GetEntityColor(pLocal, pPlayer), true);
	}
}

void CMiscVisuals::ProjectileArc()
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

			RenderUtils::RenderBox(trace.endpos, angles, { -1.0f, -10.0f, -10.0f }, { 1.0f, 10.0f, 10.0f }, { clr.r, clr.g, clr.b, 50 }, false);
			RenderUtils::RenderWireframeBox(trace.endpos, angles, { -1.0f, -10.0f, -10.0f }, { 1.0f, 10.0f, 10.0f }, clr, true);

			break;
		}

		RenderUtils::RenderLine(pre, post, clr, false);
	}
}
