#pragma once
#include <string>
#include <yaml-cpp/yaml.h>
#include "ResourceLoaderModule.h"
#include "ModelLoaderModule.h"
#include "Game.h"

namespace Glory
{
	class ResourceMeta
	{
	public:
		ResourceMeta(const std::string& path);
		ResourceMeta(const std::string& path, UUID uuid, size_t hash);
		virtual ~ResourceMeta();

		//void test()
		//{
		//	ModelLoaderModule* pLoader = Game::GetGame().GetEngine()->GetModule<ModelLoaderModule>();
		//	ModelImportSettings importSettings;
		//	//Read(pLoader, importSettings);
		//}

		//template<class T, typename S>
		//uint64_t Read(ResourceLoaderModule<T,S>* pLoader, S& importSettings)
		//{
		//	uint64_t uuid = ReadUUID();
		//	importSettings = pLoader->ReadImportSettings_Internal(m_Node);
		//}

		uint64_t ReadUUID() const;
		size_t ReadHash() const;

		void Write(LoaderModule* pLoader, const std::string& extension);
		void Read();

	private:
		YAML::Node m_Node;

		std::string m_Path;
		UUID m_UUID;
		size_t m_TypeHash;
	};
}
