#include <VertexHelpers.h>
#include <FileLoaderModule.h>
#include <ios>
#include <Material.h>
#include <Engine.h>
#include "EntitySceneScenesModule.h"
#include "Entity.h"
#include "Systems.h"
#include "ModelLoaderModule.h"
#include "ImageLoaderModule.h"
#include "EntitySceneObject.h"
#include <Engine.h>

namespace Glory
{
	EntitySceneScenesModule::EntitySceneScenesModule()
	{
	}

	EntitySceneScenesModule::~EntitySceneScenesModule()
	{
	}

	GScene* EntitySceneScenesModule::CreateScene(const std::string& sceneName)
	{
		return new EntityScene(sceneName);
	}

	GScene* EntitySceneScenesModule::CreateScene(const std::string& sceneName, UUID uuid)
	{
		return new EntityScene(sceneName, uuid);
	}

	void EntitySceneScenesModule::Initialize()
	{
	}

	void EntitySceneScenesModule::PostInitialize()
	{
		// dis is a test pls ignore
		EntityScene* pScene = (EntityScene*)CreateEmptyScene();
		EntitySceneObject* pObject1 = (EntitySceneObject*)pScene->CreateEmptyObject();

		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		
		ModelImportSettings modelImportSettings;
		modelImportSettings.m_Extension = "fbx";
		
		FileData* pFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Models/monkeh.fbx", importSettings);
		ModelData* pModel = (ModelData*)m_pEngine->GetModule<ModelLoaderModule>()->Load(pFile->Data(), pFile->Size(), modelImportSettings);
		delete pFile;
		
		ImageImportSettings imageImportSettings;
		imageImportSettings.m_Extension = "png";
		
		pFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Resources/viking_room_1.png", importSettings);
		ImageData* pTexture = (ImageData*)m_pEngine->GetModule<ImageLoaderModule>()->Load(pFile->Data(), pFile->Size());
		delete pFile;
		
		importSettings.AddNullTerminateAtEnd = true;
		FileData* pVert = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/texturetest.vert", importSettings);
		FileData* pFrag = (FileData*)m_pEngine->GetModule<FileLoaderModule>()->Load("./Shaders/texturetest.frag", importSettings);
		
		std::vector<FileData*> pShaderFiles = { pVert, pFrag };
		std::vector<ShaderType> shaderTypes = { ShaderType::ST_Vertex, ShaderType::ST_Fragment };
		
		MaterialData* pMaterialData = new MaterialData(pShaderFiles, shaderTypes);
		pMaterialData->SetTexture(pTexture);
		
		Entity& entity = pObject1->GetEntityHandle();
		entity.AddComponent<LookAt>(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		entity.AddComponent<CameraComponent>();

		entity = ((EntitySceneObject*)pScene->CreateEmptyObject())->GetEntityHandle();
		entity.AddComponent<LookAt>(glm::vec3(-2.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		entity.AddComponent<CameraComponent>(45.0f, 0.1f, 10.0f, 0, -5);

		for (int i = -4; i < 4; i++)
		{
			for (int j = -4; j < 4; j++)
			{
				EntitySceneObject* pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
				Entity& entity1 = pObject->GetEntityHandle();
				entity1.GetComponent<Transform>().Position = glm::vec3(i * 5.0f, 0.0f, j * 5.0f);
				entity1.AddComponent<LayerComponent>();
				entity1.AddComponent<MeshFilter>(pModel);
				entity1.AddComponent<MeshRenderer>(pMaterialData);
			}
		}
	}

	void EntitySceneScenesModule::OnCleanup()
	{
	}
}
