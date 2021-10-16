#include "EntitySceneScenesModule.h"
#include "Entity.h"
#include "Systems.h"
#include <VertexHelpers.h>
#include <FileLoaderModule.h>
#include <ios>
#include <Material.h>
#include "ModelLoaderModule.h"
#include "ImageLoaderModule.h"
#include "MeshRenderSystem.h"
#include <Engine.h>

namespace Glory
{
	EntitySceneScenesModule::EntitySceneScenesModule()
	{
	}

	EntitySceneScenesModule::~EntitySceneScenesModule()
	{
	}

	void EntitySceneScenesModule::Initialize()
	{
		// Register engine components
		m_Scene.m_Registry.RegisterSystem<TransformSystem>();
		m_Scene.m_Registry.RegisterSystem<MeshRenderSystem>();
	}

	void EntitySceneScenesModule::PostInitialize()
	{
		// dis is a test pls ignore

		ModelData* pModel = (ModelData*)m_pEngine->GetModule<ModelLoaderModule>()->Load("./Models/viking_room.obj");
		ImageData* pTexture = (ImageData*)m_pEngine->GetModule<ImageLoaderModule>()->Load("./Resources/viking_room_1.png");
		
		FileImportSettings importSettings;
		importSettings.Flags = (int)std::ios::ate;
		importSettings.AddNullTerminateAtEnd = true;
		FileData* pVert = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/texturetest.vert", importSettings);
		FileData* pFrag = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/texturetest.frag", importSettings);
		
		std::vector<FileData*> pShaderFiles = { pVert, pFrag };
		std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };
		
		MaterialData* pMaterialData = new MaterialData(pShaderFiles, shaderTypes);
		pMaterialData->SetTexture(pTexture);
		
		m_Entity = m_Scene.CreateEntity();
		m_Entity.AddComponent<MeshFilter>(pModel);
		MeshRenderer& meshRenderer = m_Entity.AddComponent<MeshRenderer>(pMaterialData);
	}

	void EntitySceneScenesModule::Cleanup()
	{
	}

	void EntitySceneScenesModule::Tick()
	{
		m_Scene.m_Registry.Update();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}

	void EntitySceneScenesModule::OnPaint()
	{
		m_Scene.m_Registry.Draw();
		//while (m_Scene.m_Registry.IsUpdating()) {}
	}
}
