#pragma once
#include <ImporterTemplate.h>
#include <ModelData.h>

#include <EntityID.h>

namespace Glory
{
    class TextureData;
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
        enum AxisConversion
        {
            X,
            Y,
            Z
        };

        struct Context
        {
            PrefabData* Prefab;
            std::vector<TextureData*> Textures;
            std::vector<MaterialData*> Materials;
            AxisConversion UpAxis{ AxisConversion::Y };
            int UpAxisSign{ 1 };
            AxisConversion FrontAxis{ AxisConversion::Z };
            int FrontAxisSign{ 1 };
            float UnitScaleFactor{ 1.0f };
        };

        bool SupportsExtension(const std::filesystem::path& extension) const override;
        ImportedResource LoadResource(const std::filesystem::path& path, void*) const override;

        void ProcessNode(Context& context, Utils::ECS::EntityID parent, aiNode* node, const aiScene* scene, ImportedResource& resource) const;
        MeshData* ProcessMesh(Context& context, aiMesh* mesh) const;
    };
}
