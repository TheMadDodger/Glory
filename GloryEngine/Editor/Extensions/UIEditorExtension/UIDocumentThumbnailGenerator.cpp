#include "UIDocumentThumbnailGenerator.h"
#include "UIMainWindow.h"
#include "UIEditor.h"
#include "UIElementsGraphWindow.h"
#include "UIElementInspector.h"
#include "AddUIElementWindow.h"

#include <Importer.h>
#include <EditorTextureData.h>

namespace Glory::Editor
{
    UIDocumentThumbnailGenerator::UIDocumentThumbnailGenerator(): m_pUIDocThumbnail(nullptr)
    {
    }

    UIDocumentThumbnailGenerator::~UIDocumentThumbnailGenerator()
    {
        if (!m_pUIDocThumbnail) return;
        delete m_pUIDocThumbnail;
        m_pUIDocThumbnail = nullptr;
    }

    TextureData* UIDocumentThumbnailGenerator::GetThumbnail(const ResourceMeta* pResourceType)
    {
        if (!m_pUIDocThumbnail)
        {
            ImportedResource resource = Importer::Import("./EditorAssets/Mono/file.png");
            ImageData* pImageData = (ImageData*)*resource;
            resource.Cleanup();
            m_pUIDocThumbnail = new EditorTextureData(pImageData);
        }

        return m_pUIDocThumbnail;
    }

    void UIDocumentThumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        UIMainWindow* pMainWindow = editor.GetMainWindow<UIMainWindow>();
        pMainWindow->SetDocument(uuid);
        editor.GetWindow<UIMainWindow, UIEditor>();
        editor.GetWindow<UIMainWindow, UIElementsGraphWindow>();
        editor.GetWindow<UIMainWindow, UIElementInspector>();
        editor.GetWindow<UIMainWindow, AddUIElementWindow>();
    }
}
