#pragma once

namespace Glory
{
	struct Character;
	class FontData;

	class IFontImageGenerator
	{
	public:
		virtual Character* GetCharacterMap(FontData* pFont, uint32_t size) = 0;
	};
}
