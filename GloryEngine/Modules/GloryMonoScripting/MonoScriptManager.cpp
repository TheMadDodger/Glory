#include "MonoScriptManager.h"
#include "ScriptProperty.h"
#include "Assembly.h"
#include "MonoManager.h"
#include "GloryMonoScipting.h"
#include "CoreLibManager.h"
#include "AssemblyDomain.h"

#include <Hash.h>
#include <AssetReference.h>
#include <GScene.h>
#include <SceneManager.h>
#include <SceneObjectRef.h>
#include <Engine.h>

namespace Glory
{
	void MonoScriptManager::Initialize(AssemblyClass* pEngineClass, MonoObject* pEngineObject)
	{
		MonoMethod* pTypeCountMethod = pEngineClass->GetMethod(".::GetScriptTypeCount");
		MonoObject* pExcept;
		MonoObject* result = mono_runtime_invoke(pTypeCountMethod, pEngineObject, nullptr, &pExcept);
		int typeCount = *(int*)mono_object_unbox(result);
		if (pExcept)
			mono_print_unhandled_exception(pExcept);

		m_pScriptTypes.clear();
		m_pScriptTypes.reserve((size_t)typeCount);
		m_pScriptDummies.clear();
		m_pScriptDummies.reserve((size_t)typeCount);
		m_ScriptTypeNames.clear();
		m_ScriptTypeNames.reserve((size_t)typeCount);
		m_ScriptTypeHashes.clear();
		m_ScriptTypeHashes.reserve((size_t)typeCount);
		m_ScriptClasses.clear();
		m_ScriptClasses.reserve((size_t)typeCount);
		m_ScriptProperties.clear();
		m_ScriptProperties.reserve((size_t)typeCount);
		m_DefaultValues.clear();
		m_DefaultValues.reserve((size_t)typeCount);

		MonoMethod* pTypeMethod = pEngineClass->GetMethod(".::GetScriptType");
		MonoMethod* pDummyMethod = pEngineClass->GetMethod(".::GetScriptDummy");
		MonoMethod* pTypeNameMethod = pEngineClass->GetMethod(".::GetScriptTypeName");

		for (int i = 0; i < typeCount; ++i)
		{
			void* args[1] = { &i };
			result = mono_runtime_invoke(pTypeMethod, pEngineObject, args, &pExcept);
			if (pExcept)
			{
				mono_print_unhandled_exception(pExcept);
				return;
			}
			m_pScriptTypes.push_back(result);
			result = mono_runtime_invoke(pDummyMethod, pEngineObject, args, &pExcept);
			if (pExcept)
			{
				mono_print_unhandled_exception(pExcept);
				return;
			}
			MonoClass* pClass = mono_object_get_class(result);
			m_pScriptDummies.push_back(result);
			result = mono_runtime_invoke(pTypeNameMethod, pEngineObject, args, &pExcept);
			if (pExcept)
			{
				mono_print_unhandled_exception(pExcept);
				return;
			}
			char* str = mono_string_to_utf8((MonoString*)result);
			m_ScriptTypeNames.push_back(std::string(str));
			m_ScriptTypeHashes.push_back(Hashing::Hash(m_ScriptTypeNames[i].c_str()));
			mono_free(str);
			m_ScriptClasses.push_back(AssemblyClass(m_ScriptTypeNames[i], pClass));
			LoadScriptProperties(i, m_ScriptClasses[i]);
		}
	}

	int MonoScriptManager::TypeIndexFromName(std::string_view name) const
	{
		for (size_t i = 0; i < m_ScriptTypeNames.size(); ++i)
		{
			const std::string_view type = m_ScriptTypeNames[i];
			if (name != type) continue;
			return i;
		}
		return -1;
	}

	int MonoScriptManager::TypeIndexFromHash(uint32_t hash) const
	{
		for (size_t i = 0; i < m_ScriptTypeNames.size(); ++i)
		{
			if (hash != m_ScriptTypeHashes[i]) continue;
			return i;
		}
		return -1;
	}

	MonoObject* MonoScriptManager::Dummy(size_t index) const
	{
		return m_pScriptDummies[index];
	}

	MonoObject* MonoScriptManager::Type(size_t index) const
	{
		return m_pScriptTypes[index];
	}

	std::string_view MonoScriptManager::TypeName(size_t index) const
	{
		return m_ScriptTypeNames[index];
	}

	uint32_t MonoScriptManager::TypeHash(size_t index) const
	{
		return m_ScriptTypeHashes[index];
	}

	size_t MonoScriptManager::TypeCount() const
	{
		return m_pScriptTypes.size();
	}

