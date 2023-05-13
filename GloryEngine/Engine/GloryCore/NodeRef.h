#pragma once
#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace Glory
{
	struct NodeValueRef
	{
	public:
		NodeValueRef(YAML::Node& rootNode, const std::filesystem::path& path);

		NodeValueRef operator[](const std::filesystem::path& path);
		NodeValueRef operator[](const size_t index);

		template<typename T>
		T As()
		{
			return Node().as<T>();
		}

		template<typename T>
		T As(const T& defaultValue)
		{
			if (!Exists() || !IsScalar()) return defaultValue;
			return As<T>();
		}

		template<typename T>
		T AsEnum()
		{
			const std::string& valueStr = Node().as<std::string>();
			T value;
			if (!GloryReflect::Enum<T>().FromString(valueStr, value)) return T(0);
			return value;
		}

		template<typename T>
		void Set(const T& value)
		{
			Node() = value;
		}

		template<typename T>
		void SetEnum(const T& value)
		{
			std::string valueStr;
			if (!GloryReflect::Enum<T>().ToString(value, valueStr)) {
				Set("");
				return;
			}
			Set(valueStr);
		}

		void Set(YAML::Node& node);

		template<typename T>
		void PushBack(const T& value)
		{
			Node().push_back(value);
		}

		void PushBack(YAML::Node& node);
		void Remove(size_t index);
		void Remove(const std::string& key);
		void Insert(YAML::Node& node, size_t index);
		size_t Size();

		bool Exists();
		bool IsScalar();
		bool IsSequence();
		bool IsMap();
		YAML::Node Node();
		const std::filesystem::path& Path();
		void Erase();
		NodeValueRef Parent();

	private:
		YAML::Node FindNode(YAML::Node& node, std::filesystem::path path);

	private:
		YAML::Node& m_RootNode;
		const std::filesystem::path m_Path;
	};

	struct NodeRef
	{
	public:
		NodeRef(YAML::Node& node);

		NodeValueRef operator[](const std::filesystem::path& path);
		NodeValueRef ValueRef();

	private:
		YAML::Node& m_RootNode;
		NodeValueRef m_RootValueRef;
	};

	struct YAMLFileRef
	{
	public:
		YAMLFileRef();
		YAMLFileRef(const std::filesystem::path& filePath);

		void Load();
		void Save();
		const std::filesystem::path& Path() const;

		NodeRef RootNodeRef();
		NodeValueRef operator[](const std::filesystem::path& path);

	private:
		YAML::Node m_RootNode;
		std::filesystem::path m_FilePath;
	};
}
