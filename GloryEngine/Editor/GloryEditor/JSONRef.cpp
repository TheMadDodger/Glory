#include "JSONRef.h"

#include <rapidjson/prettywriter.h>

#include <fstream>
#include <Debug.h>

namespace Glory
{
	JSONValueRef::JSONValueRef(rapidjson::Document& doc, const std::filesystem::path& path)
		: m_Document(doc), m_Path(path)
	{
	}

	JSONValueRef JSONValueRef::operator[](const std::filesystem::path& path)
	{
		std::filesystem::path fsPath = m_Path;
		fsPath.append(path.string());
		return JSONValueRef(m_Document, fsPath.string());
	}

	JSONValueRef JSONValueRef::operator[](const size_t index)
	{
		std::filesystem::path fsPath = m_Path;
		fsPath.append("##" + std::to_string(index));
		return JSONValueRef(m_Document, fsPath);
	}

	std::string_view JSONValueRef::AsString() const
	{
		return Value().GetString();
	}

	int JSONValueRef::AsInt() const
	{
		return Value().GetInt();
	}

	uint32_t JSONValueRef::AsUInt() const
	{
		return Value().GetUint();
	}

	uint64_t JSONValueRef::AsUInt64() const
	{
		return Value().GetUint64();
	}

	void JSONValueRef::SetObject()
	{
		Value().SetObject();
	}

	void JSONValueRef::SetArray()
	{
		Value().SetArray();
	}

	void JSONValueRef::SetString(const std::string& str)
	{
		Value().SetString(str.data(), m_Document.GetAllocator());
	}

	void JSONValueRef::SetInt(int value)
	{
		Value().SetInt(value);
	}

	void JSONValueRef::SetUInt(uint32_t value)
	{
		Value().SetUint(value);
	}

	void JSONValueRef::SetUInt64(uint64_t value)
	{
		Value().SetUint64(value);
	}

	void JSONValueRef::Set(rapidjson::Value& value)
	{
		Value() = value;
	}

	void JSONValueRef::PushBackValue(rapidjson::Value& value)
	{
		rapidjson::Value& v = Value();
		if (v.IsNull() && !v.IsArray())
		{
			v.SetArray();
		}
		v.PushBack(value, m_Document.GetAllocator());
	}

	bool JSONValueRef::Remove(size_t index)
	{
		rapidjson::Value& value = Value();
		rapidjson::Value* itor = value.Begin() + index;
		if (!itor || itor == value.End()) return false;
		value.Erase(itor);
		return true;
	}

	bool JSONValueRef::Remove(const std::string& key)
	{
		return Value().RemoveMember(key.data());
	}

	size_t JSONValueRef::Size()
	{
		return Value().Size();
	}

	bool JSONValueRef::Exists() const
	{
		return Exists(m_Document, m_Path);
	}

	bool JSONValueRef::IsSequence() const
	{
		return Value().IsArray();
	}

	bool JSONValueRef::IsObject() const
	{
		return Value().IsObject();
	}

	const std::filesystem::path& JSONValueRef::Path()
	{
		return m_Path;
	}

	void JSONValueRef::Erase()
	{
		JSONValueRef parent = Parent();
		const std::filesystem::path& path = m_Path.lexically_relative(parent.Path());
		const std::string& pathString = path.string();
		if (pathString._Starts_with("##"))
		{
			const size_t index = std::stoul(pathString.substr(2));
			parent.Remove(index);
			return;
		}
		parent.Remove(path.string());
	}

	JSONValueRef JSONValueRef::Parent()
	{
		return JSONValueRef(m_Document, m_Path.parent_path());
	}

	rapidjson::Value::ConstMemberIterator JSONValueRef::begin() const
	{
		return Value().MemberBegin();
	}

	rapidjson::Value::ConstMemberIterator JSONValueRef::end() const
	{
		return Value().MemberEnd();
	}

	std::vector<std::string_view> JSONValueRef::Keys() const
	{
		std::vector<std::string_view> keys;
		for (rapidjson::Value::ConstMemberIterator itor = begin(); itor != end(); ++itor)
		{
			keys.push_back(itor->name.GetString());
		}
		return keys;
	}

