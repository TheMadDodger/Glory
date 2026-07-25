#pragma once
#include <CommonMacros.h>

#include <imgui_te_context.h>

#include <string_view>
#include <filesystem>
#include <array>

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
X(mouseMove);\
X(validatePopupStack);\
X(validateSceneManager);\

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

#undef X;
#undef BUILT_IN_TESTS;

#define GLORY_YAMLTEST_CHECK_NODE_MSG(expr, node, file, msg)\
do\
{\
	const bool res = (bool)(expr);\
	auto mark = node.Mark();\
	const auto formatMsg = std::format msg;\
	ImGuiTestEngine_Check(file.string().c_str(), "", mark.line, ImGuiTestCheckFlags_None, res, formatMsg.c_str());\
	if (!res) return false;\
} while (false);

#define GLORY_YAMLTEST_CHECK_NODE_DEFINED(parentName, name, parent, node, file)\
GLORY_YAMLTEST_CHECK_NODE_MSG(node.IsDefined(), node, file,\
(parentName " : " name " defined == {}", node.IsDefined() ? "true" : "false"))

#define GLORY_YAMLTEST_CHECK_NODE_TYPE(parentName, name, node, type, file)\
GLORY_YAMLTEST_CHECK_NODE_MSG(node.Type() == YAML::NodeType::type, node, file,\
(parentName " : " name " type {} == {}", NodeTypeToString[node.Type()], NodeTypeToString[YAML::NodeType::type]))