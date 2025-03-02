#include "UIDocumentTumbnailGenerator.h"
#include "UIMainWindow.h"
#include "UIEditor.h"
#include "UIElementsGraphWindow.h"
#include "UIElementInspector.h"

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
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        UIMainWindow* pMainWindow = editor.GetMainWindow<UIMainWindow>();
        pMainWindow->SetDocument(uuid);
        editor.GetWindow<UIMainWindow, UIEditor>();
        editor.GetWindow<UIMainWindow, UIElementsGraphWindow>();
        editor.GetWindow<UIMainWindow, UIElementInspector>();
    }
}
