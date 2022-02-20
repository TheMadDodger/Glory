#pragma once
#include <vector>
#include <any>
#include <map>

namespace Glory
{
	class AnyConverter
	{
	public:
		//static void Initialize();
		//static void Cleanup();

		static void Convert(std::any& data, std::vector<char>& out);

	protected:
		AnyConverter(size_t typeHash);
		virtual ~AnyConverter();

		virtual void OnConvert(std::any& data, std::vector<char>& out) = 0;

	private:
		size_t m_TypeHash;

		static std::map<size_t, AnyConverter*> m_Converters;
	};

	template<typename T>
	class BasicTemplatedConverter : public AnyConverter
	{
	public:
		BasicTemplatedConverter()
			: AnyConverter(ResourceType::GetHash<T>()) {}
		
		virtual ~BasicTemplatedConverter() {}

	private:
		virtual void OnConvert(std::any& data, std::vector<char>& out) override
		{
			T value = std::any_cast<T>(data);
			size_t size = sizeof(T);
			size_t offset = out.size();
			out.resize(offset + size);
			memcpy((void*)&out[offset], (const void*)&value, size);
		}
	};
}
