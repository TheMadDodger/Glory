#pragma once
#include "FontData.h"

#include <Reflection.h>
#include <AssetReference.h>

namespace Glory
{
	struct TextComponent
	{
		TextComponent() : m_Font(0), m_Size(48), m_Text("Hello World"), m_Dirty(true) {}

		REFLECTABLE(TextComponent,
			(AssetReference<FontData>)(m_Font),
			(uint32_t)(m_Size),
			(std::string)(m_Text)
		);

		bool m_Dirty;
	};
}
