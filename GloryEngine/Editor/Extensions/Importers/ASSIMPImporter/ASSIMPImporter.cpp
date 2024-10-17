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

namespace Glory::Editor
{
    constexpr size_t NumSupportedExtensions = 4;
    constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
        ".obj",
        ".fbx",
        ".gltf",
        ".glb",
    };

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

        /* @todo: Use for unit conversion */
        //scene->mMetaData

        EditorPipelineManager& pipelines = EditorApplication::GetInstance()->GetPipelineManager();
        MaterialManager& materials = EditorApplication::GetInstance()->GetMaterialManager();
        const std::filesystem::path assetsPath = ProjectSpace::GetOpenProject()->RootPath();

        Context context;
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
                        aiColor3D color;
                        if (info->ShaderName() == "Color" && material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS)
                        {
                            pMaterial->Set(materials, info->ShaderName(), glm::vec4(color.r, color.g, color.b, 1.0f));
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

        ProcessNode(context, pScene->mRootNode, pScene, resource);

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

    void ASSIMPImporter::ProcessNode(Context& context, aiNode* node, const aiScene* scene, ImportedResource& resource) const
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshData* pMeshData = ProcessMesh(mesh);
            if (!pMeshData) continue;
            resource.AddChild(pMeshData, pMeshData->Name());
        }

        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            ProcessNode(context, node->mChildren[i], scene, resource);
        }
    }

    MeshData* ASSIMPImporter::ProcessMesh(aiMesh* mesh) const
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
            vertexData.push_back(mesh->mVertices[i].x);
            vertexData.push_back(mesh->mVertices[i].y);
            vertexData.push_back(mesh->mVertices[i].z);
            vertexData.push_back(mesh->mNormals[i].x);
            vertexData.push_back(mesh->mNormals[i].y);
            vertexData.push_back(mesh->mNormals[i].z);

            // Tangents and Bitangents
            vertexData.push_back(mesh->mTangents[i].x);
            vertexData.push_back(mesh->mTangents[i].y);
            vertexData.push_back(mesh->mTangents[i].z);
            vertexData.push_back(mesh->mBitangents[i].x);
            vertexData.push_back(mesh->mBitangents[i].y);
            vertexData.push_back(mesh->mBitangents[i].z);

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
