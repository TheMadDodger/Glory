#pragma once
#include <vector>

namespace GloryReflect
{
	class ArrayTypeBase
	{
	public:
		virtual void Resize(void* pArrayAddress, size_t newSize) const = 0;
		virtual const size_t Size(void* pArrayAddress) const = 0;
		virtual void* ElementAddress(void* pArrayAddress, size_t index) const = 0;

	};

	template<typename T>
	class ArrayType : public ArrayTypeBase
	{
	public:
		virtual void Resize(void* pArrayAddress, size_t newSize) const override
		{
			ResizeInternal((std::vector<T>*)pArrayAddress, newSize);
		}

		virtual const size_t Size(void* pArrayAddress) const override
		{
			return SizeInternal((std::vector<T>*)pArrayAddress);
		}

		virtual void* ElementAddress(void* pArrayAddress, size_t index) const override
		{
			return ElementAddressInternal((std::vector<T>*)pArrayAddress, index);
		}

	private:
		void ResizeInternal(std::vector<T>* pArray, size_t newSize) const
		{
			size_t currentSize = pArray->size();
			if (currentSize > 0)
			{
				T value = pArray->at(currentSize - 1);
				pArray->resize(newSize, value);
				return;
			}

			pArray->resize(newSize);
		}

		const size_t SizeInternal(std::vector<T>* pArray) const
		{
			return pArray->size();
		}

		virtual void* ElementAddressInternal(std::vector<T>* pArray, size_t index) const
		{
			return &(*pArray)[index];
		}
	};
}
