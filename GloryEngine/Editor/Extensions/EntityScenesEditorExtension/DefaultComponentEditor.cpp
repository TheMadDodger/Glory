#include "DefaultComponentEditor.h"
#include <imgui.h>
#include <PropertyDrawer.h>
#include <Undo.h>
#include <TypeData.h>

namespace Glory::Editor
{
	DefaultComponentEditor::DefaultComponentEditor()
	{
	}

	DefaultComponentEditor::~DefaultComponentEditor()
	{
	}

	bool DefaultComponentEditor::OnGUI()
	{
		Undo::StartRecord("Property Change", m_pTarget->GetUUID());
		bool change = false;
		EntityComponentObject* pComponentObject = (EntityComponentObject*)GetTarget();
		uint32_t hash = pComponentObject->ComponentType();
		GloryECS::EntityRegistry* pRegistry = pComponentObject->GetRegistry();
		GloryECS::EntityID entity = pComponentObject->EntityID();
		UUID componentID = pComponentObject->GetUUID();

		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(hash);
		if (pTypeData)
		{
			for (int i = 0; i < pTypeData->FieldCount(); i++)
			{
				const GloryReflect::FieldData* pFieldData = pTypeData->GetFieldData(i);
				size_t offset = pFieldData->Offset();
				void* pAddress = (void*)((char*)(pRegistry->GetComponentAddress(entity, componentID)) + offset);
				change |= PropertyDrawer::DrawProperty(pFieldData, pAddress, GloryReflect::Reflect::GetFieldFlags(pFieldData));
			}
		}

		Undo::StopRecord();
		return change;
	}

	void DefaultComponentEditor::Initialize()
	{
		EntityComponentObject* pEntityComponentObject = (EntityComponentObject*)m_pTarget;
		const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(pEntityComponentObject->ComponentType());
		m_Name = pTypeData->TypeName();
	}

	std::string DefaultComponentEditor::Name()
	{
		return m_Name;
	}
}
