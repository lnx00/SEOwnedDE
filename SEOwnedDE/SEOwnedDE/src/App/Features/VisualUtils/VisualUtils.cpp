#include "VisualUtils.h"

#include "../CFG.h"

#include "Icons.h"

#include "../Players/Players.h"

bool CVisualUtils::IsEntityOwnedBy(C_BaseEntity* pEntity, C_BaseEntity* pWho)
{
	switch (pEntity->GetClassId())
	{
	case ETFClassIds::CTFGrenadePipebombProjectile:
	case ETFClassIds::CTFProjectile_Jar:
	case ETFClassIds::CTFProjectile_JarGas:
	case ETFClassIds::CTFProjectile_JarMilk:
	case ETFClassIds::CTFProjectile_Cleaver:
		{
			return pEntity->As<C_BaseGrenade>()->m_hThrower().Get() == pWho;
		}

	case ETFClassIds::CTFProjectile_SentryRocket:
		{
			if (const auto pOwner = pEntity->m_hOwnerEntity().Get())
				return pOwner->As<C_BaseObject>()->m_hBuilder() == pWho;

			break;
		}

	case ETFClassIds::CObjectSentrygun:
	case ETFClassIds::CObjectDispenser:
	case ETFClassIds::CObjectTeleporter:
		{
			return pEntity->As<C_BaseObject>()->m_hBuilder() == pWho;
		}

	default: return pEntity->m_hOwnerEntity().Get() == pWho;
	}

	return false;
}

Color_t CVisualUtils::GetEntityColor(C_TFPlayer* pLocal, C_BaseEntity* pEntity)
{
	if (!pLocal || !pEntity)
		return { 255, 255, 255, 255 };

	if (pEntity->entindex() == G::nTargetIndex)
		return CFG::Color_Target;

	if (pEntity->GetClassId() == ETFClassIds::CTFPlayer)
	{
		const auto pPlayer = pEntity->As<C_TFPlayer>();

		if (pPlayer->IsInvulnerable())
			return CFG::Color_Invulnerable;

		if (pPlayer->IsInvisible())
			return CFG::Color_Invisible;

		if (pPlayer != pLocal && pPlayer->IsPlayerOnSteamFriendsList())
			return CFG::Color_Friend;

		if (pPlayer != pLocal)
		{
			// TODO: Handle these colors in F::Players
			PlayerPriority info{};
			F::Players->GetInfo(pPlayer->entindex(), info);

			if (info.Cheater)
			{
				return CFG::Color_Cheater;
			}

			if (info.RetardLegit)
			{
				return CFG::Color_RetardLegit;
			}
		}
	}

	if (pEntity == pLocal || IsEntityOwnedBy(pEntity, pLocal))
		return CFG::Color_Local;

	if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
		return CFG::Color_Teammate;

	if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
		return CFG::Color_Enemy;

	return { 255, 255, 255, 255 };
}

Color_t CVisualUtils::GetHealthColor(int nHealth, int nMaxHealth)
{
	if (nHealth > nMaxHealth)
		return CFG::Color_OverHeal;

	nHealth = std::max(0, std::min(nHealth, nMaxHealth));
	const int r = std::min((510 * (nMaxHealth - nHealth)) / nMaxHealth, 210);
	const int g = std::min((510 * nHealth) / nMaxHealth, 230);
	return { static_cast<byte>(r), static_cast<byte>(g), 50, 255 };
}

Color_t CVisualUtils::GetHealthColorAlt(int nHealth, int nMaxHealth)
{
	nHealth = std::max(0, std::min(nHealth, nMaxHealth));
	const int r = std::min((510 * (nMaxHealth - nHealth)) / nMaxHealth, 255);
	const int g = std::min((510 * nHealth) / nMaxHealth, 230);
	return { static_cast<byte>(r), static_cast<byte>(g), 50, 255 };
}

int CVisualUtils::CreateTextureFromArray(const unsigned char* rgba, int w, int h)
{
	const int nTextureIdOut = I::MatSystemSurface->CreateNewTextureID(true);
	I::MatSystemSurface->DrawSetTextureRGBAEx(nTextureIdOut, rgba, w, h, IMAGE_FORMAT_BGRA8888);
	return nTextureIdOut;
}

int CVisualUtils::CreateTextureFromVTF(const char* name)
{
	const int nTextureIdOut = I::MatSystemSurface->CreateNewTextureID(false);
	I::MatSystemSurface->DrawSetTextureFile(nTextureIdOut, name, 0, true);
	return nTextureIdOut;
}

