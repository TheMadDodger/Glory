#include "StandardPropertyDrawers.h"
#include "Undo.h"
#include "EditorUI.h"
#include <imgui.h>
#include <PropertyFlags.h>
#include <glm/gtc/quaternion.hpp>
#include <LayerManager.h>

namespace Glory::Editor
{
	bool FloatDrawer::OnGUI(const std::string& label, float* data, uint32_t flags) const
	{
        float oldValue = *data;
        return EditorUI::InputFloat(EditorUI::MakeCleanName(label), data);
	}

	bool IntDrawer::OnGUI(const std::string& label, int* data, uint32_t flags) const
	{
		return EditorUI::InputInt(EditorUI::MakeCleanName(label), data);
	}

	bool BoolDrawer::OnGUI(const std::string& label, bool* data, uint32_t flags) const
	{
		return EditorUI::CheckBox(EditorUI::MakeCleanName(label), data);
	}

	bool DoubleDrawer::OnGUI(const std::string& label, double* data, uint32_t flags) const
	{
		return EditorUI::InputDouble(EditorUI::MakeCleanName(label), data);
	}

	bool Vector2Drawer::OnGUI(const std::string& label, glm::vec2* data, uint32_t flags) const
	{
		return EditorUI::InputFloat2(EditorUI::MakeCleanName(label), data);
	}

	bool Vector3Drawer::OnGUI(const std::string& label, glm::vec3* data, uint32_t flags) const
	{
		return EditorUI::InputFloat3(EditorUI::MakeCleanName(label), data);
	}

	bool Vector4Drawer::OnGUI(const std::string& label, glm::vec4* data, uint32_t flags) const
	{
		if (flags & Color)
		{
            const bool hdr = flags & HDR;
            return EditorUI::InputColor(EditorUI::MakeCleanName(label), data, hdr);
		}

		return EditorUI::InputFloat4(EditorUI::MakeCleanName(label), data);
	}

    bool QuatDrawer::OnGUI(const std::string& label, glm::quat* data, uint32_t flags) const
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

    bool LayerMaskDrawer::OnGUI(const std::string& label, LayerMask* data, uint32_t flags) const
    {
        return EditorUI::InputLayerMask(EditorUI::MakeCleanName(label), data);
    }

    bool LayerRefDrawer::OnGUI(const std::string& label, LayerRef* data, uint32_t flags) const
    {
		return EditorUI::InputLeyerRef(EditorUI::MakeCleanName(label), data);
    }
}