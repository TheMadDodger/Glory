#include "StringsOverrideTableEditor.h"

#include <AssetPicker.h>
#include <Undo.h>

namespace Glory::Editor
{
	StringsOverrideTableEditor::StringsOverrideTableEditor()
	{
	}

	StringsOverrideTableEditor::~StringsOverrideTableEditor()
	{
	}

	void StringsOverrideTableEditor::Initialize()
	{
	}

	bool StringsOverrideTableEditor::OnGUI()
	{
		YAMLResource<StringsOverrideTable>* pTextureData = (YAMLResource<StringsOverrideTable>*)m_pTarget;
		Utils::YAMLFileRef& file = **pTextureData;
		auto baseTable = file["BaseTable"];
		const UUID baseTableID = baseTable.As<uint64_t>(0);
		UUID newBaseTableID = baseTableID;
		bool change = false;
		const uint32_t stringTableHash = ResourceTypes::GetHash<StringTable>();
		if (AssetPicker::ResourceDropdown("Base Table", stringTableHash, &newBaseTableID, false))
		{
			Undo::StartRecord("Base Table Change", pTextureData->GetUUID());
			Undo::ApplyYAMLEdit(file, baseTable.Path(), uint64_t(baseTableID), uint64_t(newBaseTableID));
			Undo::StopRecord();
			change = true;
		}
		auto language = file["Language"];
		/** @todo */
		return change;
	}

	std::string StringsOverrideTableEditor::Name()
	{
		return "Strings Override Table";
	}
}
