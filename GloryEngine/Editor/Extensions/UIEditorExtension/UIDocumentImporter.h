#pragma once
#include <ImporterTemplate.h>
#include <UIDocumentData.h>
#include <UIDocument.h>
#include <Glory.h>

namespace Glory::Editor
{
    class UIDocumentImporter : public YAMLImporterTemplate<UIDocumentData>
    {
	public:
		UIDocumentImporter() {};
		virtual ~UIDocumentImporter() {};

		std::string_view Name() const override;

		GLORY_API static void DeserializeComponent(Engine* pEngine, UIDocumentData* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef component);
		GLORY_API static void DeserializeEntity(Engine* pEngine, UIDocumentData* pDocument, Utils::NodeValueRef node);
		GLORY_API static void SerializeEntityRecursive(Engine* pEngine, UIDocumentData* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef entities);
		GLORY_API static void SerializeEntityRecursive(Engine* pEngine, UIDocument* pDocument, Utils::ECS::EntityID entity, Utils::NodeValueRef entities);

	private:
		virtual bool SupportsExtension(const std::filesystem::path& extension) const override;
		virtual ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;
		virtual bool SaveResource(const std::filesystem::path& path, UIDocumentData* pDocument) const override;

	private:
		virtual void Initialize() override;
		virtual void Cleanup() override {};
    };
}
