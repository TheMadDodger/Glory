#pragma once
#include <string>
#include <vector>

namespace GloryReflect
{
	class EnumType
	{
	public: // Instance methods
		virtual bool FromString(const std::string& str, void* out) = 0;
		virtual bool ToString(const void* value, std::string& out) = 0;
		virtual size_t NumValues() = 0;
		virtual const std::string& GetName(size_t index) = 0;
	};

	template<typename T>
	class Enum : public EnumType
	{
	public:
		bool FromString(const std::string& str, T& out)
		{
			const auto beginIter = std::begin(m_EnumStringValues);
			const auto endIter = std::end(m_EnumStringValues);
			const auto iter = std::find(beginIter, endIter, str);
			if (iter == endIter)
				return false;
			const size_t index = iter - beginIter;
			out = T(index);
			return true;
		}

		bool ToString(const T& value, std::string& out)
		{
			const size_t index = (size_t)value;
			if (index >= m_NumValues)
				return false;
			out = m_EnumStringValues[index];
			return true;
		}

		virtual size_t NumValues() override
		{
			return m_NumValues;
		}

	protected:
		virtual bool FromString(const std::string& str, void* out) override
		{
			T* outValue = (T*)out;
			return FromString(str, *outValue);
		}

		virtual bool ToString(const void* value, std::string& out) override
		{
			const T* inValue = (T*)value;
			return ToString(*inValue, out);
		}

		virtual const std::string& GetName(size_t index) override
		{
			return m_EnumStringValues[index];
		}

	private:
		bool Valid()
		{
			return false;
		}

	private:
		friend class Reflect;
		static const size_t m_NumValues;
		static const std::string m_EnumStringValues[];
	};
}