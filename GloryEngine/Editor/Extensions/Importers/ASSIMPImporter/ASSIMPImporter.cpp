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

#include <EntityRegistry.h>

namespace Glory::Editor
{
    constexpr size_t NumSupportedExtensions = 9;
    constexpr std::string_view SupportedExtensions[NumSupportedExtensions] = {
        ".obj",
        ".fbx",
        ".gltf",
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
        Utils::ECS::EntityRegistry registry;

        PrefabData* pPrefab = new PrefabData();
        ProcessNode(pPrefab, 0, pScene->mRootNode, pScene, resource);
        resource.AddChild(pPrefab, path.filename().replace_extension().string() + "_Prefab");

        importer.FreeScene();
        return resource;
    }

    void ASSIMPImporter::ProcessNode(PrefabData* pPrefab, Utils::ECS::EntityID parent, aiNode* node, const aiScene* scene, ImportedResource& resource) const
    {
        // process all the node's meshes (if any)
        aiVector3D scale, position;
        aiQuaternion rotation;
        node->mTransformation.Decompose(scale, rotation, position);

        const glm::vec3 convertedScale{ scale.x, scale.y, scale.z };
        const glm::quat convertedQuat{ rotation.w, rotation.x, rotation.y, rotation.z };
        const glm::vec3 convertedPos{ position.x, position.y, position.z };

        Entity entity = pPrefab->CreateEmptyObject(node->mName.C_Str());
        Transform& transform = entity.GetComponent<Transform>();
        transform.Position = convertedPos;
        transform.Rotation = convertedQuat;
        transform.Scale = convertedScale;

        if (parent)
            entity.SetParent(parent);

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            Entity meshChild = pPrefab->CreateEmptyObject(node->mName.C_Str());
            meshChild.SetParent(entity.GetEntityID());
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshData* pMeshData = ProcessMesh(mesh);
            if (!pMeshData) continue;
            resource.AddChild(pMeshData, pMeshData->Name());
            meshChild.AddComponent<MeshRenderer>(pMeshData, nullptr);
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            ProcessNode(pPrefab, entity.GetEntityID(), node->mChildren[i], scene, resource);
        }
    }

    MeshData* ASSIMPImporter::ProcessMesh(aiMesh* mesh) const
    {
        float* vertices = nullptr;
        std::vector<AttributeType> attributes;
        std::vector<uint32_t> indices;
        uint32_t vertexSize = 0;
        size_t arraySize = 0;
        //vector<Texture> textures;

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
