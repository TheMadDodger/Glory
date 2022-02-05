#include "DefaultComponentEditor.h"
#include <imgui.h>
#include <PropertyDrawer.h>

namespace Glory::Editor
{
	DefaultComponentEditor::DefaultComponentEditor()
	{
	}

	DefaultComponentEditor::~DefaultComponentEditor()
	{
	}

	void DefaultComponentEditor::OnGUI()
	{
		for (size_t i = 0; i < m_Properties.size(); i++)
		{
			PropertyDrawer::DrawProperty(m_Properties[i]);
		}
	}

	void DefaultComponentEditor::Initialize()
	{
		EntityComponentObject* pEntityComponentObject = (EntityComponentObject*)m_pTarget;
		m_Properties.clear();
		m_Name = pEntityComponentObject->GetRegistry()->GetSystems()->AcquireSerializedProperties(pEntityComponentObject->GetComponentData(), m_Properties);
	}

	std::string DefaultComponentEditor::Name()
	{
		return m_Name;
	}
}
