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
		for (size_t i = 0; i < m_Properties.size(); i++)
		{
			change |= PropertyDrawer::DrawProperty(m_Properties[i]);
		}
		Undo::StopRecord();
		return change;
	}

	void DefaultComponentEditor::Initialize()
	{
		EntityComponentObject* pEntityComponentObject = (EntityComponentObject*)m_pTarget;
		m_Properties.clear();
		m_Name = "Tranform";
		//m_Name = pEntityComponentObject->GetRegistry()->GetSystems()->AcquireSerializedProperties(pEntityComponentObject->GetComponentData(), m_Properties);
	}

	std::string DefaultComponentEditor::Name()
	{
		return m_Name;
	}
}
