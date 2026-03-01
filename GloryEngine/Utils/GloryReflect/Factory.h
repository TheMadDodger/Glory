#pragma once
#include <string>
#include <typeindex>
#include <functional>

namespace Glory::Utils::Reflect
{
	class FactoryBase
	{
	public:
		FactoryBase(uint32_t typeHash);
		virtual ~FactoryBase();

		virtual void* CreateAsPointer() const = 0;
		virtual void CreateAsTemporary(std::function<void(void*)> callback) const = 0;

	private:
		friend class Reflect;
		const uint32_t m_TypeHash;
	};

	template<typename T>
	class Factory : public FactoryBase
	{
	public:
		Factory() : FactoryBase(Reflect::Hash<T>()) {}
		virtual ~Factory() {}

		virtual void* CreateAsPointer() const override
		{
			return new T();
		}

		virtual void CreateAsTemporary(std::function<void(void*)> callback) const
		{
			T value = T();
			callback(&value);
		}
	};
}
