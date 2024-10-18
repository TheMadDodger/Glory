#include "ASSIMPImporter.h"
#include "VertexDefinitions.h"

#include <EditorApplication.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Components.h>
#include <MeshData.h>
#include <PrefabData.h>
#include <Debug.h>
#include <sstream>
#include <SceneManager.h>
#include <PipelineData.h>

#include <EntityRegistry.h>

#include <EditorPipelineManager.h>
#include <EditorMaterialManager.h>
#include <EditorAssetDatabase.h>

#define _FIRST(x, y, z) x
#define UNPACK(x) x
#define FIRST(x) _FIRST(UNPACK(x))

#define ReadMetaData(name, out) if (pScene->mMetaData->HasKey(name)) \
{ \
    pScene->mMetaData->Get(name, out); \
}

#define SWAP(value, a, b) temp = value.a;\
value.a = value.b;\
value.b = temp;

namespace Glory::Editor
{
    constexpr size_t NumSupportedExtensions = 4;
    constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
        ".obj",
        ".fbx",
        ".gltf",
        ".glb",
    };

#pragma warning(push)
#pragma warning(disable : 4002)
#pragma warning(disable : 4003)
    constexpr size_t MaterialPropertyKeysCount = 7;
    constexpr std::string_view MaterialPropertyKeys[MaterialPropertyKeysCount] = {
        FIRST(AI_MATKEY_OPACITY),
        FIRST(AI_MATKEY_TRANSPARENCYFACTOR),
        FIRST(AI_MATKEY_BUMPSCALING),
        FIRST(AI_MATKEY_SHININESS),
        FIRST(AI_MATKEY_REFLECTIVITY),
        FIRST(AI_MATKEY_SHININESS_STRENGTH),
        FIRST(AI_MATKEY_REFRACTI),
    };

    constexpr size_t MaterialColorPropertyKeysCount = 6;
    constexpr std::string_view MaterialColorPropertyKeys[MaterialColorPropertyKeysCount] = {
        FIRST(AI_MATKEY_COLOR_DIFFUSE),
        FIRST(AI_MATKEY_COLOR_AMBIENT),
        FIRST(AI_MATKEY_COLOR_SPECULAR),
        FIRST(AI_MATKEY_COLOR_EMISSIVE),
        FIRST(AI_MATKEY_COLOR_TRANSPARENT),
        FIRST(AI_MATKEY_COLOR_REFLECTIVE),
    };
