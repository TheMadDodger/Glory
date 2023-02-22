#include "TumbnailGenerator.h"

namespace Glory::Editor
{
	BaseTumbnailGenerator::BaseTumbnailGenerator()
	{
	}

	BaseTumbnailGenerator::~BaseTumbnailGenerator()
	{
	}

	void BaseTumbnailGenerator::OnFileDoubleClick(UUID uuid) {}

	const std::string& BaseTumbnailGenerator::GetExtension()
	{
		return "";
	}
}