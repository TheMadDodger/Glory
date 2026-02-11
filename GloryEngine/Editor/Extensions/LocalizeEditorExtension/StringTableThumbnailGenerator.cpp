#include "StringTableThumbnailGenerator.h"
#include "StringTableEditor.h"

#include <EditorTextureData.h>
#include <Importer.h>
#include <MainEditor.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
    StringTableThumbnailGenerator::StringTableThumbnailGenerator(): m_pTextDBThumbnail(nullptr)
    {
    }

    StringTableThumbnailGenerator::~StringTableThumbnailGenerator()
    {
        if (!m_pTextDBThumbnail) return;
        delete m_pTextDBThumbnail;
        m_pTextDBThumbnail = nullptr;
    }

    TextureData* StringTableThumbnailGenerator::GetThumbnail(const ResourceMeta* pResourceType)
    {
        if (!m_pTextDBThumbnail)
        {
            ImportedResource resource = Importer::Import("./EditorAssets/Mono/file.png");
            ImageData* pImageData = (ImageData*)*resource;
            resource.Cleanup();
            m_pTextDBThumbnail = new EditorTextureData(pImageData);
        }

        return m_pTextDBThumbnail;
    }

    void StringTableThumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        StringTableEditor* pEditor = editor.GetWindow<StringTableEditor>();
        pEditor->SetTable(uuid);
    }

    StringsOverrideTableThumbnailGenerator::StringsOverrideTableThumbnailGenerator(): m_pTextDBThumbnail(nullptr)
    {
    }

    StringsOverrideTableThumbnailGenerator::~StringsOverrideTableThumbnailGenerator()
    {
    }

    TextureData* StringsOverrideTableThumbnailGenerator::GetThumbnail(const ResourceMeta* pResourceType)
    {
        if (!m_pTextDBThumbnail)
        {
            ImportedResource resource = Importer::Import("./EditorAssets/Mono/file.png");
            ImageData* pImageData = (ImageData*)*resource;
            resource.Cleanup();
            m_pTextDBThumbnail = new EditorTextureData(pImageData);
        }

        return m_pTextDBThumbnail;
    }

    void StringsOverrideTableThumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        StringTableEditor* pEditor = editor.GetWindow<StringTableEditor>();
        pEditor->SetTable(uuid);
    }
}
