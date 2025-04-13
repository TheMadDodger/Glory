#include "TextDatabaseTumbnailGenerator.h"
#include "TextDatabaseEditor.h"

#include <EditorTextureData.h>
#include <Importer.h>
#include <MainEditor.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
    TextDatabaseTumbnailGenerator::TextDatabaseTumbnailGenerator(): m_pTextDBTumbnail(nullptr)
    {
    }

    TextDatabaseTumbnailGenerator::~TextDatabaseTumbnailGenerator()
    {
        if (!m_pTextDBTumbnail) return;
        delete m_pTextDBTumbnail;
        m_pTextDBTumbnail = nullptr;
    }

    TextureData* TextDatabaseTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
    {
        if (!m_pTextDBTumbnail)
        {
            ImportedResource resource = Importer::Import("./EditorAssets/Mono/file.png");
            ImageData* pImageData = (ImageData*)*resource;
            resource.Cleanup();
            m_pTextDBTumbnail = new EditorTextureData(pImageData);
        }

        return m_pTextDBTumbnail;
    }

    void TextDatabaseTumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        editor.GetWindow<TextDatabaseEditor>();
    }
}
