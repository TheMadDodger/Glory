#include "ASSIMPModule.h"
#include <assimp/postprocess.h>
#include <Debug.h>
#include "VertexDefinitions.h"

namespace Glory
{
	ASSIMPModule::ASSIMPModule()
	{
	}

	ASSIMPModule::~ASSIMPModule()
	{
	}

	void ASSIMPModule::Initialize()
	{
		
	}

	void ASSIMPModule::Cleanup()
	{
		
	}

	ModelData* ASSIMPModule::LoadModel(const std::string& path, const ModelImportSettings& importSettings)
	{
		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(path, aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

        //const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        //const aiScene* pScene = importer.ReadFile(path, 0);

		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
		{
			Debug::LogError("ASSIMP: Could not import file: " + path + " Error: " + importer.GetErrorString());
			return nullptr;
		}
        //directory = file.substr(0, file.find_last_of('/'));

        ModelData* pModel = new ModelData();
        ProcessNode(pScene->mRootNode, pScene, pModel);

		importer.FreeScene();

		return pModel;
	}

    ModelData* ASSIMPModule::LoadModel(const void* buffer, size_t length, const ModelImportSettings& importSettings)
    {
        Assimp::Importer importer;

        const aiScene* pScene = importer.ReadFileFromMemory(buffer, length, aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType, importSettings.m_Extension.c_str());

        if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
        {
            Debug::LogError("ASSIMP: Could not import file from memeory Error: " + std::string(importer.GetErrorString()));
            return nullptr;
        }
        //directory = file.substr(0, file.find_last_of('/'));

        ModelData* pModel = new ModelData();
        ProcessNode(pScene->mRootNode, pScene, pModel);

        importer.FreeScene();

        return pModel;
    }

    void ASSIMPModule::ProcessNode(aiNode* node, const aiScene* scene, ModelData* pModel)
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            MeshData* pMeshData = ProcessMesh(mesh);
            pModel->AddMesh(pMeshData);
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene, pModel);
        }
    }

    MeshData* ASSIMPModule::ProcessMesh(aiMesh* mesh)
    {
        float* vertices = nullptr;
        std::vector<AttributeType> attributes;
        std::vector<uint32_t> indices;
        size_t vertexSize = 0;
        size_t arraySize = 0;
        //vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            if (i == 0)
            {
                attributes.push_back(AttributeType::Float3);
                attributes.push_back(AttributeType::Float3);
                vertexSize = sizeof(float) * 6;

                //vertexSize += sizeof(Vector3) * 2;
            }

            std::vector<float> vertexData;
            // process vertex positions, normals and texture coordinates
            vertexData.push_back(mesh->mVertices[i].x);
            vertexData.push_back(mesh->mVertices[i].y);
            vertexData.push_back(mesh->mVertices[i].z);
            vertexData.push_back(mesh->mNormals[i].x);
            vertexData.push_back(mesh->mNormals[i].y);
            vertexData.push_back(mesh->mNormals[i].z);

            if (mesh->mTextureCoords[0])
            {
                vertexData.push_back(mesh->mTextureCoords[0][i].x);
                vertexData.push_back(1.0f - mesh->mTextureCoords[0][i].y);
            
                if (i == 0)
                {
                    attributes.push_back(AttributeType::Float2);
                    vertexSize += sizeof(float) * 2;
                }
            }

            //if (mesh->HasTangentsAndBitangents())
            //{
            //    vertexData.push_back(mesh->mTangents[i].x);
            //    vertexData.push_back(mesh->mTangents[i].y);
            //    vertexData.push_back(mesh->mTangents[i].z);
            //    vertexData.push_back(mesh->mBitangents[i].x);
            //    vertexData.push_back(mesh->mBitangents[i].y);
            //    vertexData.push_back(mesh->mBitangents[i].z);

            //    if (i == 0)
            //    {
            //        attributes.push_back(MeshAttribute::ATangent);
            //        attributes.push_back(MeshAttribute::ABiNormal);

            //        vertexSize += sizeof(Vector3) * 2;
            //    }
            //}
            if (i == 0)
            {
                arraySize = mesh->mNumVertices * (vertexSize / sizeof(float));
                vertices = new float[arraySize];
            }

            //vertices[i] = RawVertex(vertexData);
            memcpy(&vertices[i * (vertexSize / sizeof(float))], &vertexData[0], vertexSize);
        }

        /*for (size_t i = 0; i < mesh->mNumVertices * vertexSize; i++)
        {
            Utilities::Debug::LogInfo(std::to_string(vertices[i]));
        }*/

        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        std::vector<float> verticesVector;
        verticesVector.assign(&vertices[0], &vertices[arraySize]);
        MeshData* pMesh = new MeshData(mesh->mNumVertices, vertexSize, verticesVector, indices.size(), indices, attributes);
        //pMesh->BuildMesh();
        delete[] vertices;
        return pMesh;
    }
}
