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
		AssetReference<ImageData> imageRef = image.As<uint64_t>();
		if(AssetPicker::ResourceDropdown("Image", ResourceTypes::GetHash<ImageData>(), imageRef.AssetUUIDMember()))
		{
			image.Set(imageRef.AssetUUID());
		}

		change |= EditorUI::InputEnum<Filter>(file, "Sampler/MinFilter");
		change |= EditorUI::InputEnum<Filter>(file, "Sampler/MagFilter");
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, "Sampler/AddressModeU");
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, "Sampler/AddressModeV");
		change |= EditorUI::InputEnum<SamplerAddressMode>(file, "Sampler/AddressModeW");

		if (change)
		{
			EditorApplication::GetInstance()->GetEngine()->GetMainModule<GraphicsModule>()->GetResourceManager()->SetDirty(pTextureData->GetGPUUUID());
			Tumbnail::SetDirty(pTextureData->GetUUID());
			pTextureData->SetDirty(true);
		}
		return change;
	}
}
