#include "WorldModulation.h"

#include "../CFG.h"

void CWorldModulation::ApplyModulation(Color_t clr, bool world, bool sky)
{
	if (!world && !sky)
	{
		return;
	}

	for (auto h{ I::MaterialSystem->FirstMaterial() }; h != I::MaterialSystem->InvalidMaterial(); h = I::MaterialSystem->NextMaterial(h))
	{
		const auto mat = I::MaterialSystem->GetMaterial(h);
		if (!mat || mat->IsErrorMaterial() || !mat->IsPrecached())
		{
			continue;
		}
		
		if (world && std::string_view(mat->GetTextureGroupName()).find(TEXTURE_GROUP_WORLD) != std::string_view::npos
			&& std::string_view(mat->GetName()).find("sky") == std::string_view::npos)
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

		if (sky && std::string_view(mat->GetName()).find("sky") != std::string_view::npos)
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

	bool resetSky{};
	if (ignoreSkyChanged())
	{
		resetSky = true;
	}

	if (CFG::Visuals_World_Modulation_Mode == 0)
	{
		if (CFG::Visuals_Night_Mode <= 0.0f)
		{
			RestoreWorldModulation();

			return;
		}

		auto valueChanged = [&]()
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
			const auto col{ static_cast<byte>(Math::RemapValClamped(CFG::Visuals_Night_Mode, 0.0f, 100.0f, 255.0f, 5.0f)) };

			ApplyModulation({ col, col, col, static_cast<byte>(255) }, true, !CFG::Visuals_World_Modulation_No_Sky_Change);
		}

		if (resetSky && CFG::Visuals_World_Modulation_No_Sky_Change)
		{
			ApplyModulation({ 255, 255, 255, 255 }, false, true);
		}
	}

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
				ApplyModulation({ 255, 255, 255, 255 }, false, true);
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
	{
		return;
	}

	ApplyModulation({ 255, 255, 255, 255 }, true, true);

	m_bWorldWasModulated = false;
}