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
#include "Serializer.h"
#include "EntitySceneSerializer.h"
#include "EntitySceneObjectSerializer.h"
#include "TransformSerializer.h"
#include <fstream>

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

	EntitySceneObject* EntitySceneScenesModule::CreateDeserializedObject(GScene* pScene, const std::string& name, UUID uuid)
	{
		return (EntitySceneObject*)CreateObject(pScene, name, uuid);
	}

	void EntitySceneScenesModule::Initialize()
	{
		Serializer::RegisterSerializer<EntitySceneSerializer>();
		Serializer::RegisterSerializer<EntitySceneObjectSerializer>();
		Serializer::RegisterSerializer<TransformSerializer>();

		YAML::Emitter out;

		out << YAML::BeginMap;

		size_t hash = ResourceType::GetHash(typeid(Object));
		out << YAML::Key << "Object";
		out << YAML::Value << hash;
		hash = ResourceType::GetHash(typeid(GScene));
		out << YAML::Key << "GScene";
		out << YAML::Value << hash;
		hash = ResourceType::GetHash(typeid(EntityScene));
		out << YAML::Key << "EntityScene";
		out << YAML::Value << hash;
		hash = ResourceType::GetHash(typeid(SceneObject));
		out << YAML::Key << "SceneObject";
		out << YAML::Value << hash;
		hash = ResourceType::GetHash(typeid(EntitySceneObject));
		out << YAML::Key << "EntitySceneObject";
		out << YAML::Value << hash;
		hash = ResourceType::GetHash(typeid(EntityComponentObject));
		out << YAML::Key << "EntityComponentObject";
		out << YAML::Value << hash;
		hash = ResourceType::GetHash(typeid(Transform));
		out << YAML::Key << "Transform";
		out << YAML::Value << hash;

		out << YAML::EndMap;

		std::ofstream outStream("hashes.txt");
		outStream << out.c_str();
		outStream.close();
	}

	void EntitySceneScenesModule::PostInitialize()
	{
		// dis is a test pls ignore
		EntityScene* pScene = (EntityScene*)CreateEmptyScene();
		EntitySceneObject* pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();
		pObject = (EntitySceneObject*)pScene->CreateEmptyObject();

		FileImportSettings importSettings;
		importSettings.Flags = (int)(std::ios::ate | std::ios::binary);
		
		ModelImportSettings modelImportSettings;
		modelImportSettings.m_Extension = "obj";
		
		FileData* pFile = (FileData*)m_pEngine->GetLoaderModule<FileData>()->Load("./Models/viking_room.obj", importSettings);
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
		
		Entity entity = pObject->GetEntityHandle();
		entity.AddComponent<MeshFilter>(pModel);
		MeshRenderer& meshRenderer = entity.AddComponent<MeshRenderer>(pMaterialData);
	}

	void EntitySceneScenesModule::OnCleanup()
	{
	}
}
