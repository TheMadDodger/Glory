#include "MonoScript.h"
#include "MonoScriptObjectManager.h"
#include "MonoAssetManager.h"
#include "MonoSceneManager.h"
#include "ScriptingMethodsHelper.h"
#include "MonoManager.h"
#include "Assembly.h"
#include "AssemblyDomain.h"

#include <ScenesModule.h>
#include <AssetDatabase.h>
#include <Reflection.h>
#include <PropertySerializer.h>
#include <AssetReference.h>

#include <mono/jit/jit.h>

namespace Glory
{
	MonoScript::MonoScript()
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::MonoScript(FileData* pFileData) : Script(pFileData)
	{
		APPEND_TYPE(MonoScript);
	}

	MonoScript::~MonoScript()
	{
	}

	void MonoScript::Invoke(Object* pObject, const std::string& method, void** args)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		AssemblyDomain* pDomain = MonoManager::Instance()->ActiveDomain();
		Assembly* pAssembly = pDomain->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(pObject, pClass->m_pClass);
		if (pMonoObject == nullptr) return;
		std::string fullMethodName = ".::" + method;
		MonoMethod* pMethod = pClass->GetMethod(fullMethodName);
		if (pMethod == nullptr) return;
		MonoObject* pException = nullptr;
		pDomain->InvokeMethod(pMethod, pMonoObject, &pException, args);
	}

	void MonoScript::InvokeSafe(Object* pObject, const std::string& method, std::vector<void*>& args)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(pObject, pClass->m_pClass);
		if (pMonoObject == nullptr) return;
		Game::GetGame().GetEngine()->GetScriptingModule<GloryMonoScipting>()->GetMonoManager()->GetMethodsHelper()->InvokeScriptingMethod(pMonoObject, method, args);
	}

	void MonoScript::SetValue(Object* pObject, const std::string& name, void* value)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(pObject, pClass->m_pClass);
		if (pMonoObject == nullptr) return;
		const AssemblyClassField* pField = pClass->GetField(name);
		if (pField == nullptr) return;
		pField->SetValue(pMonoObject, value);
	}

	void MonoScript::GetValue(Object* pObject, const std::string& name, void* value)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		MonoObject* pMonoObject = LoadObject(pObject, pClass->m_pClass);
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
		MonoObject* pDummyObject = pDomain->ScriptObjectManager()->GetScriptDummyObject(pClass->m_pClass);
		if (!data.IsDefined() || !data.IsMap())
			data = YAML::Node(YAML::NodeType::Map);

		scriptProperties.clear();
		for (size_t i = 0; i < pClass->NumFields(); i++)
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
				AssetReferenceBase assetReference;
				MonoObject* pMonoObject;
				pField->GetValue(pDummyObject, &pMonoObject);
				if (pMonoObject)
				{
					MonoClassField* pIDField = mono_class_get_field_from_name(mono_object_get_class(pMonoObject), "_objectID");
					mono_field_get_value(pMonoObject, pIDField, assetReference.AssetUUIDMember());
				}

				PropertySerializer::GetSerializer(ST_Asset)->Serialize(pField->Name(), &assetReference, pField->ElementTypeHash(), e);
				break;
			}
			case ST_Object:
			{
				SceneObjectRef objectRef;
				MonoObject* pMonoObject;
				pField->GetValue(pDummyObject, &pMonoObject);
				if (pMonoObject)
				{
					MonoClassField* pIDField = mono_class_get_field_from_name(mono_object_get_class(pMonoObject), "_objectID");
					MonoClassField* pSceneIDField = mono_class_get_field_from_name(mono_object_get_class(pMonoObject), "_sceneID");
					mono_field_get_value(pMonoObject, pIDField, objectRef.ObjectUUIDMember());
					mono_field_get_value(pMonoObject, pSceneIDField, objectRef.SceneUUIDMember());
				}

				PropertySerializer::GetSerializer(ST_Object)->Serialize(pField->Name(), &objectRef, pField->ElementTypeHash(), e);
				break;
			}

			default:
				Reflect::CreateAsTemporary(pField->ElementTypeHash(), [&](void* value) {
					const TypeData* pType = Reflect::GetTyeData(pField->ElementTypeHash());
					pField->GetValue(pDummyObject, value);
					PropertySerializer::SerializeProperty(pField->Name(), pType, value, e);
				});
				break;
			}
			e << YAML::EndMap;
			YAML::Node newNode = YAML::Load(e.c_str());
			data[pField->Name()] = newNode[pField->Name()];
		}
	}

	void MonoScript::SetPropertyValues(Object* pObject, YAML::Node& node)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;

		MonoObject* pMonoObject = LoadObject(pObject, pClass->m_pClass);
		if (pMonoObject == nullptr) return;

		for (size_t i = 0; i < pClass->NumFields(); i++)
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
				PropertySerializer::GetSerializer(ST_Asset)->Deserialize(&assetReference, pField->ElementTypeHash(), valueNode);
				MonoObject* pAssetObject = MonoAssetManager::MakeMonoAssetObject(assetReference.AssetUUID(), pField->TypeName());
				pField->SetValue(pMonoObject, pAssetObject);
				break;
			}
			case ST_Object:
			{
				SceneObjectRef objectRef;
				PropertySerializer::GetSerializer(ST_Object)->Deserialize(&objectRef, pField->ElementTypeHash(), valueNode);

				GScene* pScene = Game::GetGame().GetEngine()->GetMainModule<ScenesModule>()->GetOpenScene(objectRef.SceneUUID());
				if (!pScene) continue;
				MonoSceneObjectManager* pObjectManager = MonoSceneManager::GetSceneObjectManager(pScene);
				if (!pObjectManager) continue;
				SceneObject* pSceneObject = pScene->FindSceneObject(objectRef.ObjectUUID());
				if (!pSceneObject) continue;
				MonoObject* pMonoSceneObject = pObjectManager->GetSceneObject(pSceneObject);
				pField->SetValue(pMonoObject, pMonoSceneObject);
				break;
			}

			default:
				Reflect::CreateAsTemporary(pField->TypeHash(), [&](void* value) {
					const TypeData* pType = Reflect::GetTyeData(pField->ElementTypeHash());
					PropertySerializer::DeserializeProperty(pType, value, valueNode);
					pField->SetValue(pMonoObject, value);
				});
				break;
			}
		}
	}

	void MonoScript::GetPropertyValues(Object* pObject, YAML::Node& node)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;

		MonoObject* pMonoObject = LoadObject(pObject, pClass->m_pClass);
		if (pMonoObject == nullptr) return;

		YAML::Emitter emitter;
		emitter << YAML::BeginMap;
		for (size_t i = 0; i < pClass->NumFields(); i++)
		{
			const AssemblyClassField* pField = pClass->GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;


			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				AssetReferenceBase assetReference;
				MonoObject* pMonoResourceObject = nullptr;
				pField->GetValue(pMonoObject, &pMonoResourceObject);

				if (pMonoResourceObject)
				{
					MonoClass* pMonoClass = mono_object_get_class(pMonoResourceObject);
					MonoClassField* pIDField = mono_class_get_field_from_name(pMonoClass, "_objectID");
					mono_field_get_value(pMonoResourceObject, pIDField, assetReference.AssetUUIDMember());
				}

				PropertySerializer::GetSerializer(ST_Asset)->Serialize(pField->Name(), &assetReference, pField->ElementTypeHash(), emitter);

				break;
			}
			case ST_Object:
			{
				SceneObjectRef objectRef;
				MonoObject* pMonoSceneObject;
				pField->GetValue(pMonoObject, &pMonoSceneObject);
				if (pMonoSceneObject)
				{
					/* TODO: Get these fields once instead of every frame? */
					MonoClassField* pIDField = mono_class_get_field_from_name(mono_object_get_class(pMonoSceneObject), "_objectID");
					MonoClassField* pSceneIDField = mono_class_get_field_from_name(mono_object_get_class(pMonoSceneObject), "_sceneID");
					mono_field_get_value(pMonoSceneObject, pIDField, objectRef.ObjectUUIDMember());
					mono_field_get_value(pMonoSceneObject, pSceneIDField, objectRef.SceneUUIDMember());
				}
				PropertySerializer::GetSerializer(ST_Object)->Serialize(pField->Name(), &objectRef, pField->ElementTypeHash(), emitter);
				break;
			}

			default:
				Reflect::CreateAsTemporary(pField->TypeHash(), [&](void* value) {
					const TypeData* pType = Reflect::GetTyeData(pField->ElementTypeHash());
					pField->GetValue(pMonoObject, value);
					PropertySerializer::SerializeProperty(pField->Name(), pType, value, emitter);
				});
				break;
			}
		}
		emitter << YAML::EndMap;

		/* FIXME: Causes a memory leak for some reason */
		node = YAML::Load(emitter.c_str());
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

	MonoObject* MonoScript::LoadObject(Object* pObject, MonoClass* pClass)
	{
		return MonoManager::Instance()->ActiveDomain()->ScriptObjectManager()->GetScriptObject(pClass, pObject);
	}
}