#pragma warning(pop)

    ASSIMPImporter::ASSIMPImporter()
	{
	}

    ASSIMPImporter::~ASSIMPImporter()
	{
	}

    std::string_view ASSIMPImporter::Name() const
    {
        return "ASSIMP Importer";
    }

    void ASSIMPImporter::Initialize()
	{
        /* We need to manually set the component types instance */
        EditorApplication::GetInstance()->GetEngine()->GetSceneManager()->ComponentTypesInstance();
	}

	void ASSIMPImporter::Cleanup()
	{
	}

    bool ASSIMPImporter::SupportsExtension(const std::filesystem::path& extension) const
    {
        for (size_t i = 0; i < NumSupportedExtensions; ++i)
        {
            if (extension.compare(SupportedExtensions[i]) != 0) continue;
            return true;
        }
        return false;
    }

    ImportedResource ASSIMPImporter::LoadResource(const std::filesystem::path& path, void*) const
    {
        Assimp::Importer importer;

        const aiScene* pScene = importer.ReadFile(path.string(), aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
        {
            std::stringstream str;
            str << "ASSIMP: Could not import file: " << path << " Error: " << importer.GetErrorString();
            EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError(str.str());
            return nullptr;
        }

        ModelData* pModel = new ModelData();
        ImportedResource resource{ path, pModel };

        Context context;
        ReadMetaData("UpAxis", context.UpAxis);
        ReadMetaData("UpAxisSign", context.UpAxisSign);
        ReadMetaData("FrontAxis", context.FrontAxis);
        ReadMetaData("FrontAxisSign", context.FrontAxisSign);
        ReadMetaData("UnitScaleFactor", context.UnitScaleFactor);

        for (size_t i = 0; i < pScene->mMetaData->mNumProperties; ++i)
        {
            EditorApplication::GetInstance()->GetEngine()->GetDebug().LogInfo(pScene->mMetaData->mKeys[i].C_Str());
        }

        EditorPipelineManager& pipelines = EditorApplication::GetInstance()->GetPipelineManager();
        MaterialManager& materials = EditorApplication::GetInstance()->GetMaterialManager();
        const std::filesystem::path assetsPath = ProjectSpace::GetOpenProject()->RootPath();

        if (pScene->HasMaterials())
        {
            for (size_t i = 0; i < pScene->mNumMaterials; ++i)
            {
                const aiMaterial* material = pScene->mMaterials[i];
                MaterialData* pMaterial = new MaterialData();
                pMaterial->SetName(std::string_view{ material->GetName().C_Str(), material->GetName().length });

                aiShadingMode shadingMode;
                material->Get(AI_MATKEY_SHADING_MODEL, shadingMode);
                const bool usesTextures = material->GetTextureCount(aiTextureType_BASE_COLOR) ||
                    material->GetTextureCount(aiTextureType_NORMALS);

                MaterialManager& materials = EditorApplication::GetInstance()->GetMaterialManager();
                UUID pipelineID = pipelines.FindPipeline(PipelineType(shadingMode), usesTextures);
                if (!pipelineID)
                    pipelineID = pipelines.FindPipeline(PT_Phong, usesTextures);
                if (pipelineID)
                {
                    pMaterial->SetPipeline(pipelineID);
                    PipelineData* pPipeline = pipelines.GetPipelineData(pipelineID);
                    pPipeline->LoadIntoMaterial(pMaterial);

                    for (size_t i = 0; i < pMaterial->PropertyInfoCount(materials); ++i)
                    {
                        MaterialPropertyInfo* info = pMaterial->GetPropertyInfoAt(materials, i);
                        /* Match names */
                        if (info->IsResource()) continue;
                        const size_t components = info->Size()/4;
                        const std::string_view name = std::string_view{ info->ShaderName() }.substr(1);
                        switch (components)
                        {
                        case 1:
                        {
                            for (size_t i = 0; i < MaterialPropertyKeysCount; ++i)
                            {
                                const std::string_view key = MaterialPropertyKeys[i];
                                const std::string_view comparator = key.substr(5);
                                if (name != comparator) continue;
                                float value;
                                if (material->Get(key.data(), 0, 0, value) != aiReturn_SUCCESS) continue;
                                pMaterial->Set(materials, info->ShaderName(), value);
                            }
                            break;
                        }
                        case 4:
                        {
                            aiColor3D color;
                            for (size_t i = 0; i < MaterialColorPropertyKeysCount; ++i)
                            {
                                const std::string_view key = MaterialColorPropertyKeys[i];
                                const std::string_view comparator = key.substr(5);
                                if (name != comparator && name != "olor") continue;
                                if (name == "olor" && key != "$clr.diffuse" && key != "$clr.base") continue;
                                if (material->Get(key.data(), 0, 0, color) != aiReturn_SUCCESS) continue;
                                pMaterial->Set(materials, info->ShaderName(), glm::vec4(color.r, color.g, color.b, 1.0f));
                            }
                            break;
                        }
                        }
                    }
                }

                for (size_t textureTypeI = 0; textureTypeI < TT_Count; ++textureTypeI)
                {
                    const TextureType textureType = TextureType(textureTypeI);
                    const aiTextureType aiTexType = aiTextureType(textureType);
                    const size_t texCount = std::min<size_t>(material->GetTextureCount(aiTexType),
                        pMaterial->TextureCount(materials, textureType));
                    for (size_t j = 0; j < texCount; ++j)
                    {
                        aiString texPath;
                        if (material->GetTexture(aiTexType, j, &texPath) != aiReturn_SUCCESS)
                            continue;

                        std::filesystem::path texturePath = path.parent_path();
                        texturePath.append(texPath.C_Str());
                        const UUID texID = EditorAssetDatabase::ReserveAssetUUID(texturePath.string(), "Default").first;
                        pMaterial->SetTexture(materials, textureType, j, texID);
                    }
                }

                resource.AddChild(pMaterial, pMaterial->Name());
                context.Materials.push_back(pMaterial);
            }
        }

        context.Prefab = new PrefabData();
        ProcessNode(context, 0, pScene->mRootNode, pScene, resource);

        resource.AddChild(context.Prefab, path.filename().replace_extension().string() + "_Prefab");
        importer.FreeScene();
        return resource;
    }

    glm::vec4 toColorAndIntensity(const aiColor3D& color)
    {
        /* Doesn't recover exact values but it should be a match visually */
        const float total = color.r + color.b + color.g;
        return glm::vec4{ color.r/total, color.g/total, color.b/total, total };
    }

    glm::vec4 toVec4(const aiColor3D& color)
    {
        return glm::vec4(color.r, color.g, color.b, 1.0f);
    }

    void ASSIMPImporter::ProcessNode(Context& context, Utils::ECS::EntityID parent, aiNode* node, const aiScene* scene, ImportedResource& resource) const
    {
        // process all the node's meshes (if any)
        aiVector3D scale, position;
        aiQuaternion rotation;
        node->mTransformation.Decompose(scale, rotation, position);

        glm::vec3 convertedScale{ scale.x, scale.y, scale.z };
        const glm::quat convertedQuat{ rotation.w, rotation.x, rotation.y, rotation.z };
        glm::vec3 convertedPos{ position.x, position.y, position.z };

        /* Axes conversions */
        float temp;
        switch (context.UpAxis)
        {
        case AxisConversion::X:
            SWAP(convertedPos, x, y);
            SWAP(convertedScale, x, y);
            break;
        case AxisConversion::Z:
            SWAP(convertedScale, z, y);
            SWAP(convertedScale, z, y);
            break;
        default:
            break;
        }
        switch (context.FrontAxis)
        {
        case AxisConversion::X: {
            switch (context.UpAxis)
            {
            case AxisConversion::Z:
                /* Z was already swapped with Y */
                SWAP(convertedScale, x, y);
                SWAP(convertedScale, x, y);
                break;
            default:
                SWAP(convertedScale, x, z);
                SWAP(convertedScale, x, z);
                break;
            }
            break;
        }
        case AxisConversion::Y:
            SWAP(convertedScale, y, z);
            SWAP(convertedScale, y, z);
            break;
        default:
            break;
        }

        Entity entity = context.Prefab->CreateEmptyObject(node->mName.C_Str());
        Transform& transform = entity.GetComponent<Transform>();
        transform.Position = convertedPos;
        transform.Rotation = convertedQuat;
        transform.Scale = convertedScale;

        if (parent)
            entity.SetParent(parent);

        if (scene->HasLights())
        {
            for (size_t i = 0; i < scene->mNumLights; ++i)
            {
                const aiLight* light = scene->mLights[i];
                if (node->mName == light->mName)
                {
                    LightComponent& lightComp = entity.AddComponent<LightComponent>();
                    const glm::vec4 color = toColorAndIntensity(light->mColorDiffuse);
                    lightComp.m_Intensity = color.a;
                    lightComp.m_Color = color;
                    lightComp.m_Color.a = 1.0f;
                    lightComp.m_Range = 100.0f;
                    break;
                }
            }
        }

        if (scene->HasCameras())
        {
            for (size_t i = 0; i < scene->mNumCameras; ++i)
            {
                const aiCamera* cam = scene->mCameras[i];
                if (node->mName == cam->mName)
                {
                    CameraComponent& cameraComp = entity.AddComponent<CameraComponent>();
                    cameraComp.m_Far = cam->mClipPlaneFar;
                    cameraComp.m_Near = cam->mClipPlaneNear;
                    cameraComp.m_HalfFOV = glm::degrees(cam->mHorizontalFOV);
                    break;
                }
            }
        }

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            Entity meshChild = context.Prefab->CreateEmptyObject(node->mName.C_Str());
            meshChild.SetParent(entity.GetEntityID());
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshData* pMeshData = ProcessMesh(context, mesh);
            if (!pMeshData) continue;
            MaterialData* pMaterial = mesh->mMaterialIndex >= 0 ? context.Materials[mesh->mMaterialIndex] : nullptr;
            resource.AddChild(pMeshData, pMeshData->Name());
            meshChild.AddComponent<MeshRenderer>(pMeshData, pMaterial);
        }

        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            ProcessNode(context, entity.GetEntityID(), node->mChildren[i], scene, resource);
        }
    }

    MeshData* ASSIMPImporter::ProcessMesh(Context& context, aiMesh* mesh) const
    {
        float* vertices = nullptr;
        std::vector<AttributeType> attributes;
        std::vector<uint32_t> indices;
        uint32_t vertexSize = 0;
        size_t arraySize = 0;

        if (mesh->mNormals == nullptr)
        {
            EditorApplication::GetInstance()->GetEngine()->GetDebug().LogError("Mesh has no normals");
            return nullptr;
        }

        attributes.push_back(AttributeType::Float3);
        attributes.push_back(AttributeType::Float3);
        attributes.push_back(AttributeType::Float3);
        attributes.push_back(AttributeType::Float3);
        attributes.push_back(AttributeType::Float2);
        attributes.push_back(AttributeType::Float4);
        vertexSize = sizeof(float) * 18;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            std::vector<float> vertexData;
            // process vertex positions, normals and texture coordinates
            glm::vec3 pos = glm::vec3{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z }*context.UnitScaleFactor;
            glm::vec3 normal = glm::vec3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z }*context.UnitScaleFactor;
            glm::vec3 tangent = glm::vec3{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z }*context.UnitScaleFactor;
            glm::vec3 biTangent = glm::vec3{ mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z }*context.UnitScaleFactor;

            /* Axes conversions */
            float temp;
            switch (context.UpAxis)
            {
            case AxisConversion::X:
                SWAP(pos, x, y);
                SWAP(normal, x, y);
                SWAP(tangent, x, y);
                SWAP(biTangent, x, y);
                break;
            case AxisConversion::Z:
                SWAP(pos, z, y);
                SWAP(normal, z, y);
                SWAP(tangent, z, y);
                SWAP(biTangent, z, y);
                break;
            default:
                break;
            }
            switch (context.FrontAxis)
            {
            case AxisConversion::X: {
                switch (context.UpAxis)
                {
                case AxisConversion::Z:
                    /* Z was already swapped with Y */
                    SWAP(pos, x, y);
                    SWAP(normal, x, y);
                    SWAP(tangent, x, y);
                    SWAP(biTangent, x, y);
                    break;
                default:
                    SWAP(pos, x, z);
                    SWAP(normal, x, z);
                    SWAP(tangent, x, z);
                    SWAP(biTangent, x, z);
                    break;
                }
                break;
            }
            case AxisConversion::Y:
                SWAP(pos, y, z);
                SWAP(normal, y, z);
                SWAP(tangent, y, z);
                SWAP(biTangent, y, z);
                break;
            default:
                break;
            }

            vertexData.push_back(pos.x);
            vertexData.push_back(pos.y*context.UpAxisSign);
            vertexData.push_back(pos.z*context.FrontAxisSign);
            vertexData.push_back(normal.x);
            vertexData.push_back(normal.y*context.UpAxisSign);
            vertexData.push_back(normal.z*context.FrontAxisSign);

            // Tangents and Bitangents
            vertexData.push_back(tangent.x);
            vertexData.push_back(tangent.y*context.UpAxisSign);
            vertexData.push_back(tangent.z*context.FrontAxisSign);
            vertexData.push_back(biTangent.x);
            vertexData.push_back(biTangent.y*context.UpAxisSign);
            vertexData.push_back(biTangent.z*context.FrontAxisSign);

            if (mesh->mTextureCoords[0])
            {
                vertexData.push_back(mesh->mTextureCoords[0][i].x);
                vertexData.push_back(1.0f - mesh->mTextureCoords[0][i].y);

                //if (i == 0)
                //{
                //    attributes.push_back(AttributeType::Float2);
                //    vertexSize += sizeof(float) * 2;
                //}
            }
            else
            {
                vertexData.push_back(0.0f);
                vertexData.push_back(0.0f);
            }

            if (mesh->mColors[0])
            {
                vertexData.push_back(mesh->mColors[0][i].r);
                vertexData.push_back(mesh->mColors[0][i].g);
                vertexData.push_back(mesh->mColors[0][i].b);
                vertexData.push_back(mesh->mColors[0][i].a);
            }
            else
            {
                vertexData.push_back(1.0f);
                vertexData.push_back(1.0f);
                vertexData.push_back(1.0f);
                vertexData.push_back(1.0f);
            }
            if (i == 0)
            {
                arraySize = mesh->mNumVertices * (vertexSize / sizeof(float));
                vertices = new float[arraySize];
            }

            //vertices[i] = RawVertex(vertexData);
            memcpy(&vertices[i * (vertexSize / sizeof(float))], &vertexData[0], vertexSize);
        }

        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        if (vertices == nullptr)
            return nullptr;

        std::vector<float> verticesVector;
        verticesVector.assign(&vertices[0], &vertices[arraySize]);
        std::stringstream stream;
        stream << mesh->mName.C_Str() << " Material " << mesh->mMaterialIndex;
        MeshData* pMesh = new MeshData(mesh->mNumVertices, vertexSize, verticesVector, (uint32_t)indices.size(), indices, attributes);
        pMesh->SetName(stream.str());
        delete[] vertices;
        return pMesh;
    }
}
