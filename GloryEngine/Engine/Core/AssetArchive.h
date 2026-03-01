#pragma once
#include "Version.h"

#include <vector>
#include <BitSet.h>

namespace Glory
{
	class Resource;
	class BinaryStream;
	class IEngine;

	enum AssetArchiveFlags
	{
		None = 0,
		Read = 1 << 0,
		Write = 1 << 1,
		WriteVersion = 1 << 2,
		WriteNew = Write | WriteVersion,
	};

	class AssetArchive
	{
	public:
		AssetArchive(BinaryStream* pStream, AssetArchiveFlags flags=AssetArchiveFlags::Read);
		AssetArchive(AssetArchive&& other) noexcept;
		virtual ~AssetArchive();

		bool VerifyVersion();

		void Serialize(Resource* pResource) const;
		void Serialize(const Resource* pResource) const;
		void Deserialize(IEngine* pEngine);

		size_t Size() const;
		Resource* Get(IEngine* pEngine, size_t index) const;

	private:
		void WriteVersion();
		void ReadVersion();

		Resource* ReadResource(IEngine* pEngine);

		BinaryStream* m_pStream;
		Version m_Version;
		std::vector<Resource*> m_pResources;
		mutable Utils::BitSet m_Owned;
	};
}
