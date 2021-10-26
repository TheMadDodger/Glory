#include "ResourceMeta.h"
#include <fstream>
#include "Debug.h"

namespace Glory
{
	ResourceMeta::ResourceMeta(const std::string& path) : m_Path(path), m_Node(YAML::LoadFile(path)) {}
	
	ResourceMeta::ResourceMeta(const std::string& path, UUID uuid, size_t hash)
		: m_Path(path), m_UUID(uuid), m_TypeHash(hash) {}

	ResourceMeta::~ResourceMeta() {}

	uint64_t ResourceMeta::ReadUUID() const
	{
		YAML::Node node = m_Node["UUID"];
		if (!node.IsScalar())
		{
			Debug::LogError("Missing UUID in file: " + m_Path);
		}
		return node.as<uint64_t>();
	}

	size_t ResourceMeta::ReadHash() const
	{
		YAML::Node node = m_Node["Hash"];
		if (!node.IsScalar())
		{
			Debug::LogError("Missing Hash in file: " + m_Path);
		}
		return node.as<size_t>();
	}

	void ResourceMeta::Write(LoaderModule* pLoader, const std::string& extension)
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginMap;
		emitter << YAML::Key << "UUID";
		emitter << YAML::Value << m_UUID;
		emitter << YAML::Key << "Hash";
		emitter << YAML::Value << m_TypeHash;
		emitter << YAML::Key << "ImportSettings";
		emitter << YAML::Value << YAML::BeginMap;
		pLoader->WriteImportSettings(pLoader != nullptr ? pLoader->CreateDefaultImportSettings(extension) :
			ImportSettings(extension), emitter);
		emitter << YAML::EndMap;
		emitter << YAML::EndMap;
		emitter.SetOutputCharset(YAML::EscapeNonAscii);

		std::ofstream outStream(m_Path);
		outStream << emitter.c_str();
		outStream.close();
	}

	void ResourceMeta::Read()
	{
		m_Node = YAML::LoadFile(m_Path);
	}
}