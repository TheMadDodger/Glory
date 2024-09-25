#pragma once
#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace Glory::Utils
{
	/**
	 * @brief Reference to a node value in a YAML document
	 */
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
			if (!Enum<T>().FromString(valueStr, value)) return T(0);
			return value;
		}

		template<typename T>
		T AsEnum(const T& defaultValue)
		{
			if (!Exists()) return defaultValue;
			return AsEnum<T>();
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
			if (!Enum<T>().ToString(value, valueStr)) {
				Set("");
				return;
			}
			Set(valueStr);
		}

		void Set(YAML::Node& node);
		
		void SetMap();
		void SetSequence();
		void SetNull();
		void SetScalar();

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

		bool Exists() const;
		bool IsScalar() const;
		bool IsSequence() const;
		bool IsMap() const;
		YAML::Node Node();
		const YAML::Node Node() const;
		const std::filesystem::path& Path();
		void Erase();
		NodeValueRef Parent();

		struct Iterator
		{
			Iterator(YAML::const_iterator iter):
				m_Iter(iter) {}

			const std::string operator*() const { return m_Iter->first.as<std::string>(); }

			// Prefix increment
			Iterator& operator++() { ++m_Iter; return *this; }

			// Postfix increment
			Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

			friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_Iter == b.m_Iter; };
			friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_Iter != b.m_Iter; };

		private:
			YAML::const_iterator m_Iter;
		};

		Iterator Begin() const;
		Iterator End() const;

	private:
		YAML::Node FindNode(YAML::Node& node, std::filesystem::path path);
		const YAML::Node FindNode(YAML::Node& node, std::filesystem::path path) const;

	private:
		YAML::Node& m_RootNode;
		const std::filesystem::path m_Path;
	};

	/**
	 * @brief Reference to a node in a YAML document
	 */
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

	/**
	 * @brief YAML document held in memory
	 */
	struct InMemoryYAML
	{
	public:
		InMemoryYAML();
		InMemoryYAML(const char* data);

		NodeRef RootNodeRef();
		NodeValueRef operator[](const std::filesystem::path& path);

		std::string ToString();

		bool ParsingFailed() const;

	protected:
		YAML::Node m_RootNode;
		bool m_ParsingFailed;
	};

	/**
	 * @brief Reference to a YAML file
	 */
	struct YAMLFileRef : public InMemoryYAML
	{
	public:
		YAMLFileRef();
		YAMLFileRef(const std::filesystem::path& filePath);

		void Load();
		void Save();
		const std::filesystem::path& Path() const;

		void ChangePath(const std::filesystem::path& newPath);

	private:
		std::filesystem::path m_FilePath;
	};
}
