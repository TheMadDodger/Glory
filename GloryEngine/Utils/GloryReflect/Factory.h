#pragma once
#include <any>
#include <string>
#include <typeindex>
#include <functional>

namespace GloryReflect
{
	class FactoryBase
	{
	public:
		FactoryBase(uint32_t typeHash);
		virtual ~FactoryBase();

		virtual std::any CreateAsValue() const = 0;
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

		virtual std::any CreateAsValue() const override
		{
			return T();
		}

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
