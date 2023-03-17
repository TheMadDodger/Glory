#pragma once
#include "PropertySerializer.h"

namespace Glory
{
    class ShapePropertySerializer : public PropertySerializer
    {
	public:
		ShapePropertySerializer();
		virtual ~ShapePropertySerializer();

	private:
		void Serialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Emitter& out) override;
		void Deserialize(const GloryReflect::FieldData* pFieldData, void* data, YAML::Node& object) override;

		void Serialize(const SerializedProperty* serializedProperty, YAML::Emitter& out) override {}
		void Deserialize(const SerializedProperty* serializedProperty, YAML::Node& object) override {}
		void Deserialize(std::any& out, YAML::Node& object) override {}
    };
}
