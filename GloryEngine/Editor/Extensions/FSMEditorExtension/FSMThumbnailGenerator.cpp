#include "FSMThumbnailGenerator.h"
#include "FSMEditor.h"
#include "FSMPropertiesWindow.h"
#include "FSMNodeEditor.h"
#include "FSMNodeInspector.h"
#include "FSMDebugger.h"

#include <EditorTextureData.h>
#include <Importer.h>
#include <MainEditor.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
    FSMThumbnailGenerator::FSMThumbnailGenerator(): m_pFSMThumbnail(nullptr)
    {
    }

    FSMThumbnailGenerator::~FSMThumbnailGenerator()
    {
        if (!m_pFSMThumbnail) return;
        delete m_pFSMThumbnail;
        m_pFSMThumbnail = nullptr;
    }

    TextureData* FSMThumbnailGenerator::GetThumbnail(const ResourceMeta* pResourceType)
    {
        if (!m_pFSMThumbnail)
        {
            ImportedResource resource = Importer::Import("./EditorAssets/Mono/file.png");
            ImageData* pImageData = (ImageData*)*resource;
            resource.Cleanup();
            m_pFSMThumbnail = new EditorTextureData(pImageData);
        }

        return m_pFSMThumbnail;
    }

    void FSMThumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        FSMEditor* pMainWindow = editor.GetMainWindow<FSMEditor>();
        pMainWindow->SetFSM(uuid);
        editor.GetWindow<FSMEditor, FSMPropertiesWindow>();
        editor.GetWindow<FSMEditor, FSMNodeEditor>();
        editor.GetWindow<FSMEditor, FSMNodeInspector>();
        editor.GetWindow<FSMEditor, FSMDebugger>();
    }
}
