#include "YAMLAction.h"

namespace Glory::Editor
{
	YAMLAction::YAMLAction(Utils::YAMLFileRef& file, const std::filesystem::path& propertyPath, YAML::Node oldValue, YAML::Node newValue)
		: m_File(file), m_PropertyPath(propertyPath), m_OldValue(oldValue), m_NewValue(newValue)
	{
	}

	void YAMLAction::OnUndo(const ActionRecord& actionRecord)
	{
		Utils::NodeValueRef node = m_File[m_PropertyPath];
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
		Undo::TriggerChangeHandler(m_File, m_PropertyPath);
	}

	void YAMLAction::OnRedo(const ActionRecord& actionRecord)
	{
		Utils::NodeValueRef node = m_File[m_PropertyPath];
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
		Undo::TriggerChangeHandler(m_File, m_PropertyPath);
	}
}