#include "TextureDataEditor.h"
#include "AssetPicker.h"
#include "EditorAssetDatabase.h"
#include "PropertyDrawer.h"
#include "EditorUI.h"
#include "Tumbnail.h"
#include <imgui.h>
#include <ResourceType.h>

namespace Glory::Editor
{
	TextureDataEditor::TextureDataEditor() {}

	TextureDataEditor::~TextureDataEditor() {}

	bool TextureDataEditor::OnGUI()
	{
		TextureData* pTextureData = (TextureData*)m_pTarget;
		AssetReference<ImageData>& imageRef = pTextureData->Image();
		const bool exists = EditorAssetDatabase::AssetExists(imageRef.AssetUUID());
		ImGui::BeginDisabled(exists);
		bool change = AssetPicker::ResourceDropdown("Image", ResourceType::GetHash<ImageData>(), imageRef.AssetUUIDMember());
		ImGui::EndDisabled();

		SamplerSettings& sampler = pTextureData->GetSamplerSettings();
		change |= EditorUI::InputEnum<Filter>("Mag Filter", &sampler.MagFilter);
		change |= EditorUI::InputEnum<Filter>("Min Filter", &sampler.MinFilter);
		change |= EditorUI::InputEnum<SamplerAddressMode>("Address Mode U", &sampler.AddressModeU);
		change |= EditorUI::InputEnum<SamplerAddressMode>("Address Mode V", &sampler.AddressModeV);
		change |= EditorUI::InputEnum<SamplerAddressMode>("Address Mode W", &sampler.AddressModeW);

		if (change)
		{
			Game::GetGame().GetEngine()->GetGraphicsModule()->GetResourceManager()->SetDirty(pTextureData->GetGPUUUID());
			Tumbnail::SetDirty(pTextureData->GetUUID());
		}
		return change;
	}
}
