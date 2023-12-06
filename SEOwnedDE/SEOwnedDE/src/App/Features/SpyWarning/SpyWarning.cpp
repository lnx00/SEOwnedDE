#include "SpyWarning.h"

#include "../CFG.h"

#include "../VisualUtils/VisualUtils.h"

constexpr auto SCREEN_OFFSET_X_SCALE = 0.5f;
constexpr auto SCREEN_OFFSET_Y_SCALE = 0.2f;
constexpr auto ARROW_RADIUS = 32.0f;
constexpr auto MAX_DIST = 500.0f;

void CSpyWarning::Run()
{
	if (!CFG::Viuals_SpyWarning_Active)
		return;

	// Anti screenshot?
	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	const auto pLocal = H::Entities->GetLocal();
	if (!pLocal || pLocal->deadflag())
		return;

	std::vector<C_TFPlayer*> spies{};
	const auto localRenderCenter = pLocal->GetRenderCenter();
	for (const auto pEntity : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
	{
		if (!pEntity)
			continue;

		// Is the player valid?
		const auto pPlayer = pEntity->As<C_TFPlayer>();
		if (!pPlayer || pPlayer->deadflag() || pPlayer->m_iClass() != TF_CLASS_SPY)
			continue;

		// Maximum distance
		const auto renderCenter = pPlayer->GetRenderCenter();
		if (renderCenter.DistTo(localRenderCenter) > MAX_DIST)
			continue;

		// Are we in a halloween kart?
		if (pPlayer->InCond(TF_COND_HALLOWEEN_KART) || pPlayer->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
			continue;

		// Ignore cloaked
		if (CFG::Viuals_SpyWarning_Ignore_Cloaked)
		{
			if (pPlayer->InCond(TF_COND_STEALTHED) || pPlayer->m_flInvisibility() >= 0.8f)
				continue;
		}

		// Ignore friends
		if (CFG::Viuals_SpyWarning_Ignore_Friends && pPlayer->IsPlayerOnSteamFriendsList())
			continue;

		// Is the spy in FOV?
		if (Math::CalcFov(I::EngineClient->GetViewAngles(), Math::CalcAngle(pLocal->GetShootPos(), renderCenter)) < 70.0f)
			continue;

		// Ignore invisible
		if (CFG::Viuals_SpyWarning_Ignore_Invisible)
		{
			if (!H::AimUtils->TracePositionWorld(pLocal->GetShootPos(), renderCenter))
				continue;
		}

		spies.push_back(pPlayer);
	}

	// Announce spy
	static bool lastEmpty = spies.empty();
	if (spies.empty() != lastEmpty)
	{
		lastEmpty = spies.empty();

		if (!spies.empty() && CFG::Viuals_SpyWarning_Announce)
		{
			I::EngineClient->ClientCmd_Unrestricted("voicemenu 1 1");
		}
	}

	// Indicator (Icon + Triangle)
	if (!I::EngineVGui->IsGameUIVisible())
	{
		// Direction triangle
		for (const auto& pPlayer : spies)
		{
			if (!pPlayer)
				continue;

			const int nScreenCenterX = static_cast<int>(static_cast<float>(H::Draw->GetScreenW()) * SCREEN_OFFSET_X_SCALE);
			const int nScreenCenterY = static_cast<int>(static_cast<float>(H::Draw->GetScreenH()) * SCREEN_OFFSET_Y_SCALE);
			const auto spyPos = pPlayer->GetRenderCenter();

			Vec3 vScreen = {};
			H::Draw->ScreenPosition(spyPos, vScreen);

			Vec3 vAngle = {};
			Math::VectorAngles({nScreenCenterX - vScreen.x, nScreenCenterY - vScreen.y, 0.0f}, vAngle);

			const float flYaw = DEG2RAD(vAngle.y);

			// Scale the triangle
			const auto dist = pLocal->GetShootPos().DistTo(spyPos);
			const float flRadius = Math::RemapValClamped(dist, 0.0f, MAX_DIST, ARROW_RADIUS, ARROW_RADIUS * 2.0f);
			const float flScale = Math::RemapValClamped(dist, 0.0f, MAX_DIST, 2.0f, 1.0f);

			const float flDrawX = nScreenCenterX - flRadius * cosf(flYaw);
			const float flDrawY = nScreenCenterY - flRadius * sinf(flYaw);

			std::array vPoints = {
				Vec2(flDrawX + (6.0f * flScale), flDrawY + (6.0f * flScale)),
				Vec2(flDrawX - (4.0f * flScale), flDrawY),
				Vec2(flDrawX + (6.0f * flScale), flDrawY - (6.0f * flScale))
			};

			Math::RotateTriangle(vPoints, vAngle.y);
			H::Draw->FilledTriangle(vPoints, F::VisualUtils->GetEntityColor(pLocal, pPlayer));
		}

		// Spy icon
		if (!spies.empty())
		{
			const int nScreenCenterX = static_cast<int>(static_cast<float>(H::Draw->GetScreenW()) * SCREEN_OFFSET_X_SCALE);
			const int nScreenCenterY = static_cast<int>(static_cast<float>(H::Draw->GetScreenH()) * SCREEN_OFFSET_Y_SCALE);

			H::Draw->Texture(nScreenCenterX, nScreenCenterY, 36, 36, F::VisualUtils->GetClassIcon(TF_CLASS_SPY), POS_CENTERXY);
		}
	}
}
