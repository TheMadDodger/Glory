#pragma once
#include <ImporterTemplate.h>
#include <AudioData.h>

namespace Glory::Editor
{
	class GenericAudioImporter : public ImporterTemplate<AudioData>
	{
	public:
		GenericAudioImporter();
		virtual ~GenericAudioImporter();

		std::string_view Name() const override;

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void* userData) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override;

		int m_InitializedFlags;
	};
}
