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
		ResourceMeta(const std::string& extension, UUID uuid, size_t hash);
		virtual ~ResourceMeta();

		void Read(const YAML::Node& node);
		const std::string& Extension() const;
		UUID ID() const;
		size_t Hash() const;
		size_t SerializedVersion() const;
		void IncrementSerializedVersion();

	private:
		uint64_t ReadUUID() const;
		size_t ReadHash() const;
		size_t ReadSerializedVersion() const;

	private:
		friend class AssetDatabase;
		YAML::Node m_Node;
		std::string m_Extension;
		UUID m_UUID;
		size_t m_TypeHash;
		size_t m_SerializedVersion;
	};
}

namespace std
{
	//template<>
	//struct hash<Glory::ResourceMeta>
	//{
	//	std::size_t operator()(const Glory::ResourceMeta& meta) const
	//	{
	//		std::ifstream inStream(meta.Path());
	//		std::stringstream buffer;
	//		buffer << inStream.rdbuf();
	//		inStream.close();
	//		std::string str = buffer.str();
	//		return std::hash<std::string>()(str);
	//	}
	//};
}

#include <yaml-cpp/yaml.h>
namespace YAML
{
	Emitter& operator<<(Emitter& out, const Glory::ResourceMeta& meta);

	template<>
	struct convert<Glory::ResourceMeta>
	{
		static Node encode(const Glory::ResourceMeta& meta)
		{
			Node node;
			node = YAML::Node(YAML::NodeType::Map);
			node["Extension"] = meta.Extension();
			node["UUID"] = (uint64_t)meta.ID();
			node["Hash"] = (uint64_t)meta.Hash();
			node["SerializedVersion"] = (uint64_t)meta.SerializedVersion();
			return node;
		}

		static bool decode(const Node& node, Glory::ResourceMeta& meta)
		{
			if (!node.IsMap())
				return false;

			meta.Read(node);
			return true;
		}
	};
}
