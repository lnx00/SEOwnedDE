#include "WorldModulation.h"

#include "../CFG.h"

void CWorldModulation::ApplyModulation(const Color_t& clr, bool world, bool sky)
{
	if (!world && !sky)
	{
		return;
	}

	for (auto h{ I::MaterialSystem->FirstMaterial() }; h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		// Valid material?
		const auto mat = I::MaterialSystem->GetMaterial(h);
		if (!mat || mat->IsErrorMaterial() || !mat->IsPrecached())
		{
			continue;
		}

		const bool isWorld = std::string_view(mat->GetTextureGroupName()).find(TEXTURE_GROUP_WORLD) != std::string_view::npos;
		const bool isSky = std::string_view(mat->GetName()).find("sky") != std::string_view::npos;

		// Apply color for world
		if (world && isWorld && !isSky)
		{
			if (const auto var = mat->FindVar("$color2", nullptr))
			{
				var->SetVecValue(ColorUtils::ToFloat(clr.r), ColorUtils::ToFloat(clr.g), ColorUtils::ToFloat(clr.b));
			}
			else
			{
				mat->ColorModulate(ColorUtils::ToFloat(clr.r), ColorUtils::ToFloat(clr.g), ColorUtils::ToFloat(clr.b));
			}
		}

		// Apply color for sky
		if (sky && isSky)
		{
			mat->ColorModulate(ColorUtils::ToFloat(clr.r), ColorUtils::ToFloat(clr.g), ColorUtils::ToFloat(clr.b));
		}
	}

	m_bWorldWasModulated = true;
}

void CWorldModulation::UpdateWorldModulation()
{
	auto modeChanged = [&]
	{
		static auto old = CFG::Visuals_World_Modulation_Mode;
		const auto cur = CFG::Visuals_World_Modulation_Mode;

		if (cur != old)
		{
			old = cur;
			return true;
		}

		return false;
	};

	auto ignoreSkyChanged = [&]
	{
		static auto old = CFG::Visuals_World_Modulation_No_Sky_Change;
		const auto cur = CFG::Visuals_World_Modulation_No_Sky_Change;

		if (cur != old)
		{
			old = cur;
			return true;
		}

		return false;
	};

	const bool resetSky = ignoreSkyChanged();

	// Night mode
	if (CFG::Visuals_World_Modulation_Mode == 0)
	{
		if (CFG::Visuals_Night_Mode <= 0.0f)
		{
			RestoreWorldModulation();
			return;
		}

		auto valueChanged = [&]
		{
			static auto old{ CFG::Visuals_Night_Mode };
			const auto cur{ CFG::Visuals_Night_Mode };

			if (cur != old)
			{
				old = cur;
				return true;
			}

			return false;
		};

		if (!m_bWorldWasModulated || valueChanged() || modeChanged() || resetSky)
		{
			const auto col = static_cast<byte>(Math::RemapValClamped(CFG::Visuals_Night_Mode, 0.0f, 100.0f, 255.0f, 5.0f));
			ApplyModulation({ col, col, col, static_cast<byte>(255) }, true, !CFG::Visuals_World_Modulation_No_Sky_Change);
		}

		if (resetSky && CFG::Visuals_World_Modulation_No_Sky_Change)
		{
			ApplyModulation(Colors::WHITE, false, true);
		}
	}

	// Custom color
	else
	{
		if (!m_bWorldWasModulated || modeChanged())
		{
			ApplyModulation(CFG::Color_World, true, false);
			ApplyModulation(CFG::Color_Sky, false, !CFG::Visuals_World_Modulation_No_Sky_Change);
		}

		auto worldValueChanged = [&]
		{
			static auto old{ CFG::Color_World };
			const auto cur{ CFG::Color_World };

			if (cur.r != old.r || cur.g != old.g || cur.b != old.b)
			{
				old = cur;
				return true;
			}

			return false;
		};

		auto skyValueChanged = [&]
		{
			static auto old{ CFG::Color_Sky };
			const auto cur{ CFG::Color_Sky };

			if (cur.r != old.r || cur.g != old.g || cur.b != old.b)
			{
				old = cur;
				return true;
			}

			return false;
		};

		ApplyModulation(CFG::Color_World, worldValueChanged(), false);

		if (!CFG::Visuals_World_Modulation_No_Sky_Change)
		{
			ApplyModulation(CFG::Color_Sky, false, skyValueChanged());
		}

		if (resetSky)
		{
			if (CFG::Visuals_World_Modulation_No_Sky_Change)
			{
				ApplyModulation(Colors::WHITE, false, true);
			}
			else
			{
				ApplyModulation(CFG::Color_Sky, false, true);
			}
		}
	}
}

void CWorldModulation::RestoreWorldModulation()
{
	if (!m_bWorldWasModulated)
		return;

	ApplyModulation(Colors::WHITE, true, true);

	m_bWorldWasModulated = false;
}
