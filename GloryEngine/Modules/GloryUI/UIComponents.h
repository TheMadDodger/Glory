#pragma once
#include "FontData.h"

#include <Reflection.h>
#include <AssetReference.h>

namespace Glory
{
	struct TextComponent
	{
		TextComponent(): m_Font(0), m_Text("Hello World") {}

		REFLECTABLE(TextComponent,
			(AssetReference<FontData>)(m_Font),
			(uint32_t)(m_Size),
			(std::string)(m_Text)
		);
	};
}
