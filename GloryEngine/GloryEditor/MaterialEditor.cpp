#include "MaterialEditor.h"
#include <imgui.h>
#include <ResourceType.h>
#include "PropertyDrawer.h"

namespace Glory::Editor
{
	MaterialEditor::MaterialEditor() {}

	MaterialEditor::~MaterialEditor() {}

	void MaterialEditor::OnGUI()
	{
		MaterialData* pMaterial = (MaterialData*)m_pTarget;

		std::vector<MaterialPropertyData> properties;
		pMaterial->CopyProperties(properties);

		for (size_t i = 0; i < properties.size(); i++)
		{
			MaterialPropertyData* pProperty = &properties[i];
			size_t hash = ResourceType::GetHash(pProperty->Type());
			PropertyDrawer::DrawProperty(pProperty->Name(), pProperty->Data(), pProperty->Flags());
		}

		pMaterial->PasteProperties(properties);
	}
}
