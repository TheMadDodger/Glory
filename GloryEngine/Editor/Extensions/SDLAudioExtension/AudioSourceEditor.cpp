#include "AudioSourceEditor.h"

#include <EditorApplication.h>
#include <Audio3DModule.h>

namespace Glory::Editor
{
	AudioSourceEditor::AudioSourceEditor()
	{
	}

	AudioSourceEditor::~AudioSourceEditor()
	{
	}

	void AudioSourceEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
	}

	bool AudioSourceEditor::OnGUI()
	{
		Undo::StartRecord("Property Change", m_pComponentObject->GetUUID(), true);
		bool change = false;

		const uint32_t hash = ResourceTypes::GetHash<AudioSource>();
		const TypeData* pTypeData = Reflect::GetTyeData(hash);
		AudioSource& component = GetTargetComponent();

		Audio3DModule* p3DAudio = EditorApplication::GetInstance()->GetEngine()->GetOptionalModule<Audio3DModule>();

		PropertyDrawer::SetDisabledCheckCallback([&](const Utils::Reflect::FieldData* pField) {
			switch (pField->Offset())
			{
			case offsetof(AudioSource, m_Enable3D):
				return !p3DAudio;
			case offsetof(AudioSource, m_Occlusion):
				return !(component.m_Enable3D && p3DAudio && p3DAudio->HasFeature(Audio3DModule::Features::Occlusion));

			default:
				return false;
			}
		});

		if (pTypeData)
		{
			const Utils::ECS::EntityID entity = m_pComponentObject->EntityID();
			Utils::ECS::BaseTypeView* pTypeView = m_pComponentObject->GetRegistry()->GetTypeView(hash);
			bool active = pTypeView->IsActive(entity);
			if (EditorUI::CheckBox("Active", &active))
			{
				pTypeView->SetActive(entity, active);
				change = true;
			}

			PropertyDrawer::DrawProperty("", pTypeData, &component, 0);
		}
		PropertyDrawer::SetDisabledCheckCallback();

		Undo::StopRecord();

		if (change) Validate();
		return change;
	}

	std::string AudioSourceEditor::Name()
	{
		return "Audio Source";
	}
}