	void MonoScriptManager::LoadScriptProperties(size_t index, AssemblyClass& cls)
	{
		// Dummy object for default values
		MonoObject* pDummyObject = m_pScriptDummies[index];

		m_ScriptProperties.push_back(std::vector<ScriptProperty>());
		m_DefaultValues.push_back(std::vector<char>());

		std::vector<ScriptProperty>& properties = m_ScriptProperties[index];
		std::vector<char>& defaultValues = m_DefaultValues[index];
		properties.reserve(cls.NumFields());

		size_t offset = 0;
		for (size_t i = 0; i < cls.NumFields(); ++i)
		{
			const AssemblyClassField* pField = cls.GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;

			ScriptProperty prop;
			prop.m_Name = pField->Name();
			prop.m_TypeName = pField->TypeName();
			prop.m_TypeHash = pField->TypeHash();
			prop.m_ElementTypeHash = pField->ElementTypeHash();
			prop.m_RelativeOffset = offset;
			prop.m_ChildrenCount = 0;
			prop.m_ChildrenOffset = 0;
			prop.m_Size = pField->Size();
			prop.ValueType = (SerializedType)pField->TypeHash();

			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				/* Asset reference from a dummy object will always be null */
				prop.m_Size = sizeof(AssetReferenceBase);
				defaultValues.resize(defaultValues.size() + prop.m_Size);
				break;
			}
			case ST_Object:
			{
				/* Object reference from a dummy object will always be null */
				prop.m_Size = sizeof(SceneObjectRef);
				defaultValues.resize(defaultValues.size() + prop.m_Size);
				break;
			}
			default:
				defaultValues.resize(defaultValues.size() + prop.m_Size);
				pField->GetValue(pDummyObject, &defaultValues[prop.m_RelativeOffset]);
				break;
			}

