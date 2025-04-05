#include "FSMTumbnailGenerator.h"

#include <EditorTextureData.h>
#include <Importer.h>

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
        
    }
}
