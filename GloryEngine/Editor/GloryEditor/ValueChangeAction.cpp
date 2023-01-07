#include "ValueChangeAction.h"
#include <Reflection.h>
#include <PropertySerializer.h>

#include <Debug.h>

namespace Glory::Editor
{
	ValueChangeAction::ValueChangeAction(const std::vector<const GloryReflect::FieldData*>& pFieldStack)
		: m_pFieldStack(pFieldStack)
	{
	}

	ValueChangeAction::~ValueChangeAction()
	{
	}

	void ValueChangeAction::SetOldValue(void* pObject)
	{
		const GloryReflect::FieldData* pField = m_pFieldStack[m_pFieldStack.size() - 1];

		// Rewind offset so Get() reads from the correct address
		void* pAddress = rewind ? (void*)((char*)(pObject)-pField->Offset()) : pObject;
		YAML::Emitter out;
		out << YAML::BeginMap;
		PropertySerializer::SerializeProperty(pField, pAddress, out);
		out << YAML::EndMap;
		m_OldValue = YAML::Load(out.c_str());
	}

	void ValueChangeAction::SetNewValue(void* pObject)
	{
		const GloryReflect::FieldData* pField = m_pFieldStack[m_pFieldStack.size() - 1];

		// Rewind offset so Get() reads from the correct address
		void* pAddress = rewind ? (void*)((char*)(pObject)-pField->Offset()) : pObject;
		YAML::Emitter out;
		out << YAML::BeginMap;
		PropertySerializer::SerializeProperty(pField, pAddress, out);
		out << YAML::EndMap;
		m_NewValue = YAML::Load(out.c_str());
	}

	void ValueChangeAction::OnUndo(const ActionRecord& actionRecord)
	{
		Object* pObject = Object::FindObject(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();
		for (size_t i = 0; i < m_pFieldStack.size() - 1; i++)
		{
			const GloryReflect::FieldData* pField = m_pFieldStack[i];
			if (pField->Type() == ST_Array)
			{
				/* Get array address */
				pAddress = pField->GetAddress(pAddress);
				/* Get array element address */
				pAddress = GloryReflect::Reflect::ElementAddress(pAddress, pField->ArrayElementType(), 0);
				continue;
			}

			/* Get field address */
			pAddress = pField->GetAddress(pAddress);
		}

		const GloryReflect::FieldData* pField = m_pFieldStack[m_pFieldStack.size() - 1];
		PropertySerializer::DeserializeProperty(pField, pAddress, m_OldValue[pField->Name()]);
	}

	void ValueChangeAction::OnRedo(const ActionRecord& actionRecord)
	{
		Object* pObject = Object::FindObject(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();
		for (size_t i = 0; i < m_pFieldStack.size() - 1; i++)
		{
			const GloryReflect::FieldData* pField = m_pFieldStack[i];
			if (pField->Type() == ST_Array)
			{
				/* Get array address */
				pAddress = pField->GetAddress(pAddress);
				/* Get array element address */
				pAddress = GloryReflect::Reflect::ElementAddress(pAddress, pField->ArrayElementType(), 0);
				continue;
			}

			/* Get field address */
			pAddress = pField->GetAddress(pAddress);
		}

		const GloryReflect::FieldData* pField = m_pFieldStack[m_pFieldStack.size() - 1];
		PropertySerializer::DeserializeProperty(pField, pAddress, m_NewValue[pField->Name()]);
	}
}
