#include "EnvironmentGenerator.h"

#include "AssetPicker.h"
#include "EditorUI.h"
#include "Importer.h"
#include "EditorAssetDatabase.h"

#include <RendererModule.h>
#include <AssetManager.h>
#include <CubemapData.h>
#include <GraphicsModule.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Glory
{
	struct CameraRef;
	struct RenderFrame;
}

namespace Glory::Editor
{
	static constexpr char* EnvironmentPassName = "Environment Generator Pass";

	EnvironmentGenerator::EnvironmentGenerator() :
		EditorWindowTemplate("Environment Generator", 600.0f, 600.0f), m_CurrentCubemap(0), m_Generate(false), m_pIrradianceResult(nullptr),
		m_pFaces{
			new float[32*32*3],
			new float[32*32*3],
			new float[32*32*3],
			new float[32*32*3],
			new float[32*32*3],
			new float[32*32*3],
		}
	{
	}

	EnvironmentGenerator::~EnvironmentGenerator()
	{
		for (size_t i = 0; i < 6; ++i)
		{
			delete[] m_pFaces[i];
		}
	}

	void EnvironmentGenerator::OnGUI()
	{
		AssetManager& assets = EditorApplication::GetInstance()->GetEngine()->GetAssetManager();
		if (AssetPicker::ResourceDropdown("Cubemap", ResourceTypes::GetHash<CubemapData>(), &m_CurrentCubemap))
		{
			AssetLocation location;
			if (EditorAssetDatabase::GetAssetLocation(m_CurrentCubemap, location))
			{
				std::filesystem::path path = location.Path;
				m_Filename = path.filename().replace_extension().string();
				m_OutputPath = path.parent_path().string();
			}
		}

		Resource* pResource = m_CurrentCubemap ? assets.FindResource(m_CurrentCubemap) : nullptr;
		CubemapData* pCubemap = pResource ? static_cast<CubemapData*>(pResource) : nullptr;

		if (m_CurrentCubemap != 0 && !pCubemap)
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Cubemap not yet compiled!");
		else if (m_CurrentCubemap == 0)
			ImGui::TextUnformatted("Select a cubemap");

		ImGui::BeginDisabled(m_CurrentCubemap == 0 || !pCubemap || m_Generate);
		if (ImGui::Button("Generate Environment Maps"))
		{
			m_Generate = true;
		}
		ImGui::EndDisabled();
	}

	void EnvironmentGenerator::OnOpen()
	{
		Engine* pEngine = EditorApplication::GetInstance()->GetEngine();
		RendererModule* pRenderer = pEngine->GetMainModule<RendererModule>();
		GraphicsModule* pGraphics = pEngine->GetMainModule<GraphicsModule>();
		pRenderer->AddRenderPass(RenderPassType::RP_Postpass, { EnvironmentPassName, [this](CameraRef, const RenderFrame&) { EnvironmentPass(); } });

		RenderTextureCreateInfo textureInfo;
		textureInfo.HasDepth = true;
		textureInfo.HasStencil = false;
		textureInfo.Width = 32;
		textureInfo.Height = 32;
		textureInfo.Attachments.push_back(Attachment("Color", PixelFormat::PF_RGB, PixelFormat::PF_R16G16B16Sfloat, ImageType::IT_2D, ImageAspect::IA_Color, DataType::DT_Float));
		m_pIrradianceResult = pGraphics->GetResourceManager()->CreateRenderTexture(textureInfo);
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

		Texture* pCubemapTexture = pGraphics->GetResourceManager()->CreateCubemapTexture(pCubemap);
		if (!pCubemapTexture)
		{
			m_Generate = false;
			return;
		}

		MaterialData* pIrradianceMaterial = pRenderer->GetInternalMaterial("irradiance");
		if (!pIrradianceMaterial)
		{
			m_Generate = false;
			return;
		}

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		Material* pMaterial = pGraphics->UseMaterial(pIrradianceMaterial);
		if (!pMaterial)
		{
			m_Generate = false;
			return;
		}

		ObjectData object;
		object.Model = glm::identity<glm::mat4>();
		object.Projection = captureProjection;
		object.SceneID = 0;
		object.ObjectID = 0;

		pMaterial->SetSubemapTexture("cubemap", pCubemapTexture);
		for (size_t i = 0; i < 6; ++i)
		{
			/* ReadColorPixels() unbinds the buffer so we should rebind is every iteration */
			m_pIrradianceResult->BindForDraw();
			/* Render irradiance */
			object.View = captureViews[i];
			pMaterial->SetObjectData(object);
			pGraphics->Clear();
			pGraphics->DrawUnitCube();

			/* Read pixels to buffer */
			m_pIrradianceResult->ReadColorPixels("Color", m_pFaces[i], DataType::DT_Float);
		}
		m_pIrradianceResult->UnBindForDraw();

		static constexpr std::string_view sides[6] = {
			"Right",
			"Left",
			"Down",
			"Up",
			"Front",
			"Back",
		};

		/* Generate images */
		ImageData* pImages[6];
		for (size_t i = 0; i < 6; ++i)
		{
			pImages[i] = new ImageData(32, 32, PixelFormat::PF_R16G16B16Sfloat, PixelFormat::PF_RGB,
				3*sizeof(float), std::move((char*)m_pFaces[i]), 32*32*3*sizeof(float), false, DataType::DT_Float);
			m_pFaces[i] = new float[32*32*3];
			std::filesystem::path path = m_OutputPath;
			path.append(m_Filename + "_irradiance_" + sides[i].data()).replace_extension(".hdr");
			path = EditorAssetDatabase::GetAbsoluteAssetPath(path.string());
			Importer::Export(path, pImages[i]);
			ImportedResource importedResource{ path, pImages[i] };
			TextureData* pDefault = new TextureData(pImages[i]);
			importedResource.AddChild(pDefault, "Default");
			EditorAssetDatabase::ImportAsset(path.string(), importedResource);
		}

		/* Generate cubemap */
		CubemapData* pIrradianceMap = new CubemapData(pImages[0], pImages[1], pImages[2], pImages[3], pImages[4], pImages[5]);
		SamplerSettings& sampler = pIrradianceMap->GetSamplerSettings();
		sampler.MipmapMode = Filter::F_None;
		sampler.AddressModeU = SamplerAddressMode::SAM_ClampToEdge;
		sampler.AddressModeV = SamplerAddressMode::SAM_ClampToEdge;
		sampler.AddressModeW = SamplerAddressMode::SAM_ClampToEdge;
		std::filesystem::path path = m_OutputPath;
		path.append(m_Filename + "_irradiance").replace_extension(".gcube");
		path = EditorAssetDatabase::GetAbsoluteAssetPath(path.string());
		EditorAssetDatabase::CreateAsset(pIrradianceMap, path.string());

		m_Generate = false;
	}
}
