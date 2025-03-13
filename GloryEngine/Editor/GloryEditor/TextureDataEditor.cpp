#include "TextureDataEditor.h"
#include "AssetPicker.h"
#include "EditorAssetDatabase.h"
#include "PropertyDrawer.h"
#include "EditorUI.h"
#include "Tumbnail.h"
#include "EditorApplication.h"

#include <Engine.h>
#include <GraphicsModule.h>
#include <imgui.h>
#include <ResourceType.h>

namespace Glory::Editor
{
	TextureDataEditor::TextureDataEditor() {}

	TextureDataEditor::~TextureDataEditor() {}

	bool TextureDataEditor::OnGUI()
	{
		YAMLResource<TextureData>* pTextureData = (YAMLResource<TextureData>*)m_pTarget;
		Utils::YAMLFileRef& file = **pTextureData;

		bool change = false;

		auto image = file["Image"];

		const uint64_t oldValue = image.As<uint64_t>();
		AssetReference<ImageData> imageRef = oldValue;
		if(AssetPicker::ResourceDropdown("Image", ResourceTypes::GetHash<ImageData>(), imageRef.AssetUUIDMember()))
		{
			Undo::ApplyYAMLEdit(file, image.Path(), oldValue, uint64_t(imageRef.AssetUUID()));
			change = true;
		}

		change |= EditorUI::InputEnum<Filter>(file, "Sampler/MinFilter");
		change |= EditorUI::InputEnum<Filter>(file, "Sampler/MagFilter");
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, "Sampler/AddressModeU");
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, "Sampler/AddressModeV");
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, "Sampler/AddressModeW");
		change |= EditorUI::InputEnum<Filter>(file, "Sampler/MipmapMode");

		if (change)
		{
			EditorApplication* pApplication = EditorApplication::GetInstance();
			pApplication->GetEngine()->GetMainModule<GraphicsModule>()->GetResourceManager()->SetDirty(pTextureData->GetGPUUUID());
			EditorAssetDatabase::SetAssetDirty(pTextureData);
			Tumbnail::SetDirty(pTextureData->GetUUID());
			pTextureData->SetDirty(true);
			
		}
		return change;
	}

	StaticTextureDataEditor::StaticTextureDataEditor()
	{
	}

	StaticTextureDataEditor::~StaticTextureDataEditor()
	{
	}

	bool StaticTextureDataEditor::OnGUI()
	{
		TextureData* pTextureData = (TextureData*)m_pTarget;

		ImGui::BeginDisabled(true);
		AssetReference<ImageData>& imageRef = pTextureData->Image();
		AssetPicker::ResourceDropdown("Image", ResourceTypes::GetHash<ImageData>(), imageRef.AssetUUIDMember());

		SamplerSettings& sampler = pTextureData->GetSamplerSettings();
		EditorUI::InputEnum<Filter>("MinFilter", &sampler.MinFilter);
		EditorUI::InputEnum<Filter>("MagFilter", &sampler.MagFilter);
		EditorUI::InputEnum<SamplerAddressMode>("AddressModeU", &sampler.AddressModeU);
		EditorUI::InputEnum<SamplerAddressMode>("AddressModeV", &sampler.AddressModeV);
		EditorUI::InputEnum<SamplerAddressMode>("AddressModeW", &sampler.AddressModeW);
		EditorUI::InputEnum<Filter>("MipmapMode", &sampler.MipmapMode);
		ImGui::EndDisabled();

		return false;
	}
}
