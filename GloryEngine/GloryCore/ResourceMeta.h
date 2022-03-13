#pragma once
#include <string>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "ResourceLoaderModule.h"
#include "ModelLoaderModule.h"
#include "Game.h"

namespace Glory
{
	class ResourceMeta
	{
	public:
		ResourceMeta();
		ResourceMeta(const std::filesystem::path& path, const std::string& extension);
		ResourceMeta(const std::string& path, const std::string& extension, UUID uuid, size_t hash);
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

		void Write(LoaderModule* pLoader) const;
		void Write(YAML::Emitter& emitter, LoaderModule* pLoader) const;
		void Write(YAML::Emitter& emitter, LoaderModule* pLoader, const std::any& importSettings) const;
		void Read();

		const std::string& Path() const;
		const std::string& Extension() const;
		UUID ID() const;
		size_t BaseHash() const;
		size_t Hash() const;
		size_t SerializedVersion() const;
		const std::any& ImportSettings() const;
		bool Exists() const;
		
	private:
		uint64_t ReadUUID() const;
		size_t ReadHash() const;
		size_t ReadSerializedVersion() const;

	private:
		friend class AssetDatabase;
		YAML::Node m_Node;

		std::string m_Path;
		std::string m_Extension;
		UUID m_UUID;
		size_t m_TypeHash;
		size_t m_SerializedVersion;
		std::any m_ImportSettings;
	};
}

namespace std
{
	template<>
	struct hash<Glory::ResourceMeta>
	{
		std::size_t operator()(const Glory::ResourceMeta& meta) const
		{
			std::ifstream inStream(meta.Path());
			std::stringstream buffer;
			buffer << inStream.rdbuf();
			inStream.close();
			std::string str = buffer.str();
			return std::hash<std::string>()(str);
		}
	};
}
