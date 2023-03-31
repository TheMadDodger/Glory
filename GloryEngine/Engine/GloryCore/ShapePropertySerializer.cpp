#include "ShapePropertySerializer.h"
#include "ShapeProperty.h"

namespace Glory
{
	class ShapeSerializers
	{
	public:
		static void Serialize(ShapeProperty* shapeProperty, YAML::Emitter& out)
		{
			if (SHAPE_SERIALIZERS.find(shapeProperty->m_ShapeType) == SHAPE_SERIALIZERS.end()) return;
			SHAPE_SERIALIZERS.at(shapeProperty->m_ShapeType)->SerializeInternal(shapeProperty, out);
		}

		static void Deserialize(const ShapeType shapeType, ShapeProperty* shapeProperty, YAML::Node& object)
		{
			if (SHAPE_SERIALIZERS.find(shapeType) == SHAPE_SERIALIZERS.end()) return;
			SHAPE_SERIALIZERS.at(shapeType)->DeserializeInternal(shapeProperty, object);
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
		virtual void SerializeInternal(ShapeProperty* shapeProperty, YAML::Emitter& out) const = 0;
		virtual void DeserializeInternal(ShapeProperty* shapeProperty, YAML::Node& object) const = 0;

	private:
		static std::map<ShapeType, ShapeSerializers*> SHAPE_SERIALIZERS;
	};

	template<typename T>
	class ShapeSerializer : public ShapeSerializers
	{
	private:
		void SerializeInternal(ShapeProperty* shapeProperty, YAML::Emitter& out) const override
		{
			T* pShape = shapeProperty->ShapePointer<T>();

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(ResourceType::GetHash<T>());
			PropertySerializer::SerializeProperty("Shape", pTypeData, pShape, out);
		}

		void DeserializeInternal(ShapeProperty* shapeProperty, YAML::Node& object) const override
		{
			shapeProperty->SetShape<T>(T());

			T* pShape = shapeProperty->ShapePointer<T>();

			YAML::Node shape = object["Shape"];
			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(ResourceType::GetHash<T>());
			PropertySerializer::DeserializeProperty(pTypeData, pShape, shape);
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

	ShapePropertySerializer::ShapePropertySerializer() : PropertySerializer(ResourceType::GetHash<ShapeProperty>()) {}

	void ShapePropertySerializer::Serialize(const std::string& name, void* data, uint32_t typeHash, YAML::Emitter& out)
	{
		ShapeProperty* value = (ShapeProperty*)data;
		std::string shapeType;
		if (!GloryReflect::Enum<ShapeType>().ToString(value->m_ShapeType, shapeType)) return;

		if (!name.empty())
		{
			out << YAML::Key << name;
			out << YAML::Value;
		}

		out << YAML::BeginMap;
		out << YAML::Key << "ShapeType";
		out << YAML::Value << shapeType;
		ShapeSerializers::Serialize(value, out);
		out << YAML::EndMap;
	}

	void ShapePropertySerializer::Deserialize(void* data, uint32_t typeHash, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		YAML::Node shapeTypeNode = object["ShapeType"];
		std::string shapeTypeStr = shapeTypeNode.as<std::string>();
		ShapeType shapeTye;
		if (!GloryReflect::Enum<ShapeType>().FromString(shapeTypeStr, shapeTye)) return;

		ShapeProperty* value = (ShapeProperty*)data;
		ShapeSerializers::Deserialize(shapeTye, value, object);
	}
}
