#include "MonoScript.h"
#include "MonoScriptObjectManager.h"
#include "MonoAssetManager.h"
#include "MonoSceneManager.h"
#include "ScriptingMethodsHelper.h"
#include "MonoManager.h"
#include "Assembly.h"
#include "AssemblyDomain.h"
#include "CoreLibManager.h"

#include <Engine.h>
#include <Serializers.h>
#include <SceneManager.h>
#include <BinaryStream.h>
#include <AssetDatabase.h>
#include <Reflection.h>
#include <PropertySerializer.h>
#include <AssetReference.h>

#include <mono/jit/jit.h>

namespace Glory
{
	char PropertyBuffer[1024] = "\0";

	MonoScript::MonoScript()
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::MonoScript(FileData* pFileData, std::string_view ns, std::string_view className)
		: FileData(pFileData), m_NamespaceName(ns), m_ClassName(className)
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::~MonoScript()
	{
	}

	void MonoScript::Invoke(UUID objectID, UUID sceneID, const std::string& method, void** args)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		AssemblyDomain* pDomain = MonoManager::Instance()->ActiveDomain();
		Assembly* pAssembly = pDomain->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(objectID, sceneID, pClass->m_pClass);
		if (pMonoObject == nullptr) return;
		std::string fullMethodName = ".::" + method;
		MonoMethod* pMethod = pClass->GetMethod(fullMethodName);
		if (pMethod == nullptr) return;
		pDomain->InvokeMethod(pMethod, pMonoObject, args);
	}

	void MonoScript::InvokeSafe(UUID objectID, UUID sceneID, const std::string& method, std::vector<void*>& args)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(objectID, sceneID, pClass->m_pClass);
		if (pMonoObject == nullptr) return;
		MonoManager::Instance()->GetMethodsHelper()->InvokeScriptingMethod(pMonoObject, method, args);
	}

	void MonoScript::SetValue(UUID objectID, UUID sceneID, const std::string& name, void* value)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(objectID, sceneID, pClass->m_pClass);
		if (pMonoObject == nullptr) return;
		const AssemblyClassField* pField = pClass->GetField(name);
		if (pField == nullptr) return;
		pField->SetValue(pMonoObject, value);
	}

	void MonoScript::GetValue(UUID objectID, UUID sceneID, const std::string& name, void* value)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(objectID, sceneID, pClass->m_pClass);
		if (pMonoObject == nullptr) return;
		const AssemblyClassField* pField = pClass->GetField(name);
		if (pField == nullptr) return;
		pField->GetValue(pMonoObject, value);
	}

	void MonoScript::LoadScriptProperties(std::vector<ScriptProperty>& scriptProperties, YAML::Node& data)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		AssemblyDomain* pDomain = MonoManager::Instance()->ActiveDomain();
		Assembly* pAssembly = pDomain->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);

		if (pClass == nullptr) return;

		// Dummy object for default values
		MonoObject* pDummyObject = pDomain->ScriptObjectManager()->GetMonoScriptDummyObject(pClass->m_pClass);
		if (!data.IsDefined() || !data.IsMap())
			data = YAML::Node(YAML::NodeType::Map);

		scriptProperties.clear();

		Serializers& pSerializers = MonoManager::Instance()->Module()->GetEngine()->GetSerializers();
		for (size_t i = 0; i < pClass->NumFields(); ++i)
		{
			const AssemblyClassField* pField = pClass->GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;

			ScriptProperty prop;
			prop.m_Name = pField->Name();
			prop.m_TypeName = pField->TypeName();
			prop.m_TypeHash = pField->TypeHash();
			prop.m_ElementTypeHash = pField->ElementTypeHash();
			prop.m_ChildrenCount = 0;
			prop.m_ChildrenOffset = 0;
			prop.m_Size = pField->Size();
			prop.ValueType = SerializedType::ST_Value;
			scriptProperties.push_back(prop);

			if (data[pField->Name()].IsDefined() || !pDummyObject) continue;

			YAML::Emitter e;
			e << YAML::BeginMap;

			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				/* Asset reference from a dummy object will always be null */
				AssetReferenceBase assetReference{};
				pSerializers.GetSerializer(ST_Asset)->Serialize(pField->Name(), &assetReference, pField->ElementTypeHash(), e);
				break;
			}
			case ST_Object:
			{
				/* Object reference from a dummy object will always be null */
				SceneObjectRef objectRef{};
				pSerializers.GetSerializer(ST_Object)->Serialize(pField->Name(), &objectRef, pField->ElementTypeHash(), e);
				break;
			}

			default:
				pField->GetValue(pDummyObject, PropertyBuffer);
				const TypeData* pType = Reflect::GetTyeData(pField->ElementTypeHash());
				pSerializers.SerializeProperty(pField->Name(), pType, PropertyBuffer, e);
				break;
			}
			e << YAML::EndMap;
			YAML::Node newNode = YAML::Load(e.c_str());
			data[pField->Name()] = newNode[pField->Name()];
		}
	}

	void MonoScript::SetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;

		MonoObject* pMonoObject = LoadObject(objectID, sceneID, pClass->m_pClass);
		if (pMonoObject == nullptr) return;

		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();
		Serializers& pSerializers = pEngine->GetSerializers();
		for (size_t i = 0; i < pClass->NumFields(); ++i)
		{
			const AssemblyClassField* pField = pClass->GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;

			YAML::Node valueNode = node[pField->Name()];
			if (!valueNode.IsDefined()) continue;

			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				AssetReferenceBase assetReference;
				pSerializers.GetSerializer(ST_Asset)->Deserialize(&assetReference, pField->ElementTypeHash(), valueNode);
				MonoObject* pAssetObject = MonoAssetManager::MakeMonoAssetObject(pEngine, assetReference.AssetUUID(), pField->TypeName());
				pField->SetValue(pMonoObject, pAssetObject);
				break;
			}
			case ST_Object:
			{
				SceneObjectRef objectRef;
				pSerializers.GetSerializer(ST_Object)->Deserialize(&objectRef, pField->ElementTypeHash(), valueNode);

				GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(objectRef.SceneUUID());
				if (!pScene) continue;
				MonoSceneObjectManager* pObjectManager = MonoSceneManager::GetSceneObjectManager(pEngine, pScene);
				if (!pObjectManager) continue;
				Entity entity = pScene->GetEntityByUUID(objectRef.ObjectUUID());
				if (!entity.IsValid()) continue;
				MonoObject* pMonoSceneObject = pObjectManager->GetMonoSceneObject(objectRef.ObjectUUID());
				pField->SetValue(pMonoObject, pMonoSceneObject);
				break;
			}

			default:
				const TypeData* pType = Reflect::GetTyeData(pField->ElementTypeHash());
				pSerializers.DeserializeProperty(pType, PropertyBuffer, valueNode);
				pField->SetValue(pMonoObject, PropertyBuffer);
				break;
			}
		}
	}

	void MonoScript::GetPropertyValues(UUID objectID, UUID sceneID, YAML::Node& node)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);

		if (pClass == nullptr) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		Assembly* pCoreAssembly = pCoreLibManager->GetAssemblyBinding();
		AssemblyClass* pObjectClass = pCoreAssembly->GetClass("GloryEngine", "Object");
		AssemblyClass* pSceneObjectClass = pCoreAssembly->GetClass("GloryEngine.SceneManagement", "SceneObject");
		const AssemblyClassField* pObjectIDField = pObjectClass->GetField("_objectID");
		const AssemblyClassField* pSceneIDField = pSceneObjectClass->GetField("_sceneID");

		MonoObject* pMonoObject = LoadObject(objectID, sceneID, pClass->m_pClass);
		if (pMonoObject == nullptr) return;

		Serializers& pSerializers = MonoManager::Instance()->Module()->GetEngine()->GetSerializers();
		/* FIXME: There has to be a better way to do this
		 * Ideally, store as binary data and use reflection? */
		YAML::Emitter emitter;
		emitter << YAML::BeginMap;
		for (size_t i = 0; i < pClass->NumFields(); ++i)
		{
			const AssemblyClassField* pField = pClass->GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;

			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				AssetReferenceBase assetReference{};
				MonoObject* pMonoResourceObject = nullptr;
				pField->GetValue(pMonoObject, &pMonoResourceObject);

				if (pMonoResourceObject)
				{
					pObjectIDField->GetValue(pMonoResourceObject, assetReference.AssetUUIDMember());
				}

				pSerializers.GetSerializer(ST_Asset)->Serialize(pField->Name(), &assetReference, pField->ElementTypeHash(), emitter);

				break;
			}
			case ST_Object:
			{
				SceneObjectRef objectRef{};
				MonoObject* pMonoSceneObject = nullptr;
				pField->GetValue(pMonoObject, &pMonoSceneObject);
				if (pMonoSceneObject)
				{
					pObjectIDField->GetValue(pMonoSceneObject, objectRef.ObjectUUIDMember());
					pSceneIDField->GetValue(pMonoSceneObject, objectRef.SceneUUIDMember());
				}
				pSerializers.GetSerializer(ST_Object)->Serialize(pField->Name(), &objectRef, pField->ElementTypeHash(), emitter);
				break;
			}

			default:
				pField->GetValue(pMonoObject, PropertyBuffer);
				const TypeData* pType = Reflect::GetTyeData(pField->ElementTypeHash());
				pSerializers.SerializeProperty(pField->Name(), pType, PropertyBuffer, emitter);
				break;
			}
		}
		emitter << YAML::EndMap;

		/* FIXME: Causes a memory leak for some reason */
		node.reset(YAML::Load(emitter.c_str()));
	}

	void MonoScript::Serialize(BinaryStream& container) const
	{
		container.Write(m_NamespaceName);
		container.Write(m_ClassName);
	}

	void MonoScript::Deserialize(BinaryStream& container)
	{
		container.Read(m_NamespaceName);
		container.Read(m_ClassName);
	}

	bool MonoScript::IsBehaviour()
	{
		return false;
	}

	AssemblyClass* MonoScript::LoadClass(Assembly* pAssembly, const std::string& namespaceName, const std::string& className)
	{
		if (pAssembly == nullptr) return nullptr;
		AssemblyClass* pClass = pAssembly->GetClass(namespaceName, className);
		if (pClass == nullptr) return nullptr;
		return pClass;
	}

	MonoObject* MonoScript::LoadObject(UUID objectID, UUID sceneID, MonoClass* pClass)
	{
		return MonoManager::Instance()->ActiveDomain()->ScriptObjectManager()->GetMonoScriptObject(pClass, objectID, sceneID);
	}
}
