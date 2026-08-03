#pragma once
#include "test_engine_editor_visibility.h"

#include <CommonMacros.h>

#include <imgui_te_context.h>

#include <string_view>
#include <filesystem>
#include <array>
#include <memory>
#include <functional>
#include <unordered_map>

#include <Hash.h>

#include <yaml-cpp/node/node.h>

#define TESTOP_ARGS const std::filesystem::path& path, YAML::Node& operation, ImGuiTestContext* ctx

#define TESTOP_NAME_VAR_NAME(x) TESTOP_##x##_Name
#define TESTOP_FUNC_NAME(x) TESTOP_##x##_Func

#define TESTOP_NAME(x)\
constexpr std::string_view TESTOP_NAME_VAR_NAME(x) = STRINGIFY(x)

#define TESTOP_IMPLEMENTATION_FUNC(x)\
bool TESTOP_FUNC_NAME(x)(TESTOP_ARGS)

#define TESTOP_IMPLEMENTATION(x)\
TESTOP_NAME(x);\
TESTOP_IMPLEMENTATION_FUNC(x)

#define TESTOP_IMPLEMENTATION_BODY(x)\
bool TESTOP_##x##_Func(TESTOP_ARGS)

#define BUILT_IN_TESTOPS \
X(setRef);\
X(itemClick);\
X(itemOpen);\
X(itemInputValue);\
X(mouseMove);\
X(validatePopupStack);\
X(validateSceneManager);\
X(repeat);\

namespace YAML
{
	class Node;
}

#define X(x) TESTOP_IMPLEMENTATION(x)

namespace Glory::Editor
{
	inline const std::array<std::string_view, 5> NodeTypeToString = {
		"Undefined",
		"Null",
		"Scalar",
		"Sequence",
		"Map",
	};

	BUILT_IN_TESTOPS
}

#undef X
#undef BUILT_IN_TESTS

#define GLORY_YAMLTEST_CHECK_NODE_MSG(expr, node, file, msg)\
do\
{\
	const bool res = (bool)(expr);\
	auto mark = node.Mark();\
	const auto formatMsg = std::format msg;\
	ImGuiTestEngine_Check(file.string().c_str(), "", mark.line, ImGuiTestCheckFlags_None, res, formatMsg.c_str());\
	if (!res) return false;\
} while (false);

#define GLORY_YAMLTEST_CHECK_NODE_MSG_RET(expr, node, file, msg, ret)\
do\
{\
	const bool res = (bool)(expr);\
	auto mark = node.Mark();\
	const auto formatMsg = std::format msg;\
	ImGuiTestEngine_Check(file.string().c_str(), "", mark.line, ImGuiTestCheckFlags_None, res, formatMsg.c_str());\
	if (!res) return ret;\
} while (false);

#define GLORY_YAMLTEST_CHECK_NODE_DEFINED(parentName, name, parent, node, file)\
GLORY_YAMLTEST_CHECK_NODE_MSG(node.IsDefined(), node, file,\
(parentName " : " name " defined == {}", node.IsDefined() ? "true" : "false"))

#define GLORY_YAMLTEST_CHECK_NODE_TYPE(parentName, name, node, type, file)\
GLORY_YAMLTEST_CHECK_NODE_MSG(node.Type() == YAML::NodeType::type, node, file,\
("{} : {} type {} == {}", parentName, name, NodeTypeToString[node.Type()], NodeTypeToString[YAML::NodeType::type]))

namespace Glory::Utils::Reflect
{
	struct FieldData;
}

namespace Glory::Editor::YAMLTest
{
	using TestOperation = std::function<bool(const std::filesystem::path&, YAML::Node&, ImGuiTestContext*)>;

	class BaseComparator
	{
	public:
		virtual uint32_t Type() const = 0;

		virtual bool Compare(const std::filesystem::path& path, void* data,
			YAML::Node& value, std::string_view name) const = 0;
	};

	template<typename T>
	class TemplatedComparator : public BaseComparator
	{
	public:
		virtual uint32_t Type() const override
		{
			return Hashing::Hash(typeid(T).name());
		}

		virtual bool Compare(const std::filesystem::path& path, void* data,
			YAML::Node& value, std::string_view name) const override
		{
			const T& a = *reinterpret_cast<T*>(data);
			const T b = value.as<T>();
			const bool comp = a == b;

			GLORY_YAMLTEST_CHECK_NODE_MSG(comp, value, path, ("Property {}: value({}) == reference({})", name, a, b));
			return true;
		}
	};

	struct Comparators
	{
	public:
		Comparators();

		static bool Compare(const std::filesystem::path& path, void* data,
			YAML::Node& value, const Utils::Reflect::FieldData* pField);

	private:
		std::vector<std::unique_ptr<BaseComparator>> m_Comparators;
	};

	GLORY_TESTENGINE_EXTENSION_API void RegisterTestOperation(std::string_view name, TestOperation testOp);
	GLORY_TESTENGINE_EXTENSION_API void RunYAMLTest(const std::filesystem::path& path, YAML::Node& root, ImGuiTestContext* ctx);
	GLORY_TESTENGINE_EXTENSION_API bool RunYAMLTestOperations(const std::filesystem::path& path, YAML::Node& operations, ImGuiTestContext* ctx);
}
