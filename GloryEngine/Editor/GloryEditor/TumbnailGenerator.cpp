#include "TumbnailGenerator.h"

namespace Glory::Editor
{
	const std::string BaseTumbnailGenerator::m_DefaultExtension;

	BaseTumbnailGenerator::BaseTumbnailGenerator()
	{
	}

	BaseTumbnailGenerator::~BaseTumbnailGenerator()
	{
	}

	void BaseTumbnailGenerator::OnFileDoubleClick(UUID uuid) {}

	const std::string& BaseTumbnailGenerator::GetExtension()
	{
		return m_DefaultExtension;
	}
}