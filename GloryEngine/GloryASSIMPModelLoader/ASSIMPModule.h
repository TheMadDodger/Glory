#pragma once
#include <ModelLoaderModule.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "MeshData.h"

namespace Glory
{
    class ASSIMPModule : public ModelLoaderModule
    {
    public:
        ASSIMPModule();
        virtual ~ASSIMPModule();

    private:
        virtual void Initialize() override;
        virtual void Cleanup() override;

    private:
        virtual ModelData* LoadModel(const std::string& path, const ModelImportSettings& importSettings) override;
        virtual ModelData* LoadModel(const void* buffer, size_t length, const ModelImportSettings& importSettings) override;
        void ProcessNode(aiNode* node, const aiScene* scene, ModelData* pModel);
        MeshData* ProcessMesh(aiMesh* mesh);
    };
}
