#include "ShapePropertySerializer.h"
#include "ShapeProperty.h"
#include "ResourceType.h"

namespace Glory
{
	class ShapeSerializers
	{
	public:
		static void Serialize(Serializers* pSerializers, ShapeProperty* shapeProperty, Utils::NodeValueRef node)
		{
			if (SHAPE_SERIALIZERS.find(shapeProperty->m_ShapeType) == SHAPE_SERIALIZERS.end()) return;
			SHAPE_SERIALIZERS.at(shapeProperty->m_ShapeType)->SerializeInternal(pSerializers, shapeProperty, node);
		}

		static void Deserialize(Serializers* pSerializers, const ShapeType shapeType, ShapeProperty* shapeProperty, Utils::NodeValueRef node)
		{
			if (SHAPE_SERIALIZERS.find(shapeType) == SHAPE_SERIALIZERS.end()) return;
			SHAPE_SERIALIZERS.at(shapeType)->DeserializeInternal(pSerializers, shapeProperty, node);
		}

		static void Cleanup()
		{
			for (auto drawer : SHAPE_SERIALIZERS)
			{
				delete drawer.second;
			}
			SHAPE_SERIALIZERS.clear();
		}

	protected:
		virtual void SerializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, Utils::NodeValueRef node) const = 0;
		virtual void DeserializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, Utils::NodeValueRef node) const = 0;

	private:
		static std::map<ShapeType, ShapeSerializers*> SHAPE_SERIALIZERS;
	};

	template<typename T>
	class ShapeSerializer : public ShapeSerializers
	{
	private:
		void SerializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, Utils::NodeValueRef node) const override
		{
			T* pShape = shapeProperty->ShapePointer<T>();

			const TypeData* pTypeData = Reflect::GetTyeData(ResourceTypes::GetHash<T>());
			pSerializers->SerializeProperty(pTypeData, pShape, node["Shape"]);
		}

		void DeserializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, Utils::NodeValueRef node) const override
		{
			shapeProperty->SetShape<T>(T());

			T* pShape = shapeProperty->ShapePointer<T>();

			auto shape = node["Shape"];
			const TypeData* pTypeData = Reflect::GetTyeData(ResourceTypes::GetHash<T>());
			pSerializers->DeserializeProperty(pTypeData, pShape, shape);
		}
	};

#define SHAPE_SERIALIZER(shape) {ShapeType::shape, new ShapeSerializer<shape>()}
	std::map<ShapeType, ShapeSerializers*> ShapeSerializers::SHAPE_SERIALIZERS = {
		SHAPE_SERIALIZER(Sphere),
		SHAPE_SERIALIZER(Box),
		SHAPE_SERIALIZER(Cylinder),
		SHAPE_SERIALIZER(Capsule),
		SHAPE_SERIALIZER(TaperedCapsule),
	};

	ShapePropertySerializer::~ShapePropertySerializer()
	{
		ShapeSerializers::Cleanup();
	}

	ShapePropertySerializer::ShapePropertySerializer(Serializers* pSerializers):
		PropertySerializer(pSerializers, ResourceTypes::GetHash<ShapeProperty>()) {}

	void ShapePropertySerializer::Serialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		ShapeProperty* value = (ShapeProperty*)data;
		node.Set(YAML::Node(YAML::NodeType::Map));
		node["ShapeType"].SetEnum(value->m_ShapeType);
		ShapeSerializers::Serialize(m_pSerializers, value, node);
	}

	void ShapePropertySerializer::Deserialize(void* data, uint32_t typeHash, Utils::NodeValueRef node)
	{
		if (!node.Exists() || !node.IsMap()) return;
		auto shapeTypeNode = node["ShapeType"];
		const ShapeType shapeTye = shapeTypeNode.AsEnum<ShapeType>();

		ShapeProperty* value = (ShapeProperty*)data;
		ShapeSerializers::Deserialize(m_pSerializers, shapeTye, value, node);
	}
}
