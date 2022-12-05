#include "TumbnailGenerator.h"

namespace Glory::Editor
{
	GLORY_EDITOR_API BaseTumbnailGenerator::BaseTumbnailGenerator()
	{
	}

	GLORY_EDITOR_API BaseTumbnailGenerator::~BaseTumbnailGenerator()
	{
	}

	GLORY_EDITOR_API void BaseTumbnailGenerator::OnFileDoubleClick(UUID uuid) {}

	GLORY_EDITOR_API const std::string& BaseTumbnailGenerator::GetExtension()
	{
		return "";
	}
}