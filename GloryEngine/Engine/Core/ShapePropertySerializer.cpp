#include "ShapePropertySerializer.h"
#include "ShapeProperty.h"

namespace Glory
{
	class ShapeSerializers
	{
	public:
		static void Serialize(Serializers* pSerializers, ShapeProperty* shapeProperty, YAML::Emitter& out)
		{
			if (SHAPE_SERIALIZERS.find(shapeProperty->m_ShapeType) == SHAPE_SERIALIZERS.end()) return;
			SHAPE_SERIALIZERS.at(shapeProperty->m_ShapeType)->SerializeInternal(pSerializers, shapeProperty, out);
		}

		static void Deserialize(Serializers* pSerializers, const ShapeType shapeType, ShapeProperty* shapeProperty, YAML::Node& object)
		{
			if (SHAPE_SERIALIZERS.find(shapeType) == SHAPE_SERIALIZERS.end()) return;
			SHAPE_SERIALIZERS.at(shapeType)->DeserializeInternal(pSerializers, shapeProperty, object);
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
		virtual void SerializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, YAML::Emitter& out) const = 0;
		virtual void DeserializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, YAML::Node& object) const = 0;

	private:
		static std::map<ShapeType, ShapeSerializers*> SHAPE_SERIALIZERS;
	};

	template<typename T>
	class ShapeSerializer : public ShapeSerializers
	{
	private:
		void SerializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, YAML::Emitter& out) const override
		{
			T* pShape = shapeProperty->ShapePointer<T>();

			const TypeData* pTypeData = Reflect::GetTyeData(ResourceType::GetHash<T>());
			pSerializers->SerializeProperty("Shape", pTypeData, pShape, out);
		}

		void DeserializeInternal(Serializers* pSerializers, ShapeProperty* shapeProperty, YAML::Node& object) const override
		{
			shapeProperty->SetShape<T>(T());

			T* pShape = shapeProperty->ShapePointer<T>();

			YAML::Node shape = object["Shape"];
			const TypeData* pTypeData = Reflect::GetTyeData(ResourceType::GetHash<T>());
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
		PropertySerializer(pSerializers, ResourceType::GetHash<ShapeProperty>()) {}

	void ShapePropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		ShapeProperty* value = (ShapeProperty*)data;
		std::string shapeType;
		if (!Enum<ShapeType>().ToString(value->m_ShapeType, shapeType)) return;

		if (!name.empty())
		{
			out << YAML::Key << name;
			out << YAML::Value;
		}

		out << YAML::BeginMap;
		out << YAML::Key << "ShapeType";
		out << YAML::Value << shapeType;
		ShapeSerializers::Serialize(m_pSerializers, value, out);
		out << YAML::EndMap;
	}

	void ShapePropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		YAML::Node shapeTypeNode = object["ShapeType"];
		std::string shapeTypeStr = shapeTypeNode.as<std::string>();
		ShapeType shapeTye;
		if (!Enum<ShapeType>().FromString(shapeTypeStr, shapeTye)) return;

		ShapeProperty* value = (ShapeProperty*)data;
		ShapeSerializers::Deserialize(m_pSerializers, shapeTye, value, object);
	}
}
