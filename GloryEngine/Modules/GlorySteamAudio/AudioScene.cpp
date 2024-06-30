#include "AudioScene.h"

#include <BinaryStream.h>
#include <MeshData.h>

namespace Glory
{
	AudioScene::AudioScene(UUID sceneID): m_SceneID(sceneID) {}

	AudioScene::AudioScene(AudioScene&& other) noexcept:
		m_MeshDatas(std::move(other.m_MeshDatas)), m_Meshes(std::move(other.m_Meshes)),
		m_IsAssetBits(std::move(other.m_IsAssetBits))
	{
	}

	void AudioScene::AddMesh(UUID meshID)
	{
		const size_t index = m_Meshes.size();
		m_Meshes.push_back({ meshID });
		m_IsAssetBits.Reserve(m_Meshes.size());
		m_IsAssetBits.Set(index);
	}

	void AudioScene::AddMesh(MeshData&& meshData)
	{
		const size_t index = m_Meshes.size();
		m_MeshDatas.push_back(std::move(meshData));
		m_Meshes.push_back({ index });
		m_IsAssetBits.Reserve(m_Meshes.size());
		m_IsAssetBits.UnSet(index);
	}

	void AudioScene::Serialize(BinaryStream& stream) const
	{
		stream.Write(m_MeshDatas.size());
		for (size_t i = 0; i < m_MeshDatas.size(); ++i)
		{
			m_MeshDatas[i].Serialize(stream);
		}

		stream.Write(m_Meshes);
		stream.Write(m_IsAssetBits.Data(), m_IsAssetBits.DataSize());
	}

	void AudioScene::Deserialize(BinaryStream& stream)
	{
		size_t size;
		stream.Read(size);
		m_MeshDatas.resize(size);
		for (size_t i = 0; i < m_MeshDatas.size(); ++i)
		{
			m_MeshDatas[i].Deserialize(stream);
		}

		stream.Read(m_Meshes);
		m_IsAssetBits.Reserve(m_Meshes.size());
		stream.Read(m_IsAssetBits.Data(), m_IsAssetBits.DataSize());
	}

	UUID AudioScene::SceneID() const
	{
		return m_SceneID;
	}

	size_t AudioScene::MeshCount() const
	{
		return m_Meshes.size();
	}

	const MeshData& AudioScene::Mesh(size_t index) const
	{
		const AudioMesh& mesh = m_Meshes[index];
		if (!m_IsAssetBits.IsSet(index))
		{
			return m_MeshDatas[mesh.m_Mesh.m_Index];
		}
		throw new std::exception("Mesh assets are not yet supported for audio scenes");
	}

	MeshData& AudioScene::Mesh(size_t index)
	{
		const AudioMesh& mesh = m_Meshes[index];
		if (!m_IsAssetBits.IsSet(index))
		{
			return m_MeshDatas[mesh.m_Mesh.m_Index];
		}
		throw new std::exception("Mesh assets are not yet supported for audio scenes");
	}
}