int CVisualUtils::GetClassIcon(int nClassNum)
{
	//what are arrays
	// TODO: Use the game's textures

	static int nScout = CreateTextureFromArray(Icons::leaderboard_class_scout, 64, 64);
	static int nSoldier = CreateTextureFromArray(Icons::leaderboard_class_soldier, 64, 64);
	static int nPyro = CreateTextureFromArray(Icons::leaderboard_class_pyro, 64, 64);
	static int nDemoman = CreateTextureFromArray(Icons::leaderboard_class_demo, 64, 64);
	static int nHeavy = CreateTextureFromArray(Icons::leaderboard_class_heavy, 64, 64);
	static int nEngineer = CreateTextureFromArray(Icons::leaderboard_class_engineer, 64, 64);
	static int nMedic = CreateTextureFromArray(Icons::leaderboard_class_medic, 64, 64);
	static int nSniper = CreateTextureFromArray(Icons::leaderboard_class_sniper, 64, 64);
	static int nSpy = CreateTextureFromArray(Icons::leaderboard_class_spy, 64, 64);

	switch (nClassNum)
	{
	case TF_CLASS_SCOUT: return nScout;
	case TF_CLASS_SOLDIER: return nSoldier;
	case TF_CLASS_PYRO: return nPyro;
	case TF_CLASS_DEMOMAN: return nDemoman;
	case TF_CLASS_HEAVYWEAPONS: return nHeavy;
	case TF_CLASS_ENGINEER: return nEngineer;
	case TF_CLASS_MEDIC: return nMedic;
	case TF_CLASS_SNIPER: return nSniper;
	case TF_CLASS_SPY: return nSpy;
	default: break;
	}

	return 0;
}

int CVisualUtils::GetBuildingTextureId(C_BaseObject* pObject)
{
	static int nSentryGunLvl1 = CreateTextureFromVTF("hud/hud_obj_status_sentry_1.vtf");
	static int nSentryGunLvl2 = CreateTextureFromVTF("hud/hud_obj_status_sentry_2.vtf");
	static int nSentryGunLvl3 = CreateTextureFromVTF("hud/hud_obj_status_sentry_3.vtf");
	static int nDispenser = CreateTextureFromVTF("hud/hud_obj_status_dispenser.vtf");
	static int nTeleporter = CreateTextureFromVTF("hud/hud_obj_status_tele_entrance.vtf");

	if (!pObject)
		return 0;

	switch (pObject->GetClassId())
	{
	case ETFClassIds::CObjectSentrygun:
		{
			switch (pObject->m_iUpgradeLevel())
			{
			case 1: return nSentryGunLvl1;
			case 2: return nSentryGunLvl2;
			case 3: return nSentryGunLvl3;
			default: break;
			}

			break;
		}

	case ETFClassIds::CObjectDispenser: return nDispenser;
	case ETFClassIds::CObjectTeleporter: return nTeleporter;

	default: break;
	}

	return 0;
}

int CVisualUtils::GetHealthIconTextureId()
{
	static int nOut = CreateTextureFromArray(Icons::health, 64, 64);
	return nOut;
}

int CVisualUtils::GetAmmoIconTextureId()
{
	static int nOut = CreateTextureFromArray(Icons::ammo, 64, 64);
	return nOut;
}

int CVisualUtils::GetHalloweenGiftTextureId()
{
	static int nOut = CreateTextureFromArray(Icons::halloween_gift, 50, 53);
	return nOut;
}

bool CVisualUtils::IsOnScreen(const C_TFPlayer* pLocal, const C_BaseEntity* pEntity)
{
	const Vec3& vPos = pEntity->GetAbsOrigin();
	if (vPos.DistTo(pLocal->GetAbsOrigin()) > 300.0f)
	{
		Vec3 vScreen = {};

		if (H::Draw->W2S(vPos, vScreen))
		{
			if (vScreen.x < -400
				|| vScreen.x > H::Draw->GetScreenW() + 400
				|| vScreen.y < -400
				|| vScreen.y > H::Draw->GetScreenH() + 400)
				return false;
		}

		else
		{
			return false;
		}
	}

	return true;
}

