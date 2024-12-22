#include "TextComponentEditor.h"

namespace Glory::Editor
{
	TextComponentEditor::TextComponentEditor()
	{
	}

	TextComponentEditor::~TextComponentEditor()
	{
	}

	void TextComponentEditor::Initialize()
	{
		EntityComponentEditor::Initialize();
	}

	bool TextComponentEditor::OnGUI()
	{
		const bool change = EntityComponentEditor::OnGUI();
		if (change)
			GetTargetComponent().m_Dirty = true;
		return change;
	}

	std::string TextComponentEditor::Name()
	{
		return "Text";
	}
}
