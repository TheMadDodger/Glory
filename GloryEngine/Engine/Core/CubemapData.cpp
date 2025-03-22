#include "CubemapData.h"
#include "BinaryStream.h"
#include "ImageData.h"

namespace Glory
{
	CubemapData::CubemapData()
	{
		APPEND_TYPE(CubemapData);
	}

	CubemapData::CubemapData(UUID right, UUID left, UUID down, UUID up, UUID front, UUID back):
		m_Faces{ right, left, down, up, front, back }
	{
		APPEND_TYPE(CubemapData);
	}

	CubemapData::CubemapData(ImageData* pRight, ImageData* pLeft, ImageData* pDown, ImageData* pUp, ImageData* pFront, ImageData* pBack):
		m_Faces{ pRight->GetUUID(), pLeft->GetUUID(), pDown->GetUUID(), pUp->GetUUID(), pFront->GetUUID(), pBack->GetUUID() }
	{
		APPEND_TYPE(CubemapData);
	}

	CubemapData::~CubemapData()
	{
	}

	ImageData* CubemapData::GetImageData(AssetManager* pAssetManager, size_t face)
	{
		auto& faceImage = Image(face);
		return faceImage ? faceImage.Get(pAssetManager) : nullptr;
	}

	AssetReference<ImageData>& CubemapData::Image(size_t face)
	{
		return m_Faces[face];
	}

	SamplerSettings& CubemapData::GetSamplerSettings()
	{
		return m_SamplerSettings;
	}

	void CubemapData::Serialize(BinaryStream& container) const
	{
		for (size_t i = 0; i < 6; ++i)
		{
			const UUID faceID = m_Faces[i].AssetUUID();
			container.Write(faceID);
		}
		container.Write(m_SamplerSettings);
	}

	void CubemapData::Deserialize(BinaryStream& container)
	{
		for (size_t i = 0; i < 6; ++i)
		{
			const UUID faceID = m_Faces[i].AssetUUID();
			container.Read(*m_Faces[i].AssetUUIDMember());
		}
		container.Read(m_SamplerSettings);
	}

	void CubemapData::References(Engine* pEngine, std::vector<UUID>& references) const
	{
		for (size_t i = 0; i < 6; ++i)
		{
			const UUID faceID = m_Faces[i].AssetUUID();
			if (faceID) references.push_back(faceID);
		}
	}
}
