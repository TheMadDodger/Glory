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
			pField = pType->GetFieldData(subPath.string());
			pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());
		}

		if (!pField)
		{
			throw std::exception("Unknown field");
		}

		// Rewind offset so Get() reads from the correct address
		void* pAddress = (void*)((char*)(pObject)-pField->Offset());
		YAML::Emitter out;
		out << YAML::BeginMap;
		PropertySerializer::SerializeProperty(pField, pAddress, out);
		out << YAML::EndMap;
		m_OldValue = YAML::Load(out.c_str());
	}

	void ValueChangeAction::SetNewValue(void* pObject)
	{
		const GloryReflect::FieldData* pField = nullptr;
		const GloryReflect::TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			pField = pType->GetFieldData(subPath.string());
			pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());
		}

		if (!pField)
		{
			throw std::exception("Unknown field");
		}

		// Rewind offset so Get() reads from the correct address
		void* pAddress = (void*)((char*)(pObject)-pField->Offset());
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

		const GloryReflect::FieldData* pField = nullptr;
		const GloryReflect::TypeData* pType = m_pRootType;
		for (const std::filesystem::path& subPath : m_PropertyPath)
		{
			pField = pType->GetFieldData(subPath.string());
			pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());

			/* Get field address */
			pAddress = pField->GetAddress(pAddress);
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
			pField = pType->GetFieldData(subPath.string());
			pType = GloryReflect::Reflect::GetTyeData(pField->ArrayElementType());

			/* Get field address */
			pAddress = pField->GetAddress(pAddress);
		}

		if (!pField)
		{
			throw std::exception("Unknown field");
		}

		PropertySerializer::DeserializeProperty(pField, pAddress, m_NewValue[pField->Name()]);
	}
}
