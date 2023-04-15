#include "ValueChangeAction.h"
#include <Reflection.h>
#include <PropertySerializer.h>

#include <Debug.h>

namespace Glory::Editor
{
	ValueChangeAction::ValueChangeAction(const GloryReflect::TypeData* pType, const std::filesystem::path& path)
		: m_pRootType(pType), m_PropertyPath(path)
	{
	}

	ValueChangeAction::~ValueChangeAction()
	{
	}

	void ValueChangeAction::SetOldValue(void* pObject)
	{
		const GloryReflect::FieldData* pField = nullptr;
		const GloryReflect::TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = GloryReflect::Reflect::GetArrayElementData(pField, index);
			}
			else
				pField = pType->GetFieldData(pathStr);

			if (!pField) return;

			pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());
		}

		if (!pField) return;

		YAML::Emitter out;
		out << YAML::BeginMap;
		PropertySerializer::SerializeProperty(pField, pObject, out);
		out << YAML::EndMap;
		m_OldValue = YAML::Load(out.c_str());
	}

	void ValueChangeAction::SetNewValue(void* pObject)
	{
		const GloryReflect::FieldData* pField = nullptr;
		const GloryReflect::TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = GloryReflect::Reflect::GetArrayElementData(pField, index);
			}
			else
				pField = pType->GetFieldData(pathStr);

			if (!pField) return;

			pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());
		}

		if (!pField) return;

		YAML::Emitter out;
		out << YAML::BeginMap;
		PropertySerializer::SerializeProperty(pField, pObject, out);
		out << YAML::EndMap;
		m_NewValue = YAML::Load(out.c_str());
	}

	void ValueChangeAction::OnUndo(const ActionRecord& actionRecord)
	{
		Object* pObject = Object::FindObject(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();

		const GloryReflect::FieldData* pField = nullptr;
		const GloryReflect::TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = GloryReflect::Reflect::GetArrayElementData(pField, index);
				if (!pField) return;
				pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());

				/* Get element address */
				pAddress = GloryReflect::Reflect::ElementAddress(pAddress, pType->TypeHash(), index);
			}
			else
			{
				pField = pType->GetFieldData(pathStr);
				if (!pField) return;
				pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());

				/* Get field address */
				pAddress = pField->GetAddress(pAddress);
			}
		}

		if (!pField)
		{
			throw std::exception("Unknown field");
		}

		PropertySerializer::DeserializeProperty(pField, pAddress, m_OldValue[pField->Name()]);
	}

	void ValueChangeAction::OnRedo(const ActionRecord& actionRecord)
	{
		Object* pObject = Object::FindObject(actionRecord.ObjectID);
		void* pAddress = pObject->GetRootDataAddress();

		const GloryReflect::FieldData* pField = nullptr;
		const GloryReflect::TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			const std::string& pathStr = subPath.string();
			if (pathStr._Starts_with("##"))
			{
				if (!pField) throw new std::exception("Unexpected property path");
				const size_t index = (size_t)std::stoul(pathStr.substr(2));
				pField = GloryReflect::Reflect::GetArrayElementData(pField, index);
				if (!pField) return;
				pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());

				/* Get element address */
				pAddress = GloryReflect::Reflect::ElementAddress(pAddress, pType->TypeHash(), index);
			}
			else
			{
				pField = pType->GetFieldData(pathStr);
				if (!pField) return;
				pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());

				/* Get field address */
				pAddress = pField->GetAddress(pAddress);
			}
		}

		if (!pField)
		{
			throw std::exception("Unknown field");
		}

		PropertySerializer::DeserializeProperty(pField, pAddress, m_NewValue[pField->Name()]);
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
