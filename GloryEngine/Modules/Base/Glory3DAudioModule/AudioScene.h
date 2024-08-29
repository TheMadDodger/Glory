#pragma once
#include <vector>

#include <UUID.h>
#include <AssetReference.h>
#include <SoundMaterialData.h>
#include <BitSet.h>
#include <Glory.h>
#include <Resource.h>

namespace Glory
{
	class MeshData;
	class BinaryStream;
	struct SoundMaterial;

	/**
	 * @brief Meta data of an audio mesh
	 *
	 * The mesh can either be a generated mesh in the @ref AudioScene
	 * or a mesh asset from the project.
	 */
	struct AudioMesh
	{
		AudioMesh() = default;

		union MeshUnion
		{
			UUID m_AssetID;
			size_t m_Index;
		} m_Mesh{0};

		size_t m_Material;
	};

	/**
	 * @brief Audio occlusion scene
	 *
	 * This scene contains a collection of meshes that can
	 * be used to do audio occlusion checks against.
	 */
	class AudioScene
	{
	public:
		/** @brief Constructor */
		GLORY_API AudioScene();
		/**
		 * @overload
		 * @param sceneID ID of the @ref GScene this audio scene is created for
		 */
		GLORY_API AudioScene(UUID sceneID);
		/** @overload */
		GLORY_API AudioScene(AudioScene&& other) noexcept;
		/** @brief Destructor */
		GLORY_API ~AudioScene() = default;

		/**
		 * @brief Add a mesh asset reference to the scene
		 * @param meshID ID of the mesh asset to add
		 * @param material Material of the mesh
		 */
		GLORY_API void AddMesh(UUID meshID, SoundMaterial&& material);
		/**
		 * @brief Add a mesh to the scene
		 * @param meshData Mesh to add
		 * @param material Material of the mesh
		 */
		GLORY_API void AddMesh(MeshData&& meshData, SoundMaterial&& material);

		/** @brief Serialize this audio scene into a stream */
		GLORY_API void Serialize(BinaryStream& stream) const;
		/** @brief Deserialize a stream into this audio scene */
		GLORY_API void Deserialize(BinaryStream& stream);

		/** @brief ID of the scene @ref GScene this audio scene was created for */
		GLORY_API UUID SceneID() const;

		/** @brief Number of meshes in this audio scene */
		GLORY_API size_t MeshCount() const;
		/**
		 * @brief Get a mesh in this audio scene
		 * @param index Index of the mesh
		 */
		GLORY_API const MeshData& Mesh(size_t index) const;
		/** @overload */
		GLORY_API MeshData& Mesh(size_t index);
		/**
		 * @brief Get the material for a mesh in this audio scene
		 * @param index Index of the mesh
		 */
		GLORY_API const SoundMaterial& Material(size_t index) const;
		/** @overload */
		GLORY_API SoundMaterial& Material(size_t index);

	private:
		UUID m_SceneID;
		std::vector<MeshData> m_MeshDatas;
		std::vector<AudioMesh> m_Meshes;
		std::vector<SoundMaterial> m_Materials;
		Utils::BitSet m_IsAssetBits;
	};

	/** @brief Resource wrapper for AudioScene to be used for asset serialization */
	class AudioSceneData : public Resource
	{
	public:
		/** @brief Constructor */
		GLORY_API AudioSceneData();
		/** @overload */
		GLORY_API AudioSceneData(AudioScene&& audioScene);
		/** @brief Destructor */
		GLORY_API virtual ~AudioSceneData() {}

		GLORY_API void Serialize(BinaryStream& container) const override;
		GLORY_API void Deserialize(BinaryStream& container) override;

		AudioScene m_AudioScene;
	};
}
