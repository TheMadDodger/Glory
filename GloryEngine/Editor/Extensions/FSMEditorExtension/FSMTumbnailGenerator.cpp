#include "FSMTumbnailGenerator.h"
#include "FSMEditor.h"
#include "FSMPropertiesWindow.h"
#include "FSMNodeEditor.h"

#include <EditorTextureData.h>
#include <Importer.h>
#include <MainEditor.h>
#include <EditorApplication.h>

namespace Glory::Editor
{
    FSMTumbnailGenerator::FSMTumbnailGenerator(): m_pFSMTumbnail(nullptr)
    {
    }

    FSMTumbnailGenerator::~FSMTumbnailGenerator()
    {
        if (!m_pFSMTumbnail) return;
        delete m_pFSMTumbnail;
        m_pFSMTumbnail = nullptr;
    }

    TextureData* FSMTumbnailGenerator::GetTumbnail(const ResourceMeta* pResourceType)
    {
        if (!m_pFSMTumbnail)
        {
            ImportedResource resource = Importer::Import("./EditorAssets/Mono/file.png");
            ImageData* pImageData = (ImageData*)*resource;
            resource.Cleanup();
            m_pFSMTumbnail = new EditorTextureData(pImageData);
        }

        return m_pFSMTumbnail;
    }

    void FSMTumbnailGenerator::OnFileDoubleClick(UUID uuid)
    {
        MainEditor& editor = EditorApplication::GetInstance()->GetMainEditor();
        FSMEditor* pMainWindow = editor.GetMainWindow<FSMEditor>();
        pMainWindow->SetFSM(uuid);
        editor.GetWindow<FSMEditor, FSMPropertiesWindow>();
        editor.GetWindow<FSMEditor, FSMNodeEditor>();
    }
}
