#include "YAMLAction.h"
#include "EditorApplication.h"
#include "EditorResourceManager.h"
#include "EditableResource.h"
#include "EditorAssetDatabase.h"
#include "EditorSceneManager.h"

namespace Glory::Editor
{
	YAMLAction::YAMLAction(Utils::YAMLFileRef& file, const std::filesystem::path& propertyPath, YAML::Node oldValue, YAML::Node newValue)
		: m_File(file), m_PropertyPath(propertyPath), m_OldValue(oldValue), m_NewValue(newValue)
	{
	}

	YAMLResourceBase* GetResource(UUID id, bool& skipAction)
	{
		static uint32_t sceneHash = ResourceTypes::GetHash<GScene>();

		EditorResourceManager& resources = EditorApplication::GetInstance()->GetResourceManager();
		EditorSceneManager& scenes = EditorApplication::GetInstance()->GetSceneManager();
		YAMLResourceBase* pYAMLResource = nullptr;
		if (!id) return nullptr;
		ResourceMeta meta;
		if (!EditorAssetDatabase::GetAssetMetadata(id, meta)) return nullptr;
		if (meta.Hash() == sceneHash)
		{
			YAMLResourceBase* pResource = scenes.GetSceneFile(id);
			skipAction = !pResource;
			return pResource;
		}

		EditableResource* pResource = resources.GetEditableResource(id);
		if (pResource && pResource->IsEditable())
		{
			return static_cast<YAMLResourceBase*>(pResource);
		}
		return nullptr;
	}

	void YAMLAction::OnUndo(const ActionRecord& actionRecord)
	{
		bool skipAction = false;
		YAMLResourceBase* pYAMLResource = GetResource(actionRecord.ObjectID, skipAction);
		if (skipAction) return;

		Utils::NodeValueRef node = pYAMLResource ? (**pYAMLResource)[m_PropertyPath] : m_File[m_PropertyPath];
		if (m_OldValue.IsNull())
		{
			node.Erase();
			Undo::TriggerChangeHandler(m_File, m_PropertyPath);
			return;
		}
		if (m_NewValue.IsNull() && node.Parent().IsSequence())
		{
			node.Parent().PushBack(m_OldValue);
			Undo::TriggerChangeHandler(m_File, m_PropertyPath);
			return;
		}
		node.Set(m_OldValue);
		Undo::TriggerChangeHandler(pYAMLResource ? (**pYAMLResource) : m_File, m_PropertyPath);
	}

	void YAMLAction::OnRedo(const ActionRecord& actionRecord)
	{
		bool skipAction = false;
		YAMLResourceBase* pYAMLResource = GetResource(actionRecord.ObjectID, skipAction);
		if (skipAction) return;

		Utils::NodeValueRef node = pYAMLResource ? (**pYAMLResource)[m_PropertyPath] : m_File[m_PropertyPath];
		if (m_NewValue.IsNull())
		{
			node.Erase();
			Undo::TriggerChangeHandler(m_File, m_PropertyPath);
			return;
		}
		if (m_OldValue.IsNull() && node.Parent().IsSequence())
		{
			node.Parent().PushBack(m_NewValue);
			Undo::TriggerChangeHandler(m_File, m_PropertyPath);
			return;
		}
		node.Set(m_NewValue);
		Undo::TriggerChangeHandler(pYAMLResource ? (**pYAMLResource) : m_File, m_PropertyPath);
	}

	bool YAMLAction::Combine(IAction* pOther)
	{
		YAMLAction* pOtherYAMLAction = dynamic_cast<YAMLAction*>(pOther);
		if (!pOtherYAMLAction) return false;
		if (m_PropertyPath != pOtherYAMLAction->m_PropertyPath) return false;
		if (pOtherYAMLAction->m_NewValue.Type() != m_NewValue.Type()) return false;
		m_NewValue = pOtherYAMLAction->m_NewValue;
	}
}