bool CVisualUtils::IsOnScreenNoEntity(const C_TFPlayer* pLocal, const Vec3& vAbsOrigin)
{
	const Vec3& vPos = vAbsOrigin;
	if (vPos.DistTo(pLocal->GetAbsOrigin()) > 300.0f)
	{
		Vec3 vScreen = {};

		if (H::Draw->W2S(vPos, vScreen))
		{
			if (vScreen.x < -400
				|| vScreen.x > H::Draw->GetScreenW() + 400
				|| vScreen.y < -400
				|| vScreen.y > H::Draw->GetScreenH() + 400)
				return false;
		}
		else
		{
			return false;
		}
	}

	return true;
}

int CVisualUtils::GetCat(int nFrame)
{
	if (nFrame < 0 || nFrame > 3)
		return 0;

	static const int ARR_FRAMES[4] =
	{
		CreateTextureFromArray(Icons::cat_0, 12, 12),
		CreateTextureFromArray(Icons::cat_1, 12, 12),
		CreateTextureFromArray(Icons::cat_2, 12, 12),
		CreateTextureFromArray(Icons::cat_3, 12, 12)
	};

	return ARR_FRAMES[nFrame];
}

int CVisualUtils::GetCat2(int nFrame)
{
	if (nFrame < 0 || nFrame > 3)
		return 0;

	static const int ARR_FRAMES[4] =
	{
		CreateTextureFromArray(Icons::cat2_0, 12, 12),
		CreateTextureFromArray(Icons::cat2_1, 12, 12),
		CreateTextureFromArray(Icons::cat2_2, 12, 12),
		CreateTextureFromArray(Icons::cat2_3, 12, 12)
	};

	return ARR_FRAMES[nFrame];
}

int CVisualUtils::GetCatSleep(int nFrame)
{
	if (nFrame < 0 || nFrame > 3)
		return 0;

	static const int ARR_FRAMES[4] =
	{
		CreateTextureFromArray(Icons::cat_sleep0, 16, 8),
		CreateTextureFromArray(Icons::cat_sleep1, 16, 8),
		CreateTextureFromArray(Icons::cat_sleep2, 16, 8),
		CreateTextureFromArray(Icons::cat_sleep3, 16, 8)
	};

	return ARR_FRAMES[nFrame];
}

int CVisualUtils::GetCatRun(int nFrame)
{
	if (nFrame < 0 || nFrame > 7)
		return 0;

	static const int ARR_FRAMES[8] =
	{
		CreateTextureFromArray(Icons::cat_run0, 20, 13),
		CreateTextureFromArray(Icons::cat_run1, 20, 13),
		CreateTextureFromArray(Icons::cat_run2, 20, 13),
		CreateTextureFromArray(Icons::cat_run3, 20, 13),
		CreateTextureFromArray(Icons::cat_run4, 20, 13),
		CreateTextureFromArray(Icons::cat_run5, 20, 13),
		CreateTextureFromArray(Icons::cat_run6, 20, 13),
		CreateTextureFromArray(Icons::cat_run7, 20, 13)
	};

	return ARR_FRAMES[nFrame];
}

Color_t CVisualUtils::Rainbow()
{
	const float t = TICKS_TO_TIME(I::GlobalVars->tickcount);

	const int r = static_cast<int>(std::round(std::cos(I::GlobalVars->realtime + t + 0.0f) * 127.5f + 127.5f));
	const int g = static_cast<int>(std::round(std::cos(I::GlobalVars->realtime + t + 2.0f) * 127.5f + 127.5f));
	const int b = static_cast<int>(std::round(std::cos(I::GlobalVars->realtime + t + 4.0f) * 127.5f + 127.5f));

	return Color_t{ static_cast<byte>(r), static_cast<byte>(g), static_cast<byte>(b), 255 };
}

Color_t CVisualUtils::RainbowTickOffset(int nTick)
{
	const float t = TICKS_TO_TIME(nTick);

	const int r = static_cast<int>(std::lround(std::cos((I::GlobalVars->realtime * 2.0f) + t + 0.0f) * 127.5f + 127.5f));
	const int g = static_cast<int>(std::lround(std::cos((I::GlobalVars->realtime * 2.0f) + t + 2.0f) * 127.5f + 127.5f));
	const int b = static_cast<int>(std::lround(std::cos((I::GlobalVars->realtime * 2.0f) + t + 4.0f) * 127.5f + 127.5f));

	return Color_t{ static_cast<byte>(r), static_cast<byte>(g), static_cast<byte>(b), 255 };
}
