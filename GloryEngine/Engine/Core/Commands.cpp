#include "Commands.h"

namespace Glory
{
	std::vector<BaseParser*> Parser::m_Parsers = {
		new IntParser(),
		new FloatParser(),
		new StringParser(),
		new SizeTParser(),
	};
}