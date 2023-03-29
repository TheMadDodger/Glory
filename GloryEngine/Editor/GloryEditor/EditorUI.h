#pragma once
#include "GloryEditor.h"
#include <string_view>
#include <glm/glm.hpp>
#include <functional>
#include <ResourceType.h>
#include <imgui.h>
#include <NodeRef.h>

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
		enum Flag : uint64_t
		{
			NoLabel = 1,
		};

	public:
		static GLORY_EDITOR_API bool InputFloat(std::string_view label, float* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat(YAMLFileRef& file, const std::filesystem::path& path, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat2(std::string_view label, glm::vec2* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat2(YAMLFileRef & file, const std::filesystem::path& path, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat3(std::string_view label, glm::vec3* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat3(YAMLFileRef & file, const std::filesystem::path& path, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat4(std::string_view label, glm::vec4* value, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);
		static GLORY_EDITOR_API bool InputFloat4(YAMLFileRef & file, const std::filesystem::path& path, const float min = -FLT_MAX, const float max = FLT_MAX, const float steps = 0.1f);

		static GLORY_EDITOR_API bool InputInt(std::string_view label, int* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt(YAMLFileRef& file, const std::filesystem::path& path, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt2(std::string_view label, glm::ivec2* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt2(YAMLFileRef& file, const std::filesystem::path& path, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt3(std::string_view label, glm::ivec3* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt3(YAMLFileRef& file, const std::filesystem::path& path, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt4(std::string_view label, glm::ivec4* value, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);
		static GLORY_EDITOR_API bool InputInt4(YAMLFileRef& file, const std::filesystem::path& path, const int min = INT_MIN, const int max = INT_MAX, const int steps = 1);

		static GLORY_EDITOR_API bool InputDouble(std::string_view label, double* value, const double slowSteps = 0.01f, const double fastSteps = 0.1f);
		static GLORY_EDITOR_API bool InputDouble(YAMLFileRef& file, const std::filesystem::path& path, const double slowSteps = 0.01f, const double fastSteps = 0.1f);

		static GLORY_EDITOR_API bool CheckBox(std::string_view label, bool* value);
		static GLORY_EDITOR_API bool CheckBox(YAMLFileRef& file, const std::filesystem::path& path);

		static GLORY_EDITOR_API bool InputColor(std::string_view label, glm::vec4* value, const bool hdr);
		static GLORY_EDITOR_API bool InputColor(YAMLFileRef& file, const std::filesystem::path& path, const bool hdr);

		static GLORY_EDITOR_API bool InputText(std::string_view label, char* value, size_t bufferSize, ImGuiInputTextFlags flags = 0);
		static GLORY_EDITOR_API bool InputText(YAMLFileRef& file, const std::filesystem::path& path, ImGuiInputTextFlags flags = 0);

		static GLORY_EDITOR_API bool InputDropdown(std::string_view label, const std::vector<std::string_view>& options, size_t* index, std::string_view value);
		static GLORY_EDITOR_API bool InputDropdown(std::string_view label, const std::vector<std::string_view>& options, std::vector<bool>& selected, std::string_view value);
		static GLORY_EDITOR_API bool InputEnum(std::string_view label, uint32_t typeHash, uint32_t* value, const std::vector<uint32_t>& excludeValues = std::vector<uint32_t>());
		static GLORY_EDITOR_API bool InputEnum(YAMLFileRef& file, const std::filesystem::path& path, uint32_t typeHash, const std::vector<uint32_t>& excludeValues = std::vector<uint32_t>());

		template<typename T>
		static bool InputEnum(YAMLFileRef& file, const std::filesystem::path& path, const std::vector<uint32_t>& excludeValues = std::vector<uint32_t>())
		{
			return InputEnum(file, path, ResourceType::GetHash<T>(), excludeValues);
		}

		template<typename T>
		static bool InputEnum(std::string_view label, T* value, const std::vector<uint32_t>& excludeValues = std::vector<uint32_t>())
		{
			return InputEnum(label, ResourceType::GetHash<T>(), (uint32_t*)value, excludeValues);
		}

		static GLORY_EDITOR_API bool InputLayerMask(std::string_view label, LayerMask* data);
		static GLORY_EDITOR_API bool InputLeyerRef(std::string_view label, LayerRef* data);

		static GLORY_EDITOR_API bool Header(std::string_view label);

		static GLORY_EDITOR_API void EmptyDropdown(std::string_view label, std::string_view value, std::function<void()> callback, float& start, float& width, const float borderPadding = 0.0f);

		static GLORY_EDITOR_API std::string_view MakeCleanName(std::string_view label);

		static GLORY_EDITOR_API void PushFlag(Flag flag);
		static GLORY_EDITOR_API void PopFlag();
		static GLORY_EDITOR_API bool HasFlag(Flag flag);

	private:
		typedef uint64_t UIFlags;
		static const size_t BUFFERLENGTH = 256;
		static size_t m_BufferWriteIndex;
		static char m_CleanNameBuffer[BUFFERLENGTH];

		static const size_t TEXTSIZE = 1024;
		static char m_TextBuffer[TEXTSIZE];

		static UIFlags m_UIFlags;
		static std::vector<Flag> m_FlagsStack;
	};
}
