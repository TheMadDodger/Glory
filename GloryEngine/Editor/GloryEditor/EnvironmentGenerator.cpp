#include "EnvironmentGenerator.h"

#include "AssetPicker.h"
#include "EditorUI.h"

#include <RendererModule.h>
#include <AssetManager.h>
#include <CubemapData.h>
#include <GraphicsModule.h>

namespace Glory
{
	struct CameraRef;
	struct RenderFrame;
}

namespace Glory::Editor
{
	static constexpr char* EnvironmentPassName = "Environment Generator Pass";

	EnvironmentGenerator::EnvironmentGenerator() :
		EditorWindowTemplate("Environment Generator", 600.0f, 600.0f), m_CurrentCubemap(0), m_Generate(false)
	{
	}

	EnvironmentGenerator::~EnvironmentGenerator()
	{
	}

	void EnvironmentGenerator::OnGUI()
	{
		AssetManager& assets = EditorApplication::GetInstance()->GetEngine()->GetAssetManager();
		if (AssetPicker::ResourceDropdown("Cubemap", ResourceTypes::GetHash<CubemapData>(), &m_CurrentCubemap))
		{

		}

		Resource* pResource = m_CurrentCubemap ? assets.FindResource(m_CurrentCubemap) : nullptr;
		CubemapData* pCubemap = pResource ? static_cast<CubemapData*>(pResource) : nullptr;

		ImGui::BeginDisabled(m_CurrentCubemap == 0 || !pCubemap || m_Generate);
		if (ImGui::Button("Generate Environment Maps"))
		{
			m_Generate = true;
		}
		ImGui::EndDisabled();
	}

	void EnvironmentGenerator::OnOpen()
	{
		RendererModule* pRenderer = EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>();
		pRenderer->AddRenderPass(RenderPassType::RP_Postpass, { EnvironmentPassName, [this](CameraRef, const RenderFrame&) { EnvironmentPass(); } });
	}

	void EnvironmentGenerator::OnClose()
	{
		RendererModule* pRenderer = EditorApplication::GetInstance()->GetEngine()->GetMainModule<RendererModule>();
		pRenderer->RemoveRenderPass(RenderPassType::RP_Postpass, EnvironmentPassName);
	}

	void EnvironmentGenerator::EnvironmentPass()
	{
		if (!m_Generate) return;

		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		AssetManager& assets = pEngine->GetAssetManager();

		Resource* pResource = m_CurrentCubemap ? assets.FindResource(m_CurrentCubemap) : nullptr;
		CubemapData* pCubemap = pResource ? static_cast<CubemapData*>(pResource) : nullptr;

		if (!pCubemap)
		{
			m_Generate = false;
			return;
		}



		m_Generate = false;
	}
}