			offset += prop.m_Size;
			properties.push_back(prop);
		}
	}

	void MonoScriptManager::ReadDefaults(size_t typeIndex, std::vector<char>& dest) const
	{
		const std::vector<char>& defaultValues = m_DefaultValues[typeIndex];
		if (defaultValues.empty()) return;

		if (dest.size() < defaultValues.size())
			dest.resize(defaultValues.size());
		else
			return;

		std::memcpy(dest.data(), defaultValues.data(), defaultValues.size());
	}

	const std::vector<ScriptProperty>& MonoScriptManager::ScriptProperties(size_t typeIndex) const
	{
		return m_ScriptProperties[typeIndex];
	}

	void MonoScriptManager::SetPropertyValues(size_t typeIndex, MonoObject* pMonoObject, std::vector<char>& data) const
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		if (pMonoObject == nullptr) return;

		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();
		const AssemblyClass& cls = m_ScriptClasses[typeIndex];
		const std::vector<ScriptProperty>& properties = m_ScriptProperties[typeIndex];

		size_t scriptPropIndex = 0;
		for (size_t i = 0; i < cls.NumFields(); ++i)
		{
			const AssemblyClassField* pField = cls.GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;
			const ScriptProperty& prop = properties[scriptPropIndex];

			if (data.size() < prop.m_RelativeOffset + prop.m_Size) break;
			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				const AssetReferenceBase& assetReference = reinterpret_cast<const AssetReferenceBase&>(data[prop.m_RelativeOffset]);
				MonoObject* pAssetObject = m_pCoreLibManager->CreateAssetObject(assetReference.AssetUUID(), pField->TypeName());
				pField->SetValue(pMonoObject, pAssetObject);
				break;
			}
			case ST_Object:
			{
				const SceneObjectRef& objectRef = reinterpret_cast<const SceneObjectRef&>(data[prop.m_RelativeOffset]);
				GScene* pScene = pEngine->GetSceneManager()->GetOpenScene(objectRef.SceneUUID());
				if (!pScene) continue;
				Entity entity = pScene->GetEntityByUUID(objectRef.ObjectUUID());
				if (!entity.IsValid()) continue;
				MonoObject* pMonoSceneObject = MonoManager::Instance()->GetCoreLibManager()->CreateSceneObject(objectRef.ObjectUUID(), objectRef.SceneUUID());
				pField->SetValue(pMonoObject, pMonoSceneObject);
				break;
			}

			default:
				pField->SetValue(pMonoObject, &data[prop.m_RelativeOffset]);
				break;
			}
			++scriptPropIndex;
		}
	}

	void MonoScriptManager::GetPropertyValues(size_t typeIndex, MonoObject* pMonoObject, std::vector<char>& data) const
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;

		CoreLibManager* pCoreLibManager = MonoManager::Instance()->GetCoreLibManager();
		Assembly* pCoreAssembly = pCoreLibManager->GetAssemblyBinding();
		AssemblyClass* pObjectClass = pCoreAssembly->GetClass("GloryEngine", "Object");
		AssemblyClass* pSceneObjectClass = pCoreAssembly->GetClass("GloryEngine.SceneManagement", "SceneObject");
		AssemblyClass* pSceneClass = pCoreAssembly->GetClass("GloryEngine.SceneManagement", "Scene");
		const AssemblyClassField* pObjectIDField = pObjectClass->GetField("_objectID");
		const AssemblyClassField* pObjectSceneField = pSceneObjectClass->GetField("_scene");

		if (pMonoObject == nullptr) return;

		const AssemblyClass& cls = m_ScriptClasses[typeIndex];
		const std::vector<ScriptProperty>& properties = m_ScriptProperties[typeIndex];

		size_t scriptPropIndex = 0;
		for (size_t i = 0; i < cls.NumFields(); ++i)
		{
			const AssemblyClassField* pField = cls.GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;

			const ScriptProperty& prop = properties[scriptPropIndex];
			if (data.size() < prop.m_RelativeOffset + prop.m_Size)
				data.resize(data.size() + prop.m_Size);

			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				AssetReferenceBase& assetReference = reinterpret_cast<AssetReferenceBase&>(data[prop.m_RelativeOffset]);
				MonoObject* pMonoResourceObject = nullptr;
				pField->GetValue(pMonoObject, &pMonoResourceObject);

				if (!pMonoResourceObject) break;
				pObjectIDField->GetValue(pMonoResourceObject, assetReference.AssetUUIDMember());
				break;
			}
			case ST_Object:
			{
				SceneObjectRef& objectRef = reinterpret_cast<SceneObjectRef&>(data[prop.m_RelativeOffset]);
				MonoObject* pMonoSceneObject = nullptr;
				MonoObject* pMonoScene = nullptr;
				pField->GetValue(pMonoObject, &pMonoSceneObject);
				if (pMonoSceneObject)
				{
					pObjectIDField->GetValue(pMonoSceneObject, objectRef.ObjectUUIDMember());
					pObjectSceneField->GetValue(pMonoSceneObject, &pMonoScene);
					if (pMonoScene)
					{
						pObjectIDField->GetValue(pMonoScene, objectRef.SceneUUIDMember());
					}
				}
				break;
			}

			default:
				pField->GetValue(pMonoObject, &data[prop.m_RelativeOffset]);
				break;
			}
			++scriptPropIndex;
		}
	}

	void MonoScriptManager::Invoke(size_t typeIndex, MonoObject* pMonoObject, const std::string& method, void** args)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		if (pMonoObject == nullptr) return;
		AssemblyClass& cls = m_ScriptClasses[typeIndex];
		std::string fullMethodName = ".::" + method;
		MonoMethod* pMethod = cls.GetMethod(fullMethodName);
		if (pMethod == nullptr) return;
		MonoManager::Instance()->ActiveDomain()->InvokeMethod(pMethod, pMonoObject, args);
	}

	/*void MonoScript::InvokeSafe(MonoObject* pMonoObject, const std::string& method, std::vector<void*>& args)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		if (pMonoObject == nullptr) return;
		MonoManager::Instance()->GetMethodsHelper()->InvokeScriptingMethod(pMonoObject, method, args);
	}

	/*void MonoScript::SetValue(MonoObject* pMonoObject, const std::string& name, void* value)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		if (pMonoObject == nullptr) return;
		const AssemblyClassField* pField = pClass->GetField(name);
		if (pField == nullptr) return;
		pField->SetValue(pMonoObject, value);
	}

	void MonoScript::GetValue(MonoObject* pMonoObject, const std::string& name, void* value)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;
		if (pMonoObject == nullptr) return;
		const AssemblyClassField* pField = pClass->GetField(name);
		if (pField == nullptr) return;
		pField->GetValue(pMonoObject, value);
	}*/

	/*MonoObject* MonoScript::CreateScriptObject(UUID objectID, UUID sceneID, UUID componentID)
	{
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		return MonoManager::Instance()->GetCoreLibManager()->CreateScript(pClass->m_pClass, sceneID, objectID, componentID);
	}*/

	/*MonoObject* MonoScript::GetScriptObject(UUID objectID, UUID sceneID, UUID componentID)
	{
		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		return MonoManager::Instance()->GetCoreLibManager()->GetScript(sceneID, objectID, componentID);
	}*/

	/*void MonoScript::GetScriptProperties(std::vector<ScriptProperty>& dest) const
	{
		if (m_ScriptProperties.empty() || m_DefaultValues.empty()) return;

		if (dest.size() < m_ScriptProperties.size())
			dest.resize(m_ScriptProperties.size());

		for (size_t i = 0; i < m_ScriptProperties.size(); ++i)
			dest[i] = m_ScriptProperties[i];
	}*/

	/*const std::vector<ScriptProperty>& MonoScript::ScriptProperties() const
	{
		return m_ScriptProperties;
	}*/
}
