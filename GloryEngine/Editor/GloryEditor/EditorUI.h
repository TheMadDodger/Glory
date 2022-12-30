#pragma once
#include "GloryEditor.h"
#include <string_view>
#include <glm/glm.hpp>
#include <functional>
#include <ResourceType.h>
#include <imgui.h>

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
		static GLORY_EDITOR_API bool InputFloat(std::string_view label, float* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat2(std::string_view label, glm::vec2* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat3(std::string_view label, glm::vec3* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat4(std::string_view label, glm::vec4* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);

		static GLORY_EDITOR_API bool InputInt(std::string_view label, int* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt2(std::string_view label, glm::ivec2* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt3(std::string_view label, glm::ivec3* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt4(std::string_view label, glm::ivec4* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);

		static GLORY_EDITOR_API bool InputDouble(std::string_view label, double* value, const double slowSteps = 0.01f, const double fastSteps = 0.1f);

		static GLORY_EDITOR_API bool CheckBox(std::string_view label, bool* value);

		static GLORY_EDITOR_API bool InputColor(std::string_view label, glm::vec4* value, const bool hdr);

		static GLORY_EDITOR_API bool InputText(std::string_view label, char* value, size_t bufferSize, ImGuiInputTextFlags flags = 0);

		static GLORY_EDITOR_API bool InputDropdown(std::string_view label, const std::vector<std::string_view>& options, size_t* index, std::string_view value);
		static GLORY_EDITOR_API bool InputDropdown(std::string_view label, const std::vector<std::string_view>& options, const std::vector<bool>& selected, std::string_view value);
		static GLORY_EDITOR_API bool InputEnum(std::string_view label, size_t typeHash, size_t* value, std::vector<size_t> excludeValues = std::vector<size_t>());

		template<typename T>
		static bool InputEnum(std::string_view label, T* value, std::vector<size_t> excludeValues = std::vector<size_t>())
		{
			return InputEnum(label, ResourceType::GetHash<T>(), (size_t*)value, excludeValues);
		}

		static GLORY_EDITOR_API bool InputLayerMask(std::string_view label, LayerMask* data);
		static GLORY_EDITOR_API bool InputLeyerRef(std::string_view label, LayerRef* data);

		static GLORY_EDITOR_API bool Header(std::string_view label);

		static GLORY_EDITOR_API void EmptyDropdown(std::string_view label, std::string_view value, std::function<void()> callback, float& start, float& width, const float borderPadding = 0.0f);

		static GLORY_EDITOR_API std::string_view MakeCleanName(std::string_view label);

	private:
		static const size_t BUFFERLENGTH = 256;
		static size_t m_BufferWriteIndex;
		static char m_CleanNameBuffer[BUFFERLENGTH];
	};
}
