#pragma once
#include "Shapes.h"

namespace Glory
{
	struct ShapeProperty
	{
	public:
		ShapeProperty() : m_ShapeDataBuffer("\0") {}

		template<typename T>
		const T& Shape()
		{
			return (const T&)m_ShapeDataBuffer;
		}

		template<typename T>
		void SetShape(const T& data)
		{
			assert(sizeof(T) <= sizeof(m_ShapeDataBuffer));
			m_ShapeType = data.m_ShapeType;
			std::memcpy(m_ShapeDataBuffer, (void*)&data, sizeof(T));
		}

		REFLECTABLE(ShapeProperty,
			(ShapeType)(m_ShapeType)
		);

	private:
		char m_ShapeDataBuffer[64];
	};
}
