#pragma once
#include "Shapes.h"

namespace Glory
{
	struct ShapeProperty
	{
	public:
		ShapeProperty() : m_ShapeType(ShapeType::None), m_ShapeDataBuffer("\0") {}

		template<typename T>
		T* ShapePointer()
		{
			return (T*)m_ShapeDataBuffer;
		}

		template<typename T>
		const T* ShapePointer() const
		{
			return (T*)m_ShapeDataBuffer;
		}

		const Shape* BaseShapePointer()
		{
			return (Shape*)m_ShapeDataBuffer;
		}

		template<typename T>
		void SetShape(const T& data)
		{
			assert(sizeof(T) <= sizeof(m_ShapeDataBuffer));
			m_ShapeType = data.m_ShapeType;
			std::memcpy(m_ShapeDataBuffer, (void*)&data, sizeof(T));
		}

		const char* Data() const
		{
			return m_ShapeDataBuffer;
		}

		bool operator==(const ShapeProperty& other)
		{
			if (other.m_ShapeType != m_ShapeType) return false;
			return std::memcmp(m_ShapeDataBuffer, other.m_ShapeDataBuffer, sizeof(m_ShapeDataBuffer)) == 0;
		}

		REFLECTABLE_WITH_BUFFER(ShapeProperty, m_ShapeDataBuffer, 64,
			(ShapeType)(m_ShapeType)
		);

	private:
		char m_ShapeDataBuffer[64];
	};
}
