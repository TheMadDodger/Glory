#pragma once
#include "GloryEditor.h"
#include <string_view>
#include <glm/glm.hpp>
#include <functional>

namespace Glory
{
	struct LayerMask;
	struct LayerRef;
}

namespace Glory::Editor
{
	class EditorUI
	{
	public:
		static GLORY_EDITOR_API bool InputFloat(const std::string_view& label, float* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat2(const std::string_view& label, glm::vec2* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat3(const std::string_view& label, glm::vec3* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat4(const std::string_view& label, glm::vec4* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);

		static GLORY_EDITOR_API bool InputInt(const std::string_view& label, int* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt2(const std::string_view& label, glm::ivec2* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt3(const std::string_view& label, glm::ivec3* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt4(const std::string_view& label, glm::ivec4* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);

		static GLORY_EDITOR_API bool InputDouble(const std::string_view& label, double* value, const double slowSteps = 0.01f, const double fastSteps = 0.1f);

		static GLORY_EDITOR_API bool CheckBox(const std::string_view& label, bool* value);

		static GLORY_EDITOR_API bool InputColor(const std::string_view& label, glm::vec4* value, const bool hdr);

		static GLORY_EDITOR_API bool InputText(const std::string_view& label, char* value, size_t bufferSize);

		static GLORY_EDITOR_API bool InputDropdown(const std::string_view& label, const std::vector<std::string_view>& options, size_t* index, const std::string_view& value);
		static GLORY_EDITOR_API bool InputDropdown(const std::string_view& label, const std::vector<std::string_view>& options, const std::vector<bool>& selected, const std::string_view& value);

		static GLORY_EDITOR_API bool InputLayerMask(const std::string_view& label, LayerMask* data);
		static GLORY_EDITOR_API bool InputLeyerRef(const std::string_view& label, LayerRef* data);

		static GLORY_EDITOR_API bool Header(const std::string_view& label);

		static GLORY_EDITOR_API void EmptyDropdown(const std::string_view& label, const std::string_view& value, std::function<void()> callback, float& start, float& width, const float borderPadding = 0.0f);
	};
}
