#include "TeamWellBeing.h"

#include "../CFG.h"
#include "../Menu/Menu.h"
#include "../VisualUtils/VisualUtils.h"

void CTeamWellBeing::Drag()
{
	const int nMouseX = H::Input->GetMouseX();
	const int nMouseY = H::Input->GetMouseY();

	bool bHovered = false;
	static bool bDragging = false;

	if (!bDragging && F::Menu->IsMenuWindowHovered())
		return;

	static int nDeltaX = 0;
	static int nDeltaY = 0;

	const int nListX = CFG::Visuals_TeamWellBeing_Pos_X;
	const int nListY = CFG::Visuals_TeamWellBeing_Pos_Y;

	bHovered = nMouseX > nListX && nMouseX < nListX + CFG::Visuals_TeamWellBeing_Width && nMouseY > nListY && nMouseY < nListY + CFG::Menu_Drag_Bar_Height;

	if (bHovered && H::Input->IsPressed(VK_LBUTTON))
	{
		nDeltaX = nMouseX - nListX;
		nDeltaY = nMouseY - nListY;
		bDragging = true;
	}

	if (!H::Input->IsPressed(VK_LBUTTON) && !H::Input->IsHeld(VK_LBUTTON))
		bDragging = false;

	if (bDragging)
	{
		CFG::Visuals_TeamWellBeing_Pos_X = nMouseX - nDeltaX;
		CFG::Visuals_TeamWellBeing_Pos_Y = nMouseY - nDeltaY;
	}
}

