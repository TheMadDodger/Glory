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

			out << YAML::Key << "Shape";
			out << YAML::Value << YAML::BeginMap;

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(ResourceType::GetHash<T>());
			for (size_t i = 0; i < pTypeData->FieldCount(); ++i)
			{
				const GloryReflect::FieldData* pField = pTypeData->GetFieldData(i);
				PropertySerializer::SerializeProperty(pField, pShape, out);
			}

			out << YAML::EndMap;
		}

		void DeserializeInternal(ShapeProperty* shapeProperty, YAML::Node& object) const override
		{
			shapeProperty->SetShape<T>(T());

			T* pShape = shapeProperty->ShapePointer<T>();

			YAML::Node shape = object["Shape"];

			const GloryReflect::TypeData* pTypeData = GloryReflect::Reflect::GetTyeData(ResourceType::GetHash<T>());
			for (size_t i = 0; i < pTypeData->FieldCount(); ++i)
			{
				const GloryReflect::FieldData* pField = pTypeData->GetFieldData(i);
				PropertySerializer::DeserializeProperty(pField, pShape, shape[pField->Name()]);
			}
		}
	};

#define SHAPE_SERIALIZER(shape) {ShapeType::shape, new ShapeSerializer<shape>()}
	std::map<ShapeType, ShapeSerializers*> ShapeSerializers::SHAPE_SERIALIZERS = {
		SHAPE_SERIALIZER(Sphere),
		SHAPE_SERIALIZER(Box),
	};

	ShapePropertySerializer::~ShapePropertySerializer()
	{
		ShapeSerializers::Cleanup();
	}

	ShapePropertySerializer::ShapePropertySerializer() : PropertySerializer(ResourceType::GetHash<ShapeProperty>()) {}

	void ShapePropertySerializer::Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out)
	{
		size_t offset = pFieldData->Offset();
		void* pAddress = (void*)((char*)(data)+offset);

		ShapeProperty* value = (ShapeProperty*)pAddress;
		std::string shapeType;
		if (!GloryReflect::Enum<ShapeType>().ToString(value->m_ShapeType, shapeType)) return;

		out << YAML::Key << pFieldData->Name();
		out << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "ShapeType";
		out << YAML::Value << shapeType;
		ShapeSerializers::Serialize(value, out);
		out << YAML::EndMap;
	}

	void ShapePropertySerializer::Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object)
	{
		if (!object.IsDefined()) return;
		YAML::Node shapeTypeNode = object["ShapeType"];
		std::string shapeTypeStr = shapeTypeNode.as<std::string>();
		ShapeType shapeTye;
		if (!GloryReflect::Enum<ShapeType>().FromString(shapeTypeStr, shapeTye)) return;

		size_t offset = pFieldData->Offset();
		void* pAddress = (void*)((char*)(data)+offset);

		ShapeProperty* value = (ShapeProperty*)pAddress;
		ShapeSerializers::Deserialize(shapeTye, value, object);
	}
}
