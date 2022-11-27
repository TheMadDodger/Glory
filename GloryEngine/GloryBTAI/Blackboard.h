#pragma once
#include <string>
#include <map>
#include <vector>

namespace Glory
{
	class BlackboardValueBase
	{
	public:
		explicit BlackboardValueBase() {}
		virtual ~BlackboardValueBase() {}
	};

	template<typename T>
	class BlackboardValue : public BlackboardValueBase
	{
	public:
		BlackboardValue(T value) : Data(value) {}
		T Data;
	};

	class Blackboard
	{
	public:
		Blackboard() {};
		~Blackboard()
		{
			for (size_t i = 0; i < m_pValues.size(); ++i)
			{
				delete m_pValues[i];
			}
			m_pValues.clear();
			m_NameToIndex.clear();
		}

		template<typename T>
		void Add(T value, const std::string& name)
		{
			auto pValue = new BlackboardValue<T>(value);
			m_NameToIndex[name] = m_pValues.size();
			m_pValues.push_back(pValue);
		}

		template<typename T>
		bool Get(const std::string& name, T& data)
		{
			if (m_NameToIndex.count(name) <= 0)
				return false;

			size_t index = m_NameToIndex[name];
			auto pValue = dynamic_cast<BlackboardValue<T>*>(m_pValues[index]);
			if (pValue)
			{
				data = pValue->Data;
				return true;
			}
			return false;
		}

		template<typename T>
		void Set(T value, const std::string& name)
		{
			if (m_NameToIndex.count(name) <= 0)
				return;

			size_t index = m_NameToIndex[name];
			auto pValue = dynamic_cast<BlackboardValue<T>*>(m_pValues[index]);
			if (pValue)
			{
				pValue->Data = value;
			}
		}

		const void* Data() { return (const void*)m_pValues.data(); }

	private:
		std::map<std::string, size_t> m_NameToIndex;
		std::vector<BlackboardValueBase*> m_pValues;
	};
}