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
		m_ConsoleButtonDown(false), m_ConsoleOpen(false), m_ConsoleAnimationTime(0.0f)
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
	}

	const std::type_info& OverlayConsoleModule::GetModuleType()
	{
		return typeid(OverlayConsoleModule);
	}

	MaterialData* OverlayConsoleModule::ConsoleBackgroundMaterial()
	{
		return m_pConsoleBackgroundMaterial;
	}

	MaterialData* OverlayConsoleModule::ConsoleTextMaterial()
	{
		return m_pConsoleTextMaterial;
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

		const float xpos = 0.0f;
		const float ypos = 0.0f;

		const float w = 1.0f;
		const float h = 1.0f;

		const glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

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
		if (pWindows->GetMainWindow()->IsBackQuoteDown() && !m_ConsoleButtonDown)
		{
			m_ConsoleButtonDown = true;
			m_ConsoleOpen = !m_ConsoleOpen;
		}
		else if (!pWindows->GetMainWindow()->IsBackQuoteDown())
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
		//if (!m_ConsoleOpen && m_ConsoleAnimationTime == 0.0f) return;

		GraphicsModule* pGraphics = m_pEngine->GetMainModule<GraphicsModule>();
		WindowModule* pWindows = m_pEngine->GetMainModule<WindowModule>();

		int windowWidth, windowHeight;
		pWindows->GetMainWindow()->GetDrawableSize(&windowWidth, &windowHeight);

		const float height = windowHeight*0.4f;

		ObjectData object;
		const glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, windowHeight - height + height*(1.0f - m_ConsoleAnimationTime), 0.0f));
		const glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), glm::vec3(float(windowWidth), float(height), 1.0f));
		object.Model = translation*scale;
		object.Projection = glm::ortho(0.0f, float(windowWidth), 0.0f, float(windowHeight));

		Material* pMaterial = pGraphics->UseMaterial(m_pConsoleBackgroundMaterial);

		pMaterial->SetProperties(m_pEngine);
		pMaterial->SetObjectData(object);

		pGraphics->UseMaterial(m_pConsoleBackgroundMaterial);
		pGraphics->EnableDepthTest(false);

		pGraphics->DrawMesh(m_pConsoleMesh.get(), 0, m_pConsoleMesh->VertexCount());

		/* Reset render textures and materials */
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
	}

	void OverlayConsoleModule::LoadSettings(ModuleSettings& settings)
	{
		settings.RegisterAssetReference<PipelineData>("Console Background Pipeline", 113);
		settings.RegisterAssetReference<PipelineData>("Console Text Pipeline", 115);
	}
}
