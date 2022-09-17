#include "PropertyAction.h"
#include "SerializedPropertyManager.h"

namespace Glory::Editor
{
	BasePropertyAction::BasePropertyAction(const std::string& propertyName) : m_PropertyName(propertyName)
	{
	}

	BasePropertyAction::~BasePropertyAction()
	{
	}

	void BasePropertyAction::OnUndo(const ActionRecord& actionRecord)
	{
		SerializedProperty* pProperty = SerializedPropertyManager::FindProperty(actionRecord.ObjectID, m_PropertyName);
		if (pProperty == nullptr) return;
		void* pMember = pProperty->MemberPointer();
		SetOldValue(pMember);
	}

	void BasePropertyAction::OnRedo(const ActionRecord& actionRecord)
	{
		SerializedProperty* pProperty = SerializedPropertyManager::FindProperty(actionRecord.ObjectID, m_PropertyName);
		if (pProperty == nullptr) return;
		void* pMember = pProperty->MemberPointer();
		SetNewValue(pMember);
	}
}
