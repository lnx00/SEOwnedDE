#include "../../SDK/SDK.h"

#include "../Features/CFG.h"

MAKE_HOOK(
	IMaterialSystem_FindTexture,
	Memory::GetVFunc(I::MaterialSystem, 81),
	ITexture *,
	__fastcall,
	void *ecx, void *edx, char const *pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags)
{
	ITexture *const result{ CALL_ORIGINAL(ecx, edx, pTextureName, pTextureGroupName, complain, nAdditionalCreationFlags) };

	if (CFG::Visuals_Flat_Textures)
	{
		auto replaceWorld = [&]() -> ITexture *
		{
			if (!result
				|| result->IsTranslucent()
				|| !pTextureName
				|| !pTextureGroupName
				|| !std::string_view{ pTextureGroupName }.starts_with(TEXTURE_GROUP_WORLD)
				|| std::string_view{ pTextureName }.find("sky") != std::string_view::npos
				|| std::string_view{ pTextureName }.find("water") != std::string_view::npos)
			{
				return nullptr;
			}

			Vector clr{};

			result->GetLowResColorSample(0.5f, 0.5f, &clr.x);

			unsigned char bits[4]
			{
				static_cast<unsigned char>(clr.x * 255.0f),
				static_cast<unsigned char>(clr.y * 255.0f),
				static_cast<unsigned char>(clr.z * 255.0f),
				255
			};

			return I::MaterialSystem->CreateTextureFromBits(1, 1, 1, IMAGE_FORMAT_RGBA8888, 4, bits);
		};

		if (ITexture *const replacement{ replaceWorld() })
		{
			return replacement;
		}
	}

	return result;
}