#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class ShapePropertySerializer : public PropertySerializer
    {
	public:
		ShapePropertySerializer(Serializers* pSerializers);
		virtual ~ShapePropertySerializer();

	private:
		void Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
		void Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node) override;
    };
}
