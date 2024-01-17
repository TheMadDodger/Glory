#include "NodeValueChangeAction.h"
#include "EditorApplication.h"

#include <Reflection.h>
#include <ObjectManager.h>
#include <PropertySerializer.h>

#include <Debug.h>

namespace Glory::Editor
{
	NodeValueChangeAction::NodeValueChangeAction(const std::filesystem::path& propertyPath, YAML::Node oldValue, YAML::Node newValue)
		: m_PropertyPath(propertyPath), m_OldValue(oldValue), m_NewValue(newValue)
	{
	}

	NodeValueChangeAction::~NodeValueChangeAction()
	{
	}

	void NodeValueChangeAction::OnUndo(const ActionRecord& actionRecord)
	{
		Object* pObject = EditorApplication::GetInstance()->GetEngine()->GetObjectManager().Find(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();
		YAML::Node* pNode = (YAML::Node*)pAddress;
		std::vector<std::filesystem::path> splitPath;
		for (auto path : m_PropertyPath)
		{
			splitPath.push_back(path);
		}
		std::reverse(splitPath.begin(), splitPath.end());
		Set(splitPath, *pNode, m_OldValue);
	}

	void NodeValueChangeAction::OnRedo(const ActionRecord& actionRecord)
	{
		Object* pObject = EditorApplication::GetInstance()->GetEngine()->GetObjectManager().Find(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();
		YAML::Node* pNode = (YAML::Node*)pAddress;
		std::vector<std::filesystem::path> splitPath;
		for (auto path : m_PropertyPath)
		{
			splitPath.push_back(path);
		}
		std::reverse(splitPath.begin(), splitPath.end());
		Set(splitPath, *pNode, m_NewValue);
	}

	void NodeValueChangeAction::Set(std::vector<std::filesystem::path>& splitPath, YAML::Node& node, YAML::Node& value)
	{
		std::filesystem::path path = splitPath[splitPath.size() - 1];
		splitPath.pop_back();

		// TODO: Check if path is a number, arrays would crash here otherwise
		YAML::Node& nextNode = node[path.string()];

		if (splitPath.empty())
		{
			nextNode = value;
			return;
		}

		Set(splitPath, nextNode, value);
	}

	bool NodeValueChangeAction::Combine(IAction* pOther)
	{
		NodeValueChangeAction* pAction = dynamic_cast<NodeValueChangeAction*>(pOther);
		if (!pAction) return false;
		if (pAction->m_PropertyPath != m_PropertyPath) return false;
		m_NewValue = std::move(pAction->m_NewValue);
		return true;
	}
}
