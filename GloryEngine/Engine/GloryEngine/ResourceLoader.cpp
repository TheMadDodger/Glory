#include "ResourceLoader.h"
#include "Resources.h"

namespace Glory
{
	ResourceLoader::ResourceLoader()
	{
	}

	ResourceLoader::~ResourceLoader()
	{
		m_pResources = nullptr;
	}

	void ResourceLoader::Update()
	{
		m_pResources->HandleToUnload([this](UUID id) {
			QueueUnload(id);
		});
		m_pResources->HandleToLoad([this](UUID id) {
			QueueLoad(id);
		});
		m_pResources->HandleToLoadImmediately([this](UUID id) {
			QueueLoad(id);
		});

		OnUpdate();

		m_UpdateCounter.fetch_add(1ull);
	}

	void ResourceLoader::SetResources(Resources* pResources)
	{
		m_pResources = pResources;
	}

	Resource* ResourceLoader::LoadImmediately(UUID id)
	{
		QueueLoad(id, true);
		return m_pResources->GetResource(id);
	}

	void ResourceLoader::WaitForNextUpdate()
	{
		const uint64_t lastUpdate = m_UpdateCounter.load();
		while (lastUpdate == m_UpdateCounter.load())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
