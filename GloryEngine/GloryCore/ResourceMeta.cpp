#include "ResourceMeta.h"
#include "Debug.h"
#include <filesystem>

namespace Glory
{
	ResourceMeta::ResourceMeta()
		: m_Path(""), m_Extension(""), m_Node(), m_TypeHash(0) {}

	ResourceMeta::ResourceMeta(const std::string& path, const std::string& extension)
		: m_Path(path), m_Extension(extension), m_Node(YAML::LoadFile(path)), m_TypeHash(0) {}
	
	ResourceMeta::ResourceMeta(const std::string& path, const std::string& extension, UUID uuid, size_t hash)
		: m_Path(path), m_Extension(extension), m_UUID(uuid), m_TypeHash(hash) {}

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

	void ResourceMeta::Write(LoaderModule* pLoader) const
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginMap;
		if (m_ImportSettings.has_value())
			Write(emitter, pLoader);
		else
			Write(emitter, pLoader, pLoader != nullptr ? pLoader->CreateDefaultImportSettings(m_Extension) : ImportSettings());
		emitter << YAML::EndMap;

		std::ofstream outStream(m_Path);
		outStream << emitter.c_str();
		outStream.close();
	}

	void ResourceMeta::Write(YAML::Emitter& emitter, LoaderModule* pLoader) const
	{
		Write(emitter, pLoader, m_ImportSettings);
	}

	void ResourceMeta::Write(YAML::Emitter& emitter, LoaderModule* pLoader, const std::any& importSettings) const
	{
		emitter << YAML::Key << "UUID";
		emitter << YAML::Value << m_UUID;
		emitter << YAML::Key << "Hash";
		emitter << YAML::Value << m_TypeHash;
		emitter << YAML::Key << "Extension";
		emitter << YAML::Value << m_Extension;
		if (pLoader)
		{
			emitter << YAML::Key << "ImportSettings";
			emitter << YAML::Value << YAML::BeginMap;
			pLoader->WriteImportSettings(importSettings, emitter);
			emitter << YAML::EndMap;
		}
		emitter.SetOutputCharset(YAML::EscapeNonAscii);
	}

	void ResourceMeta::Read()
	{
		m_Node = YAML::LoadFile(m_Path);
		m_UUID = ReadUUID();
		m_TypeHash = ReadHash();

		LoaderModule* pLoader = Game::GetGame().GetEngine()->GetLoaderModule(m_TypeHash);

		if (pLoader == nullptr) return;
		YAML::Node importSettingsNode = m_Node["ImportSettings"];
		if (!importSettingsNode.IsMap())
			return;
		m_ImportSettings = pLoader->ReadImportSettings(importSettingsNode);
	}

	const std::string& ResourceMeta::Path() const
	{
		return m_Path;
	}

	const std::string& ResourceMeta::Extension() const
	{
		return m_Extension;
	}

	UUID ResourceMeta::ID() const
	{
		return m_UUID;
	}

	size_t ResourceMeta::Hash() const
	{
		return m_TypeHash;
	}

	const std::any& ResourceMeta::ImportSettings() const
	{
		return m_ImportSettings;
	}

	bool ResourceMeta::Exists() const
	{
		return std::filesystem::exists(m_Path);
	}
}