#include "AssetManager.h"
#include "AssetDatabase.h"
#include "Engine.h"
#include "Debug.h"
#include "BinaryStream.h"
#include "AssetArchive.h"

#include <Hash.h>

namespace Glory
{
	AssetManager::AssetManager(Engine* pEngine) : m_pEngine(pEngine) {}

	AssetManager::~AssetManager()
	{
		m_pEngine = nullptr;
	}
}
