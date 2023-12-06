#pragma once
#include "Version.h"

#include <vector>
#include <BitSet.h>

namespace Glory
{
	class Resource;
	class BinaryStream;

	class AssetArchive
	{
	public:
		AssetArchive();
		AssetArchive(BinaryStream* pStream, bool isNew=false);
		virtual ~AssetArchive();

		bool VerifyVersion();

		void Serialize(Resource* pResource);
		void Deserialize();

		size_t Size() const;
		Resource* Get(size_t index) const;

		operator bool() const;

	private:
		void WriteVersion();
		void ReadVersion();

		Resource* ReadResource();

		BinaryStream* m_pStream;
		Version m_Version;
		std::vector<Resource*> m_pResources;
		mutable Utils::BitSet m_Owned;
	};
}
