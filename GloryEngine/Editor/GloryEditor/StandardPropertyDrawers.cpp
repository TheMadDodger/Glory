#include "PropertyDrawer.h"
#include <glm/glm.hpp>
#include <LayerMask.h>
#include <LayerRef.h>
#include "Undo.h"
#include "EditorUI.h"
#include <imgui.h>
#include <PropertyFlags.h>
#include <glm/gtc/quaternion.hpp>
#include <LayerManager.h>

namespace Glory::Editor
{
	template<typename PropertyType>
	inline bool PropertyDrawerTemplate<PropertyType>::OnGUI(const std::string& label, PropertyType* data, uint32_t flags) const
	{
		throw new std::exception("Missing property drawer")
	}

	template<>
	bool PropertyDrawerTemplate<float>::OnGUI(const std::string& label, float* data, uint32_t flags) const
	{
        float oldValue = *data;
        return EditorUI::InputFloat(EditorUI::MakeCleanName(label), data);
	}

	template<>
	bool PropertyDrawerTemplate<int>::OnGUI(const std::string& label, int* data, uint32_t flags) const
	{
		return EditorUI::InputInt(EditorUI::MakeCleanName(label), data);
	}

	template<>
	bool PropertyDrawerTemplate<uint32_t>::OnGUI(const std::string& label, uint32_t* data, uint32_t flags) const
	{
		return EditorUI::InputUInt(EditorUI::MakeCleanName(label), data);
	}

	template<>
	bool PropertyDrawerTemplate<bool>::OnGUI(const std::string& label, bool* data, uint32_t flags) const
	{
		return EditorUI::CheckBox(EditorUI::MakeCleanName(label), data);
	}

	template<>
	bool PropertyDrawerTemplate<double>::OnGUI(const std::string& label, double* data, uint32_t flags) const
	{
		return EditorUI::InputDouble(EditorUI::MakeCleanName(label), data);
	}

	template<>
	bool PropertyDrawerTemplate<glm::vec2>::OnGUI(const std::string& label, glm::vec2* data, uint32_t flags) const
	{
		return EditorUI::InputFloat2(EditorUI::MakeCleanName(label), data);
	}

	template<>
	bool PropertyDrawerTemplate<glm::vec3>::OnGUI(const std::string& label, glm::vec3* data, uint32_t flags) const
	{
		return EditorUI::InputFloat3(EditorUI::MakeCleanName(label), data);
	}

	template<>
	bool PropertyDrawerTemplate<glm::vec4>::OnGUI(const std::string& label, glm::vec4* data, uint32_t flags) const
	{
		if (flags & Color)
		{
            const bool hdr = flags & HDR;
            return EditorUI::InputColor(EditorUI::MakeCleanName(label), data, hdr);
		}

		return EditorUI::InputFloat4(EditorUI::MakeCleanName(label), data);
	}

	template<>
    bool PropertyDrawerTemplate<glm::quat>::OnGUI(const std::string& label, glm::quat* data, uint32_t flags) const
    {
        glm::vec3 euler = glm::eulerAngles(*data) / 3.141592f * 180.0f;
        if (EditorUI::InputFloat3(EditorUI::MakeCleanName(label), &euler))
        {
            glm::quat q = glm::quat(euler * 3.141592f / 180.0f);
            data->x = q.x;
            data->y = q.y;
            data->z = q.z;
            data->w = q.w;
            return true;
        }
        return false;
    }

	template<>
    bool PropertyDrawerTemplate<LayerMask>::OnGUI(const std::string& label, LayerMask* data, uint32_t flags) const
    {
        return EditorUI::InputLayerMask(EditorUI::MakeCleanName(label), data);
    }

	template<>
    bool PropertyDrawerTemplate<LayerRef>::OnGUI(const std::string& label, LayerRef* data, uint32_t flags) const
    {
		return EditorUI::InputLeyerRef(EditorUI::MakeCleanName(label), data);
    }

	template<>
	bool PropertyDrawerTemplate<std::string>::OnGUI(const std::string& label, std::string* data, uint32_t flags) const
	{
		if(flags & AreaText)
			return EditorUI::InputTextMultiline(EditorUI::MakeCleanName(label), data);

		return EditorUI::InputText(EditorUI::MakeCleanName(label), data);
	}
}