	std::vector<UUID> JSONValueRef::IDKeys() const
	{
		std::vector<UUID> keys;
		for (rapidjson::Value::ConstMemberIterator itor = begin(); itor != end(); ++itor)
		{
			keys.push_back(std::stoull(itor->name.GetString()));
		}
		return keys;
	}

	rapidjson::Value& JSONValueRef::FindValue(rapidjson::Value& value, const std::filesystem::path& path)
	{
		if (path.empty() || path == ".") return value;

		rapidjson::Value* pNext = &value;
		for (const std::filesystem::path& subPath : path)
		{
			const std::string& pathStr = subPath.string();

			if (pathStr._Starts_with("##"))
			{
				const size_t index = std::stoul(pathStr.substr(2).data());
				pNext = &(*pNext)[index];
				continue;
			}

			if (pNext->IsNull() && !pNext->IsObject())
			{
				pNext->SetObject();
			}

			if (!pNext->HasMember(pathStr.data()))
			{
				rapidjson::Value name = rapidjson::Value(pathStr.data(), m_Document.GetAllocator());
				pNext->AddMember(name, rapidjson::Value(rapidjson::kNullType), m_Document.GetAllocator());
			}

			pNext = &(*pNext)[pathStr.data()];
		}
		return *pNext;
	}

	const rapidjson::Value& JSONValueRef::FindValue(const rapidjson::Value& value, const std::filesystem::path& path) const
	{
		if (path.empty() || path == ".") return value;

		const rapidjson::Value* pNext = &value;
		for (const std::filesystem::path& subPath : path)
		{
			const std::string& pathStr = subPath.string();

			if (pathStr._Starts_with("##"))
			{
				const size_t index = std::stoul(pathStr.substr(2).data());
				pNext = &(*pNext)[index];
				continue;
			}

			pNext = &(*pNext)[pathStr.data()];
		}
		return *pNext;
	}

	bool JSONValueRef::Exists(const rapidjson::Value& value, const std::filesystem::path& path) const
	{
		if (path.empty() || path == ".") return true;

		const rapidjson::Value* pNext = &value;
		for (const std::filesystem::path& subPath : path)
		{
			const std::string& pathStr = subPath.string();

			if (pathStr._Starts_with("##"))
			{
				const size_t index = std::stoul(pathStr.substr(2).data());
				if (!pNext->IsArray() || pNext->Size() <= index) return false;
				pNext = &(*pNext)[index];
				continue;
			}

			if (!pNext->IsObject() || !pNext->HasMember(pathStr.data())) return false;
			pNext = &(*pNext)[pathStr.data()];
		}
		return true;
	}

	rapidjson::Value& JSONValueRef::Value()
	{
		if (m_Path.empty() || m_Path == ".") return m_Document;
		return FindValue(m_Document, m_Path);
	}

	const rapidjson::Value& JSONValueRef::Value() const
	{
		if (m_Path.empty() || m_Path == ".") return m_Document;
		return FindValue(m_Document, m_Path);
	}

	JSONFileRef::JSONFileRef() : m_FilePath(""), m_Document(rapidjson::kObjectType)
	{
	}

	JSONFileRef::JSONFileRef(const std::filesystem::path& filePath) : m_FilePath(filePath), m_Document(rapidjson::kObjectType)
	{
	}

	void JSONFileRef::Load()
	{
		if (!std::filesystem::exists(m_FilePath)) return;
		std::ifstream fstream{m_FilePath, std::ios::ate};

		if (!fstream.is_open())
		{
			Debug::LogError("Could not open file: " + m_FilePath.string());
			return;
		}

		std::vector<char> buffer;
		const size_t fileSize = (size_t)fstream.tellg();
		buffer.resize(fileSize);
		fstream.seekg(0);
		fstream.read(buffer.data(), fileSize);
		//buffer.push_back('\0');
		fstream.close();
		m_Document.Parse(buffer.data());
	}

	void JSONFileRef::Save()
	{
		std::ofstream out{m_FilePath};
		rapidjson::StringBuffer strbuf;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
		//writer.SetFormatOptions(rapidjson::)
		m_Document.Accept(writer);
		out << strbuf.GetString();
		out.close();
	}

	const std::filesystem::path& JSONFileRef::Path() const
	{
		return m_FilePath;
	}

	JSONValueRef JSONFileRef::operator[](const std::filesystem::path& path)
	{
		return JSONValueRef(m_Document, path);
	}
}