void CTeamWellBeing::Run()
{
	if (!CFG::Visuals_TeamWellBeing_Active)
		return;

	// Anti screenshot?
	if (CFG::Misc_Clean_Screenshot && I::EngineClient->IsTakingScreenshot())
	{
		return;
	}

	if (!F::Menu->IsOpen() && (I::EngineVGui->IsGameUIVisible() || SDKUtils::BInEndOfMatch()))
		return;

	if (F::Menu->IsOpen())
	{
		Drag();
	}
	else
	{
		if (CFG::Visuals_TeamWellBeing_Medic_Only)
		{
			const auto local{ H::Entities->GetLocal() };
			if (local && local->m_iClass() != TF_CLASS_MEDIC)
			{
				return;
			}
		}
	}

	const auto& outlineColor = CFG::Menu_Accent_Secondary;
	const auto bgColor = F::VisualUtils->GetAlphaColor(CFG::Menu_Background, CFG::Visuals_TeamWellBeing_Background_Alpha);

	// Background
	H::Draw->Rect(
		CFG::Visuals_TeamWellBeing_Pos_X,
		CFG::Visuals_TeamWellBeing_Pos_Y,
		CFG::Visuals_TeamWellBeing_Width,
		CFG::Menu_Drag_Bar_Height,
		bgColor
	);

	// Title
	H::Draw->String(
		H::Fonts->Get(EFonts::Menu),
		CFG::Visuals_TeamWellBeing_Pos_X + (CFG::Visuals_TeamWellBeing_Width / 2),
		CFG::Visuals_TeamWellBeing_Pos_Y + (CFG::Menu_Drag_Bar_Height / 2),
		CFG::Menu_Text,
		POS_CENTERXY,
		"Team Well-Being"
	);

	// Outline
	H::Draw->OutlinedRect(
		CFG::Visuals_TeamWellBeing_Pos_X,
		CFG::Visuals_TeamWellBeing_Pos_Y,
		CFG::Visuals_TeamWellBeing_Width,
		CFG::Menu_Drag_Bar_Height,
		outlineColor
	);

	const auto local{ H::Entities->GetLocal() };
	if (!local || local->deadflag() || (CFG::Visuals_TeamWellBeing_Medic_Only && local->m_iClass() != TF_CLASS_MEDIC))
	{
		return;
	}

	auto drawn{ 1 };

	for (const auto ent : H::Entities->GetGroup(EEntGroup::PLAYERS_TEAMMATES))
	{
		if (!ent || ent == local)
		{
			continue;
		}

		// Should the player be drawn?
		const auto player{ ent->As<C_TFPlayer>() };
		if (player->deadflag() || player->GetCenter().DistTo(local->GetShootPos()) > 449.0f)
		{
			continue;
		}

		player_info_t pi{};
		if (!I::EngineClient->GetPlayerInfo(player->entindex(), &pi))
		{
			continue;
		}

		const auto drawY{ (CFG::Visuals_TeamWellBeing_Pos_Y + (CFG::Menu_Drag_Bar_Height * drawn)) - 1 };
		const auto textY{ drawY + (CFG::Menu_Drag_Bar_Height / 2) };

		const auto nameSeparatorX{
			CFG::Visuals_TeamWellBeing_Pos_X
			+ static_cast<int>(static_cast<float>(CFG::Visuals_TeamWellBeing_Width) * 0.3f)
		};

		const auto classSeparatorX{
			CFG::Visuals_TeamWellBeing_Pos_X
			+ static_cast<int>(static_cast<float>(CFG::Visuals_TeamWellBeing_Width) * 0.3f) + 18
		};

		// Player background
		H::Draw->Rect(
			CFG::Visuals_TeamWellBeing_Pos_X,
			drawY,
			CFG::Visuals_TeamWellBeing_Width,
			CFG::Menu_Drag_Bar_Height + 1,
			bgColor
		);

		H::Draw->StartClipping
		(
			CFG::Visuals_TeamWellBeing_Pos_X,
			drawY,
			static_cast<int>(static_cast<float>(CFG::Visuals_TeamWellBeing_Width) * 0.3f) - CFG::Menu_Spacing_X,
			CFG::Menu_Drag_Bar_Height
		);

		// Player name
		H::Draw->String(
			H::Fonts->Get(EFonts::Menu),
			CFG::Visuals_TeamWellBeing_Pos_X + CFG::Menu_Spacing_X,
			textY,
			CFG::Menu_Text_Inactive,
			POS_CENTERY,
			Utils::ConvertUtf8ToWide(pi.name).c_str()
		);

		H::Draw->EndClipping();

		H::Draw->Texture(classSeparatorX - (18 / 2), textY, 13, 13, F::VisualUtils->GetClassIcon(player->m_iClass()), POS_CENTERXY);

		const auto barStartX{ classSeparatorX + CFG::Menu_Spacing_X };
		const auto maxBarW{ static_cast<int>(static_cast<float>(CFG::Visuals_TeamWellBeing_Width) * 0.7f) - (18 + (CFG::Menu_Spacing_X * 2)) };
		const auto barH{ CFG::Menu_Drag_Bar_Height - (CFG::Menu_Spacing_Y * 2) + 1 };
		const auto barW{ static_cast<int>(Math::RemapValClamped(static_cast<float>(player->m_iHealth()), 0.0f, static_cast<float>(player->GetMaxHealth()), 0.0f, static_cast<float>(maxBarW))) };
		const auto overhealBarW{
			static_cast<int>(Math::RemapValClamped(static_cast<float>(player->m_iHealth()), static_cast<float>(player->GetMaxHealth()), static_cast<float>(player->GetMaxHealth()) * 1.45f, 0.0f,
			                                       static_cast<float>(maxBarW)))
		};

		H::Draw->Rect
		(
			barStartX,
			drawY + CFG::Menu_Spacing_Y,
			maxBarW,
			barH,
			{ 0, 0, 0, 200 }
		);

		// Health bar
		if (barW)
		{
			H::Draw->Rect
			(
				barStartX,
				drawY + CFG::Menu_Spacing_Y,
				barW,
				barH,
				F::VisualUtils->GetHealthColorAlt(player->m_iHealth(), player->GetMaxHealth())
			);

			if (overhealBarW)
			{
				H::Draw->Rect
				(
					barStartX,
					drawY + CFG::Menu_Spacing_Y,
					overhealBarW,
					barH,
					CFG::Color_OverHeal
				);
			}

			H::Draw->OutlinedRect
			(
				barStartX,
				drawY + CFG::Menu_Spacing_Y,
				maxBarW,
				barH,
				{ 0, 0, 0, 253 }
			);
		}

		H::Draw->OutlinedRect(
			CFG::Visuals_TeamWellBeing_Pos_X,
			drawY,
			CFG::Visuals_TeamWellBeing_Width,
			CFG::Menu_Drag_Bar_Height + 1,
			outlineColor
		);

		H::Draw->OutlinedRect(
			nameSeparatorX,
			drawY,
			1,
			CFG::Menu_Drag_Bar_Height + 1,
			outlineColor
		);

		H::Draw->OutlinedRect(
			classSeparatorX,
			drawY,
			1,
			CFG::Menu_Drag_Bar_Height + 1,
			outlineColor
		);

		drawn++;
	}
}
