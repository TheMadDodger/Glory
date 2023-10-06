#include "Importer.h"

#include <Debug.h>
#include <sstream>
#include <Resource.h>

namespace Glory::Editor
{
	std::vector<Importer*> Importer::m_pImporters;
	std::vector<Importer*> Importer::m_pOwnedImporters;

	Resource* Importer::Import(const std::filesystem::path& path, void* pImportSettings)
	{
		path.extension();
		auto itor = std::find_if(m_pImporters.begin(), m_pImporters.end(), [&](const Importer* pImporter) {
			return pImporter->SupportsExtension(path.extension());
		});

		if (itor == m_pImporters.end())
		{
			std::stringstream str;
			str << "Could not find importer for extension: " << path.extension() << " for file: " << path;
			Debug::LogWarning(str.str());
			return nullptr;
		}

		std::stringstream str;
		str << "Found importer " << (*itor)->Name() << " for loading " << path.extension() << " files to import: " << path;
		Debug::LogInfo(str.str());

		return (*itor)->Load(path);
	}

	void Importer::Register(Importer* pImporter)
	{
		m_pImporters.push_back(pImporter);
		pImporter->Initialize();
	}

	void Importer::RegisterOwned(Importer* pImporter)
	{
		m_pOwnedImporters.push_back(pImporter);
		m_pImporters.push_back(pImporter);
	}

	void Importer::CleanupAll()
	{
		for (size_t i = 0; i < m_pImporters.size(); ++i)
		{
			m_pImporters[i]->Cleanup();
		}
		m_pImporters.clear();

		for (size_t i = 0; i < m_pOwnedImporters.size(); ++i)
		{
			delete m_pOwnedImporters[i];
		}
		m_pOwnedImporters.clear();

	}
}
