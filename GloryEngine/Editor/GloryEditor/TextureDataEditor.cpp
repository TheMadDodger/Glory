#include "TextureDataEditor.h"
#include "AssetPicker.h"
#include "EditorAssetDatabase.h"
#include "PropertyDrawer.h"
#include "EditorUI.h"
#include "EditorApplication.h"
#include "TextureImporter.h"

#include <IEngine.h>
#include <imgui.h>
#include <ResourceType.h>
#include <ImageData.h>
#include <TextureData.h>
#include <Resources.h>

namespace Glory::Editor
{
	TextureDataEditor::TextureDataEditor() {}

	TextureDataEditor::~TextureDataEditor() {}

	bool TextureDataEditor::OnGUI()
	{
		YAMLResource<TextureData>* pTextureData = (YAMLResource<TextureData>*)m_pTarget;
		Utils::YAMLFileRef& file = pTextureData->File();
		Utils::NodeValueRef node = **pTextureData;

		bool change = false;

		auto image = node["Image"];
		auto sampler = node["Sampler"];

		const uint64_t oldValue = image.As<uint64_t>();

		UUID assetID = oldValue;
		if(AssetPicker::ResourceDropdown("Image", ResourceTypes::GetHash<ImageData>(), &assetID))
		{
			Undo::ApplyYAMLEdit(file, image.Path(), oldValue, uint64_t(assetID));
			change = true;
		}

		change |= EditorUI::InputEnum<Filter>(file, sampler["MinFilter"].Path());
		change |= EditorUI::InputEnum<Filter>(file, sampler["MagFilter"].Path());
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, sampler["AddressModeU"].Path());
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, sampler["AddressModeV"].Path());
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, sampler["AddressModeW"].Path());
		change |= EditorUI::InputEnum<Filter>(file, sampler["MipmapMode"].Path());

		if (change)
		{
			pTextureData->SetDirty(true);
			EditorAssetDatabase::SetAssetDirty(pTextureData);
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
		auto& imageRef = pTextureData->Image();
		UUID assetID;
		if (AssetPicker::ResourceDropdown("Image", ResourceTypes::GetHash<ImageData>(), &assetID))
			imageRef.SetUUID(assetID);

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
