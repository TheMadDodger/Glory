#pragma once
#include "Version.h"

#include <vector>
#include <BitSet.h>

namespace Glory
{
	class Resource;
	class BinaryStream;
	class Engine;

	class AssetArchive
	{
	public:
		AssetArchive(BinaryStream* pStream, bool isNew=false);
		AssetArchive(AssetArchive&& other) noexcept;
		virtual ~AssetArchive();

		bool VerifyVersion();

		void Serialize(Resource* pResource);
		void Deserialize(Engine* pEngine);

		size_t Size() const;
		Resource* Get(Engine* pEngine, size_t index) const;

	private:
		void WriteVersion();
		void ReadVersion();

		Resource* ReadResource(Engine* pEngine);

		BinaryStream* m_pStream;
		Version m_Version;
		std::vector<Resource*> m_pResources;
		mutable Utils::BitSet m_Owned;
	};
}
