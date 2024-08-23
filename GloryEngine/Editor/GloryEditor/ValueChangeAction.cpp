#include "ValueChangeAction.h"
#include "EditorApplication.h"

#include <Reflection.h>
#include <ObjectManager.h>
#include <PropertySerializer.h>

#include <Debug.h>

using namespace Glory::Utils::Reflect;

namespace Glory::Editor
{
	ValueChangeAction::ValueChangeAction(const TypeData* pType, const std::filesystem::path& path)
		: m_pRootType(pType), m_PropertyPath(path)
	{
	}

	ValueChangeAction::~ValueChangeAction()
	{
	}

	void ValueChangeAction::SetOldValue(void* pObject)
	{
		if (!m_pRootType) return;

		const FieldData* pField = nullptr;
		const TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = Reflect::GetArrayElementData(pField, index);
			}
			else
				pField = pType->GetFieldData(pathStr);

			if (!pField) return;

			pType = Reflect::GetTyeData(pField->ArrayElementType());
		}

		if (!pField) return;

		auto value = m_OldValue.RootNodeRef().ValueRef();
		value.SetMap();
		EditorApplication::GetInstance()->GetEngine()->GetSerializers().SerializeProperty(pField, pObject, value);
	}

	void ValueChangeAction::SetNewValue(void* pObject)
	{
		if (!m_pRootType) return;

		const FieldData* pField = nullptr;
		const TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = Reflect::GetArrayElementData(pField, index);
			}
			else
				pField = pType->GetFieldData(pathStr);

			if (!pField) return;

			pType = Reflect::GetTyeData(pField->ArrayElementType());
		}

		if (!pField) return;

		auto value = m_NewValue.RootNodeRef().ValueRef();
		value.SetMap();
		EditorApplication::GetInstance()->GetEngine()->GetSerializers().SerializeProperty(pField, pObject, value);
	}

	void ValueChangeAction::OnUndo(const ActionRecord& actionRecord)
	{
		if (!m_pRootType) return;

		Object* pObject = EditorApplication::GetInstance()->GetEngine()->GetObjectManager().Find(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();

		const FieldData* pField = nullptr;
		const TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = Reflect::GetArrayElementData(pField, index);
				if (!pField) return;
				pType = Reflect::GetTyeData(pField->ArrayElementType());

				/* Get element address */
				pAddress = Reflect::ElementAddress(pAddress, pType->TypeHash(), index);
			}
			else
			{
				pField = pType->GetFieldData(pathStr);
				if (!pField) return;
				pType = Reflect::GetTyeData(pField->ArrayElementType());

				/* Get field address */
				pAddress = pField->GetAddress(pAddress);
			}
		}

		if (!pField)
		{
			throw std::exception("Unknown field");
		}

		EditorApplication::GetInstance()->GetEngine()->GetSerializers().DeserializeProperty(pField, pAddress, m_OldValue[pField->Name()]);
	}

	void ValueChangeAction::OnRedo(const ActionRecord& actionRecord)
	{
		if (!m_pRootType) return;

		Object* pObject = EditorApplication::GetInstance()->GetEngine()->GetObjectManager().Find(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();

		const FieldData* pField = nullptr;
		const TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = Reflect::GetArrayElementData(pField, index);
				if (!pField) return;
				pType = Reflect::GetTyeData(pField->ArrayElementType());

				/* Get element address */
				pAddress = Reflect::ElementAddress(pAddress, pType->TypeHash(), index);
			}
			else
			{
				pField = pType->GetFieldData(pathStr);
				if (!pField) return;
				pType = Reflect::GetTyeData(pField->ArrayElementType());

				/* Get field address */
				pAddress = pField->GetAddress(pAddress);
			}
		}

		if (!pField)
		{
			throw std::exception("Unknown field");
		}

		EditorApplication::GetInstance()->GetEngine()->GetSerializers().DeserializeProperty(pField, pAddress, m_NewValue[pField->Name()]);
	}

	bool ValueChangeAction::Combine(IAction* pOther)
	{
		ValueChangeAction* pAction = dynamic_cast<ValueChangeAction*>(pOther);
		if (!pAction) return false;
		if (pAction->m_pRootType != m_pRootType) return false;
		if (pAction->m_PropertyPath != m_PropertyPath) return false;
		m_NewValue = std::move(pAction->m_NewValue);
		return true;
	}
}
