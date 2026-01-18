#include "SoundMaterialEditor.h"

#include <EditorUI.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
	SoundMaterialEditor::SoundMaterialEditor()
	{
	}

	SoundMaterialEditor::~SoundMaterialEditor()
	{
	}

	bool SoundMaterialEditor::OnGUI()
	{
		YAMLResource<SoundMaterialData>* pSoundMaterialData = (YAMLResource<SoundMaterialData>*)m_pTarget;
		Utils::YAMLFileRef& yamlFile = pSoundMaterialData->File();
		Utils::NodeValueRef rootNode = **pSoundMaterialData;

		bool change = false;
		change |= EditorUI::InputFloat3(yamlFile, rootNode["Absorption"].Path(), 0.f, 1.f, 0.01f);
		EditorUI::Tooltip("Fraction of sound energy absorbed at low, middle, high frequencies."
			"\nBetween 0.0 and 1.0.");
		change |= EditorUI::InputFloat(yamlFile, rootNode["Scattering"].Path(), 0.f, 1.f, 0.01f);
		EditorUI::Tooltip("Fraction of sound energy scattered in a random direction on reflection."
			"\nBetween 0.0 (pure specular) and 1.0(pure diffuse).");
		change |= EditorUI::InputFloat3(yamlFile, rootNode["Transmission"].Path(), 0.f, 1.f, 0.01f);
		EditorUI::Tooltip("Fraction of sound energy transmitted through at low, middle, high frequencies."
			"\nBetween 0.0 and 1.0. Only used for direct occlusion calculations.");

		if (change)
		{
			pSoundMaterialData->SetDirty(true);
		}
		return change;
	}
}
