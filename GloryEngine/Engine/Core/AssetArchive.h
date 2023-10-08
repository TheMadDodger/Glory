#pragma once

namespace Glory
{
	class Resource;
	class BinaryStream;

	class AssetArchive
	{
	public:
		AssetArchive(BinaryStream* pStream);
		virtual ~AssetArchive();

		void Serialize(Resource* pResource);
		void Deserialize(Resource* pResource);

	private:
		BinaryStream* m_pStream;
	};
}
