#include "UIDocumentTumbnailGenerator.h"
#include "UIEditor.h"

#include <Importer.h>
#include <EditorTextureData.h>

namespace Glory::Editor
{
    UIDocumentTumbnailGenerator::UIDocumentTumbnailGenerator(): m_pUIDocTumbnail(nullptr)
    {
    }

    UIDocumentTumbnailGenerator::~UIDocumentTumbnailGenerator()
    {
        if (!m_pUIDocTumbnail) return;
        delete m_pUIDocTumbnail;
        m_pUIDocTumbnail = nullptr;
    }

    TextureData* UIDocumentTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
    {
        if (!m_pUIDocTumbnail)
        {
            ImportedResource resource = Importer::Import("./EditorAssets/Mono/file.png");
            ImageData* pImageData = (ImageData*)*resource;
            resource.Cleanup();
            m_pUIDocTumbnail = new EditorTextureData(pImageData);
        }

        return m_pUIDocTumbnail;
    }

    void UIDocumentTumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        UIEditor* pEditor = EditorWindow::GetWindow<UIEditor>();
        pEditor->SetDocument(uuid);
    }
}
