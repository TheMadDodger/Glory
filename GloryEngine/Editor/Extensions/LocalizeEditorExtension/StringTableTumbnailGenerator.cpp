#include "StringTableTumbnailGenerator.h"
#include "StringTableEditor.h"

#include <EditorTextureData.h>
#include <Importer.h>
#include <MainEditor.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
    StringTableTumbnailGenerator::StringTableTumbnailGenerator(): m_pTextDBTumbnail(nullptr)
    {
    }

    StringTableTumbnailGenerator::~StringTableTumbnailGenerator()
    {
        if (!m_pTextDBTumbnail) return;
        delete m_pTextDBTumbnail;
        m_pTextDBTumbnail = nullptr;
    }

    TextureData* StringTableTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
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

    void StringTableTumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        StringTableEditor* pEditor = editor.GetWindow<StringTableEditor>();
        pEditor->SetTable(uuid);
    }

    StringsOverrideTableTumbnailGenerator::StringsOverrideTableTumbnailGenerator(): m_pTextDBTumbnail(nullptr)
    {
    }

    StringsOverrideTableTumbnailGenerator::~StringsOverrideTableTumbnailGenerator()
    {
    }

    TextureData* StringsOverrideTableTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
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

    void StringsOverrideTableTumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        StringTableEditor* pEditor = editor.GetWindow<StringTableEditor>();
        pEditor->SetTable(uuid);
    }
}
