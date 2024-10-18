#pragma once
#include <ImporterTemplate.h>
#include <ModelData.h>

#include <EntityID.h>

namespace Glory
{
    class MaterialData;
    class MeshData;
    class PrefabData;
}

struct aiScene;
struct aiNode;
struct aiMesh;

namespace Glory::Editor
{
    class ASSIMPImporter : public ImporterTemplate<ModelData>
    {
    public:
        ASSIMPImporter();
        virtual ~ASSIMPImporter();

    private:
        virtual std::string_view Name() const override;
        void Initialize() override;
        void Cleanup() override;

    private:
        struct Context
        {
            std::vector<MaterialData*> Materials;
        };

        bool SupportsExtension(const std::filesystem::path& extension) const override;
        ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;

        void ProcessNode(Context& context, aiNode* node, const aiScene* scene, ImportedResource& resource) const;
        MeshData* ProcessMesh(aiMesh* mesh) const;
    };
}
