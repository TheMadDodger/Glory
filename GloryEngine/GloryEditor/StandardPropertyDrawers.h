#pragma once
#include "PropertyDrawer.h"
#include <glm/glm.hpp>

namespace Glory::Editor
{
	class FloatDrawer : public PropertyDrawerTemplate<float>
	{
	public:
		virtual bool OnGUI(const std::string& label, float* data, uint32_t flags) const override;
	};

	class IntDrawer : public PropertyDrawerTemplate<int>
	{
	public:
		virtual bool OnGUI(const std::string& label, int* data, uint32_t flags) const override;
	};

	class BoolDrawer : public PropertyDrawerTemplate<bool>
	{
	public:
		virtual bool OnGUI(const std::string& label, bool* data, uint32_t flags) const override;
	};

	class DoubleDrawer : public PropertyDrawerTemplate<double>
	{
	public:
		virtual bool OnGUI(const std::string& label, double* data, uint32_t flags) const override;
	};

	class Vector2Drawer : public PropertyDrawerTemplate<glm::vec2>
	{
	public:
		virtual bool OnGUI(const std::string& label, glm::vec2* data, uint32_t flags) const override;
	};
	
	class Vector3Drawer : public PropertyDrawerTemplate<glm::vec3>
	{
	public:
		virtual bool OnGUI(const std::string& label, glm::vec3* data, uint32_t flags) const override;
	};
	
	class Vector4Drawer : public PropertyDrawerTemplate<glm::vec4>
	{
	public:
		virtual bool OnGUI(const std::string& label, glm::vec4* data, uint32_t flags) const override;
	};
}