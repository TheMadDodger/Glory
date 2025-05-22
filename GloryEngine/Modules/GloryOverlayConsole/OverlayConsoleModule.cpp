#include "OverlayConsoleModule.h"

#include <WindowModule.h>
#include <RendererModule.h>
#include <Console.h>
#include <MaterialManager.h>
#include <Engine.h>
#include <AssetManager.h>
#include <GraphicsModule.h>
#include <GPUResourceManager.h>
#include <CameraManager.h>
#include <InternalMaterial.h>
#include <InternalPipeline.h>
#include <FontData.h>
#include <FontDataStructs.h>
#include <Material.h>
#include <MaterialData.h>
#include <GameTime.h>

#include <DistributedRandom.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Glory
{
	GLORY_MODULE_VERSION_CPP(OverlayConsoleModule);

	OverlayConsoleModule::OverlayConsoleModule():
		m_ConsoleButtonDown(false), m_ConsoleOpen(false), m_ConsoleAnimationTime(0.0f),
		m_TextDirty(false), m_Scroll(0.0f)
	{
	}

	OverlayConsoleModule::~OverlayConsoleModule()
	{
	}

	void OverlayConsoleModule::CollectReferences(std::vector<UUID>& references)
	{
		ModuleSettings& settings = Settings();

		std::vector<UUID> newReferences;
		newReferences.push_back(settings.Value<uint64_t>("Console Background Pipeline"));
		newReferences.push_back(settings.Value<uint64_t>("Console Text Pipeline"));

		for (size_t i = 0; i < newReferences.size(); ++i)
		{
			if (!newReferences[i]) continue;
			references.push_back(newReferences[i]);
			Resource* pPipelineResource = m_pEngine->GetAssetManager().GetAssetImmediate(newReferences[i]);
			if (!pPipelineResource) continue;
			PipelineData* pPipelineData = static_cast<PipelineData*>(pPipelineResource);
			for (size_t i = 0; i < pPipelineData->ShaderCount(); ++i)
			{
				const UUID shaderID = pPipelineData->ShaderID(i);
				if (!shaderID) continue;
				references.push_back(shaderID);
			}
		}
		references.push_back(settings.Value<uint64_t>("Console Font"));
	}

	const std::type_info& OverlayConsoleModule::GetModuleType()
	{
		return typeid(OverlayConsoleModule);
	}

	bool OverlayConsoleModule::OnOverrideInputEvent(InputEvent& e)
	{
		const UUID consoleFont = Settings().Value<uint64_t>("Console Font");
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(consoleFont);
		if (!pResource) return false;
		FontData* pFont = static_cast<FontData*>(pResource);

		int windowWidth, windowHeight;
		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();
		pWindows->GetMainWindow()->GetDrawableSize(&windowWidth, &windowHeight);
		const float textScale = 0.5f*windowWidth/1920.0f;

		switch (e.State)
		{
		case InputState::KeyDown:
			break;
		case InputState::KeyUp:
			break;
		case InputState::Axis:
			if (e.InputDeviceType != InputDeviceType::Mouse) return true;
			if (e.KeyID != MouseAxis::MouseAxisScrollY) return true;
			m_Scroll += e.Delta*pFont->FontHeight()*textScale;
			m_TextDirty = true;
			break;
		}

		return true;
	}

	void OverlayConsoleModule::Initialize()
	{
	}

	void OverlayConsoleModule::PostInitialize()
	{
		RendererModule* pRenderer = m_pEngine->GetMainModule<RendererModule>();
		if (!pRenderer)
		{
			m_pEngine->GetDebug().LogError("OverlayConsole does not work without a renderer");
			return;
		}

		pRenderer->AddRenderPass(RenderPassType::RP_Postblit, { "Console Pass", [this](CameraRef, const RenderFrame&) {
			OverlayPass();
		} });

		m_pEngine->GetConsole().RegisterConsole(this);

		const ModuleSettings& settings = Settings();
		const UUID consoleBackgroundPipeline = settings.Value<uint64_t>("Console Background Pipeline");
		const UUID consoleTextPipeline = settings.Value<uint64_t>("Console Text Pipeline");

		m_pConsoleBackgroundMaterial = new MaterialData();
		m_pConsoleBackgroundMaterial->SetPipeline(consoleBackgroundPipeline);
		m_pConsoleTextMaterial = new MaterialData();
		m_pConsoleTextMaterial->SetPipeline(consoleTextPipeline);

		m_pConsoleMesh.reset(new MeshData(4, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));

		m_pConsoleLogTextMesh.reset(new MeshData(0, sizeof(VertexPosColorTex),
			{ AttributeType::Float2, AttributeType::Float3, AttributeType::Float2 }));

		const float xpos = 0.0f;
		const float ypos = 0.0f;

		const float w = 1.0f;
		const float h = 1.0f;

		const glm::vec4 color{ 0.0f, 0.0f, 0.0f, 1.0f };

		VertexPosColorTex vertices[4] = {
			{ { xpos, ypos + h, }, color, {0.0f, 0.0f}},
			{ { xpos, ypos, }, color, { 0.0f, 1.0f } },
			{ { xpos + w, ypos, }, color, { 1.0f, 1.0f } },
			{ { xpos + w, ypos + h, }, color, { 1.0f, 0.0f }, }
		};

		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[0]));
		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[1]));
		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[2]));
		m_pConsoleMesh->AddVertex(reinterpret_cast<float*>(&vertices[3]));
		m_pConsoleMesh->AddFace(0, 1, 2, 3);
	}

	void OverlayConsoleModule::Update()
	{
		REQUIRE_MODULE(m_pEngine, WindowModule, );

		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();
		Window* pMainWindow = pWindows->GetMainWindow();
		if (pMainWindow->IsBackQuoteDown() && !m_ConsoleButtonDown)
		{
			m_ConsoleButtonDown = true;
			m_ConsoleOpen = !m_ConsoleOpen;
			if (m_ConsoleOpen)
				pMainWindow->AddInputOverrideHandler(this);
			else
				pMainWindow->RemoveInputOverrideHandler(this);
		}
		else if (!pMainWindow->IsBackQuoteDown())
			m_ConsoleButtonDown = false;

		if (m_ConsoleOpen && m_ConsoleAnimationTime < 1.0f)
			m_ConsoleAnimationTime += m_pEngine->Time().GetDeltaTime()*10.0f;
		else if (m_ConsoleOpen) m_ConsoleAnimationTime = 1.0f;
		else if (!m_ConsoleOpen && m_ConsoleAnimationTime > 0.0f)
			m_ConsoleAnimationTime -= m_pEngine->Time().GetDeltaTime()*10.0f;
		else if (!m_ConsoleOpen) m_ConsoleAnimationTime = 0.0f;
	}

	void OverlayConsoleModule::Cleanup()
	{
		delete m_pConsoleBackgroundMaterial;
		m_pConsoleBackgroundMaterial = nullptr;

		delete m_pConsoleTextMaterial;
		m_pConsoleTextMaterial = nullptr;
	}

	void OverlayConsoleModule::OverlayPass()
	{
		/* Do not render if console is fully closed */
		Console& console = m_pEngine->GetConsole();
		if (!m_ConsoleOpen && m_ConsoleAnimationTime == 0.0f || console.LineCount() == 0) return;

		const UUID consoleFont = Settings().Value<uint64_t>("Console Font");
		Resource* pResource = m_pEngine->GetAssetManager().FindResource(consoleFont);
		if (!pResource) return;
		FontData* pFont = static_cast<FontData*>(pResource);

		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		GPUResourceManager* pGPUResourceManager = pGraphics->GetResourceManager();
		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();

		int windowWidth, windowHeight;
		pWindows->GetMainWindow()->GetWindowSize(&windowWidth, &windowHeight);

		const float textScale = 0.5f*windowWidth/1920.0f;
		const float consolePadding = 10.0f;
		const float consoleHeight = 20.0f*pFont->FontHeight()*textScale + consolePadding;
		const float animatedConsoleHeight = consoleHeight*(1.0f - m_ConsoleAnimationTime);
		const float textHeight = pFont->FontHeight()*textScale*console.LineCount();
		const float textStart = glm::max(consoleHeight - textHeight - consolePadding, 0.0f);
		const float maxScroll = glm::max(textHeight + consolePadding - consoleHeight, 0.0f);
		m_Scroll = glm::clamp(m_Scroll, 0.0f, maxScroll);

		if (m_TextDirty)
		{
			m_pConsoleLogTextMesh->ClearIndices();
			m_pConsoleLogTextMesh->ClearVertices();

			const size_t maxVisibleLines = size_t(std::floor(consoleHeight/(pFont->FontHeight()*textScale)));
			const size_t lastLine = console.LineCount() - size_t(std::floor(m_Scroll/(pFont->FontHeight()*textScale)));
			const size_t firstLine = size_t(glm::max(int(lastLine - maxVisibleLines), 0));

			for (size_t i = 0; i < lastLine - firstLine; ++i)
			{
				TextRenderData textData;
				textData.m_Color = console.LineColor(firstLine + i);
				textData.m_Text = console.Line(firstLine + i);
				textData.m_FontID = consoleFont;
				textData.m_TextWrap = 0.0f;
				textData.m_Alignment = Alignment::Left;
				textData.m_Scale = textScale;
				textData.m_Offsets.y = -1.0f*pFont->FontHeight()*textScale*(i+1);
				textData.m_Append = true;
				Utils::GenerateTextMesh(m_pConsoleLogTextMesh.get(), pFont, textData);
				m_pConsoleLogTextMesh->SetDirty(true);
			}
			m_TextDirty = false;
		}

		ObjectData object;
		const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, windowHeight - consoleHeight + animatedConsoleHeight, 0.0f));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(float(windowWidth), float(consoleHeight), 1.0f));
		object.Model = translation*scale;
		object.Projection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight));

		pGraphics->EnableDepthTest(false);

		Material* pMaterial = pGraphics->UseMaterial(m_pConsoleBackgroundMaterial);
		pMaterial->SetProperties(m_pEngine);
		pMaterial->SetObjectData(object);

		pGraphics->EnableStencilTest(true);
		pGraphics->SetStencilMask(0xFF);
		pGraphics->ClearStencil(0);
		pGraphics->SetStencilOP(Func::OP_Replace, Func::OP_Replace, Func::OP_Replace);
		pGraphics->SetStencilFunc(CompareOp::OP_Always, 255, 0xFF);
		pGraphics->DrawMesh(m_pConsoleMesh.get(), 0, m_pConsoleMesh->VertexCount());

		pGraphics->SetStencilMask(0x00);
		pGraphics->SetStencilOP(Func::OP_Keep, Func::OP_Keep, Func::OP_Keep);
		pGraphics->SetStencilFunc(CompareOp::OP_Equal, 255, 0xFF);
		pMaterial = pGraphics->UseMaterial(m_pConsoleTextMaterial);
		pMaterial->SetProperties(m_pEngine);
		object.Model = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, windowHeight + animatedConsoleHeight - textStart, 0.0f));
		pMaterial->SetObjectData(object);

		InternalTexture* pTextureData = pFont->GetGlyphTexture();
		if (!pTextureData) return;

		Texture* pTexture = pGPUResourceManager->CreateTexture((TextureData*)pTextureData);
		if (pTexture) pMaterial->SetTexture("textSampler", pTexture);

		pGraphics->DrawMesh(m_pConsoleLogTextMesh.get(), 0, m_pConsoleLogTextMesh->VertexCount());

		/* Reset render textures and materials */
		pGraphics->SetStencilMask(0x00);
		pGraphics->EnableStencilTest(false);
		pGraphics->UseMaterial(nullptr);
		pGraphics->EnableDepthTest(true);
	}

	void OverlayConsoleModule::OnConsoleClose()
	{
	}

	void OverlayConsoleModule::SetNextColor(const glm::vec4& color)
	{
	}

	void OverlayConsoleModule::ResetNextColor()
	{
	}

	void OverlayConsoleModule::Write(const std::string& line)
	{
		m_TextDirty = true;
	}

	void OverlayConsoleModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("Console Background Pipeline", 113);
		settings.RegisterAssetReference<PipelineData>("Console Text Pipeline", 115);
		settings.RegisterAssetReference<FontData>("Console Font", 116);
	}
}
