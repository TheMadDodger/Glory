#pragma once
#include <vector>

#include <UUID.h>
#include <AssetReference.h>
#include <BitSet.h>
#include <Glory.h>

namespace Glory
{
	class MeshData;
	class BinaryStream;

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
		/**
		 * @brief Constructor
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
		 */
		GLORY_API void AddMesh(UUID meshID);
		/**
		 * @brief Add a mesh to the scene
		 * @param meshData Mesh to add
		 */
		GLORY_API void AddMesh(MeshData&& meshData);

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

	private:
		UUID m_SceneID;
		std::vector<MeshData> m_MeshDatas;
		std::vector<AudioMesh> m_Meshes;
		Utils::BitSet m_IsAssetBits;
	};
}
