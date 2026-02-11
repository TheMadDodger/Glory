#include "ThumbnailGenerator.h"

namespace Glory::Editor
{
	const std::string BaseThumbnailGenerator::m_DefaultExtension;

	BaseThumbnailGenerator::BaseThumbnailGenerator()
	{
	}

	BaseThumbnailGenerator::~BaseThumbnailGenerator()
	{
	}

	void BaseThumbnailGenerator::OnFileDoubleClick(UUID uuid) {}

	const std::string& BaseThumbnailGenerator::GetExtension()
	{
		return m_DefaultExtension;
	}
}