#include "Paint.h"

#include "../CFG.h"

#pragma warning (disable : 4244) //possible loss of data (int to float)

void CPaint::Initialize()
{
	if (!m_pMatGlowColor)
	{
		m_pMatGlowColor = I::MaterialSystem->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER);
	}

	if (!m_pRtFullFrame)
	{
		m_pRtFullFrame = I::MaterialSystem->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
	}

	if (!m_pRenderBuffer0)
	{
		m_pRenderBuffer0 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
			"seo_paint_buffer0",
			m_pRtFullFrame->GetActualWidth(),
			m_pRtFullFrame->GetActualHeight(),
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);

		m_pRenderBuffer0->IncrementReferenceCount();
	}

	if (!m_pRenderBuffer1)
	{
		m_pRenderBuffer1 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
			"seo_paint_buffer1",
			m_pRtFullFrame->GetActualWidth(),
			m_pRtFullFrame->GetActualHeight(),
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);

		m_pRenderBuffer1->IncrementReferenceCount();
	}

	if (!m_pMatHaloAddToScreen)
	{
		const auto kv = new KeyValues("UnlitGeneric");
		kv->SetString("$basetexture", "seo_paint_buffer0");
		kv->SetString("$additive", "1");
		m_pMatHaloAddToScreen = I::MaterialSystem->CreateMaterial("seo_paint_material", kv);
	}

	if (!m_pMatBlurX)
	{
		const auto kv = new KeyValues("BlurFilterX");
		kv->SetString("$basetexture", "seo_paint_buffer0");
		m_pMatBlurX = I::MaterialSystem->CreateMaterial("seo_paint_material_blurx", kv);
	}

	if (!m_pMatBlurY)
	{
		const auto kv = new KeyValues("BlurFilterY");
		kv->SetString("$basetexture", "seo_paint_buffer1");
		m_pMatBlurY = I::MaterialSystem->CreateMaterial("seo_paint_material_blury", kv);
		m_pBloomAmount = m_pMatBlurY->FindVar("$bloomamount", nullptr);
	}
}

