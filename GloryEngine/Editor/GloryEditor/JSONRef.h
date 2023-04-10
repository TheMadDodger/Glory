#pragma once
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <filesystem>

namespace Glory
{
	struct JSONValueRef
	{
	public:
		JSONValueRef(rapidjson::Document& rootJSON, const std::filesystem::path& path);

		JSONValueRef operator[](const std::filesystem::path& path);
		JSONValueRef operator[](const size_t index);

		template<typename T>
		T As() const
		{
			return Value().Get<T>();
		}

		std::string_view AsString() const;

		template<typename T>
		T As(const T& defaultValue)
		{
			if (!Exists() || !Is<T>()) return defaultValue;
			return As<T>();
		}

		template<typename T>
		T AsEnum()
		{
			const std::string& valueStr = Value().as<std::string>();
			T value;
			if (!GloryReflect::Enum<T>().FromString(valueStr, value)) return T(0);
			return value;
		}

		template<typename T>
		void Set(const T& value)
		{
			Set(rapidjson::Value(value));
		}

		void SetString(const std::string& str);
		void SetInt(int value);

		void Set(rapidjson::Value& value);

		template<typename T>
		void PushBack(const T& value)
		{
			rapidjson::Value& v = Value();
			if (v.IsNull() && !v.IsArray())
			{
				v.SetArray();
			}
			v.PushBack(value, m_Document.GetAllocator());
		}

		void PushBackValue(rapidjson::Value& value);
		bool Remove(size_t index);
		bool Remove(const std::string& key);
		size_t Size();

		template<typename T>
		bool Is() const
		{
			return Value().Is<T>();
		}

		bool Exists();
		bool IsSequence();
		bool IsMap();
		rapidjson::Value& Value();
		const rapidjson::Value& Value() const;
		const std::filesystem::path& Path();
		void Erase();
		JSONValueRef Parent();

	private:
		rapidjson::Value& FindValue(rapidjson::Value& JSON, std::filesystem::path path);
		const rapidjson::Value& FindValue(const rapidjson::Value& JSON, std::filesystem::path path) const;

	private:
		rapidjson::Document& m_Document;
		const std::filesystem::path m_Path;
	};

	struct JSONFileRef
	{
	public:
		JSONFileRef();
		JSONFileRef(const std::filesystem::path& filePath);

		void Load();
		void Save();
		const std::filesystem::path& Path() const;

		JSONValueRef operator[](const std::filesystem::path& path);

	private:
		rapidjson::Document m_Document;
		std::filesystem::path m_FilePath;
	};
}
