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
}