void CPaint::Run()
{
	auto RenderLine = [&](const Vector& v1, const Vector& v2, Color_t c, bool bZBuffer)
	{
		reinterpret_cast<void(__cdecl *)(const Vector&, const Vector&, Color_t, bool)>(Signatures::RenderLine.Get())(v1, v2, c, bZBuffer);
	};

	auto Rainbow = [&](int nTick)
	{
		constexpr float rate = 3.0f;
		const float t = TICKS_TO_TIME(nTick);

		const int r = std::lround(std::cosf(I::GlobalVars->realtime + t * rate + 0.0f) * 127.5f + 127.5f);
		const int g = std::lround(std::cosf(I::GlobalVars->realtime + t * rate + 2.0f) * 127.5f + 127.5f);
		const int b = std::lround(std::cosf(I::GlobalVars->realtime + t * rate + 4.0f) * 127.5f + 127.5f);

		return Color_t{static_cast<byte>(r), static_cast<byte>(g), static_cast<byte>(b), static_cast<byte>(255)};
	};

	int w = H::Draw->GetScreenW(), h = H::Draw->GetScreenH();

	if (w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	Initialize();

	if (!CFG::Visuals_Paint_Active)
	{
		m_mapPositions.clear();
		return;
	}

	auto pLocal = H::Entities->GetLocal();

	if (!pLocal)
	{
		m_mapPositions.clear();
		return;
	}

	auto pRenderContext = I::MaterialSystem->GetRenderContext();

	if (!pRenderContext)
	{
		m_mapPositions.clear();
		return;
	}

	m_pBloomAmount->SetIntValue(CFG::Visuals_Paint_Bloom_Amount);

	if (!pLocal->deadflag() && !I::EngineVGui->IsGameUIVisible() && !I::MatSystemSurface->IsCursorVisible() && !SDKUtils::BInEndOfMatch())
	{
		static int nOldTick = I::GlobalVars->tickcount;

		if (I::GlobalVars->tickcount != nOldTick)
		{
			static int nTick = 0;

			if (H::Input->IsDown(CFG::Visuals_Paint_Key))
			{
				if (!nTick)
					nTick = TIME_TO_TICKS(I::EngineClient->Time());

				Vec3 vForward = {};
				Math::AngleVectors(I::EngineClient->GetViewAngles(), &vForward);

				Vec3 vStart = pLocal->GetShootPos();
				Vec3 vEnd = vStart + (vForward * 9001.0f);

				Ray_t ray = {};
				ray.Init(vStart, vEnd);
				trace_t trace = {};
				CTraceFilterWorldCustom filter = {};

				I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

				m_mapPositions[nTick].push_back({trace.endpos, I::GlobalVars->curtime});
			}

			else
			{
				nTick = 0;
			}

			if (H::Input->IsPressed(CFG::Visuals_Paint_Erase_Key))
			{
				if (!m_mapPositions.empty())
					m_mapPositions.clear();
			}

			nOldTick = I::GlobalVars->tickcount;
		}
	}

	bool bDrewSomething = false;

	pRenderContext->PushRenderTargetAndViewport();
	{
		pRenderContext->SetRenderTarget(m_pRenderBuffer0);
		pRenderContext->Viewport(0, 0, w, h);
		pRenderContext->ClearColor4ub(0, 0, 0, 0);
		pRenderContext->ClearBuffers(true, false, false);

		I::ModelRender->ForcedMaterialOverride(m_pMatGlowColor);

		for (auto it = m_mapPositions.begin(); it != m_mapPositions.end(); ++it)
		{
			auto& v = it->second;

			if (v.empty())
			{
				m_mapPositions.erase(it);
				continue;
			}

			if (v.size() > 1)
			{
				for (size_t n = 1; n < v.size(); n++)
				{
					auto flLifeTime = CFG::Visuals_Paint_LifeTime;
					if (flLifeTime != 0.f)
					{
						if (fabsf(I::GlobalVars->curtime - v[n].TimeAdded) > flLifeTime)
						{
							v.erase(v.begin() + n);
							continue;
						}

						if (fabsf(I::GlobalVars->curtime - v[n - 1].TimeAdded) > flLifeTime)
						{
							v.erase(v.begin() + (n - 1));
							continue;
						}
					}

					RenderLine(v[n].Position, v[n - 1].Position, Rainbow(n), false);
				}

				bDrewSomething = true;
			}
		}

		I::ModelRender->ForcedMaterialOverride(nullptr);
	}
	pRenderContext->PopRenderTargetAndViewport();

	if (bDrewSomething)
	{
		pRenderContext->PushRenderTargetAndViewport();
		{
			pRenderContext->Viewport(0, 0, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer1);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurX, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer0);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurY, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		}
		pRenderContext->PopRenderTargetAndViewport();

		ShaderStencilState_t sEffect = {};
		sEffect.m_bEnable = true;
		sEffect.m_nWriteMask = 0x0;
		sEffect.m_nTestMask = 0xFF;
		sEffect.m_nReferenceValue = 0;
		sEffect.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
		sEffect.m_PassOp = STENCILOPERATION_KEEP;
		sEffect.m_FailOp = STENCILOPERATION_KEEP;
		sEffect.m_ZFailOp = STENCILOPERATION_KEEP;
		sEffect.SetStencilState(pRenderContext);

		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
	}

	ShaderStencilState_t stencilStateDisable = {};
	stencilStateDisable.m_bEnable = false;
	stencilStateDisable.SetStencilState(pRenderContext);
}

void CPaint::CleanUp()
{
	m_mapPositions.clear();

	if (m_pMatHaloAddToScreen)
	{
		m_pMatHaloAddToScreen->DecrementReferenceCount();
		m_pMatHaloAddToScreen->DeleteIfUnreferenced();
		m_pMatHaloAddToScreen = nullptr;
	}

	if (m_pRenderBuffer0)
	{
		m_pRenderBuffer0->DecrementReferenceCount();
		m_pRenderBuffer0->DeleteIfUnreferenced();
		m_pRenderBuffer0 = nullptr;
	}

	if (m_pRenderBuffer1)
	{
		m_pRenderBuffer1->DecrementReferenceCount();
		m_pRenderBuffer1->DeleteIfUnreferenced();
		m_pRenderBuffer1 = nullptr;
	}

	if (m_pMatBlurX)
	{
		m_pMatBlurX->DecrementReferenceCount();
		m_pMatBlurX->DeleteIfUnreferenced();
		m_pMatBlurX = nullptr;
	}

	if (m_pMatBlurY)
	{
		m_pMatBlurY->DecrementReferenceCount();
		m_pMatBlurY->DeleteIfUnreferenced();
		m_pMatBlurY = nullptr;
	}
}
