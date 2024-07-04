#include "SoundOccluderEditor.h"

#include <AssetManager.h>

#include <EditorApplication.h>
#include <EditorResourceManager.h>

namespace Glory::Editor
{
	SoundOccluderEditor::SoundOccluderEditor(): m_pSoundMaterialEditor(nullptr)
	{
	}

	SoundOccluderEditor::~SoundOccluderEditor()
	{
		if (!m_pSoundMaterialEditor) return;
		Editor::ReleaseEditor(m_pSoundMaterialEditor);
		m_pSoundMaterialEditor = nullptr;
	}

	void SoundOccluderEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
	}

	bool SoundOccluderEditor::OnGUI()
	{
		Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
		bool change = false;
		const TypeData* pTypeData = SoundOccluder::GetTypeData();
		const FieldData* pMaterialAssetField = pTypeData->GetFieldData("m_MaterialAsset");
		const FieldData* pMaterialField = pTypeData->GetFieldData("m_Material");

		const Utils::ECS::EntityID entity = m_pComponentObject->EntityID();
		Utils::ECS::BaseTypeView* pTypeView = m_pComponentObject->GetRegistry()->GetTypeView(pTypeData->TypeHash());
		bool active = pTypeView->IsActive(entity);
		if (EditorUI::CheckBox("Active", &active))
		{
			pTypeView->SetActive(entity, active);
			change = true;
		}

		SoundOccluder& occluder = GetTargetComponent();
		PropertyDrawer::DrawProperty(pMaterialAssetField, &occluder.m_MaterialAsset, 0);

		const bool hasMaterialAsset = occluder.m_MaterialAsset.AssetUUID() != 0;
		ImGui::BeginDisabled(hasMaterialAsset);
		if (!hasMaterialAsset)
			PropertyDrawer::DrawProperty(pMaterialField, &occluder.m_Material, 0);
		else
		{
			EditableResource* pResource = EditorApplication::GetInstance()->GetResourceManager().GetEditableResource(occluder.m_MaterialAsset.AssetUUID());

			if (pResource)
			{
				if (!m_pSoundMaterialEditor)
					m_pSoundMaterialEditor = Editor::CreateEditor((Object*)pResource);
				m_pSoundMaterialEditor->SetTarget((Object*)pResource);

				m_pSoundMaterialEditor->OnGUI();
			}
		}
		ImGui::EndDisabled();

		Undo::StopRecord();

		if (change) Validate();
		return change;


		return EntityComponentEditor::OnGUI();
	}

	std::string SoundOccluderEditor::Name()
	{
		return "Sound Occluder";
	}
}
