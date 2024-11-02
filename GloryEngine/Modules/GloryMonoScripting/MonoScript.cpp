#include "MonoScript.h"
#include "MonoScriptObjectManager.h"
#include "ScriptingMethodsHelper.h"
#include "MonoManager.h"
#include "Assembly.h"
#include "AssemblyDomain.h"
#include "CoreLibManager.h"

#include <Engine.h>
#include <GScene.h>
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

	void MonoScript::LoadScriptProperties()
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;
		AssemblyDomain* pDomain = MonoManager::Instance()->ActiveDomain();
		Assembly* pAssembly = pDomain->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);

		if (pClass == nullptr) return;

		// Dummy object for default values
		MonoObject* pDummyObject = pDomain->ScriptObjectManager()->GetMonoScriptDummyObject(pClass->m_pClass);

		m_ScriptProperties.clear();
		m_DefaultValues.clear();

		size_t offset = 0;
		for (size_t i = 0; i < pClass->NumFields(); ++i)
		{
			const AssemblyClassField* pField = pClass->GetField(i);
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
				m_DefaultValues.resize(m_DefaultValues.size() + prop.m_Size);
				break;
			}
			case ST_Object:
			{
				/* Object reference from a dummy object will always be null */
				prop.m_Size = sizeof(SceneObjectRef);
				m_DefaultValues.resize(m_DefaultValues.size() + prop.m_Size);
				break;
			}
			default:
				m_DefaultValues.resize(m_DefaultValues.size() + prop.m_Size);
				pField->GetValue(pDummyObject, &m_DefaultValues[prop.m_RelativeOffset]);
				break;
			}

			offset += prop.m_Size;
			m_ScriptProperties.push_back(prop);
		}
	}

	void MonoScript::SetPropertyValues(UUID objectID, UUID sceneID, std::vector<char>& data)
	{
		if (!MonoManager::Instance()->ScriptExecutionAllowed()) return;

		Assembly* pAssembly = MonoManager::Instance()->ActiveDomain()->GetMainAssembly();
		AssemblyClass* pClass = LoadClass(pAssembly, m_NamespaceName, m_ClassName);
		if (pClass == nullptr) return;

		MonoObject* pMonoObject = LoadObject(objectID, sceneID, pClass->m_pClass);
		if (pMonoObject == nullptr) return;

		Engine* pEngine = MonoManager::Instance()->Module()->GetEngine();

		size_t scriptPropIndex = 0;
		for (size_t i = 0; i < pClass->NumFields(); ++i)
		{
			const AssemblyClassField* pField = pClass->GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;
			const ScriptProperty& prop = m_ScriptProperties[scriptPropIndex];

			if (data.size() < prop.m_RelativeOffset + prop.m_Size) break;
			switch (pField->TypeHash())
			{
			case ST_Asset:
			{
				const AssetReferenceBase& assetReference = reinterpret_cast<const AssetReferenceBase&>(data[prop.m_RelativeOffset]);
				MonoObject* pAssetObject = MonoManager::Instance()->GetCoreLibManager()->CreateAssetObject(assetReference.AssetUUID(), pField->TypeName());
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

	void MonoScript::GetPropertyValues(UUID objectID, UUID sceneID, std::vector<char>& data)
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

		size_t scriptPropIndex = 0;
		for (size_t i = 0; i < pClass->NumFields(); ++i)
		{
			const AssemblyClassField* pField = pClass->GetField(i);
			if (pField->FieldVisibility() != Visibility::VISIBILITY_PUBLIC || pField->IsStatic()) continue;

			const ScriptProperty& prop = m_ScriptProperties[scriptPropIndex];
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
				pField->GetValue(pMonoObject, &pMonoSceneObject);
				if (pMonoSceneObject)
				{
					pObjectIDField->GetValue(pMonoSceneObject, objectRef.ObjectUUIDMember());
					pSceneIDField->GetValue(pMonoSceneObject, objectRef.SceneUUIDMember());
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

	void MonoScript::GetScriptProperties(std::vector<ScriptProperty>& dest) const
	{
		if (m_ScriptProperties.empty() || m_DefaultValues.empty()) return;

		if (dest.size() < m_ScriptProperties.size())
			dest.resize(m_ScriptProperties.size());

		for (size_t i = 0; i < m_ScriptProperties.size(); ++i)
			dest[i] = m_ScriptProperties[i];
	}

	const std::vector<ScriptProperty>& MonoScript::ScriptProperties() const
	{
		return m_ScriptProperties;
	}

	void MonoScript::ReadDefaults(std::vector<char>& dest) const
	{
		if (m_ScriptProperties.empty() || m_DefaultValues.empty()) return;

		if (dest.size() < m_DefaultValues.size())
			dest.resize(m_DefaultValues.size());
		else
			return;

		std::memcpy(dest.data(), m_DefaultValues.data(), m_DefaultValues.size());
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
