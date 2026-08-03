#include "TestImpementations.h"

#include <EditorApplication.h>
#include <EditorSceneManager.h>
#include <Serializers.h>

#include <YAML_GLM.h>
#include <Reflection.h>

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace Glory::Editor
{
	static std::unordered_map<std::string_view, Glory::Editor::YAMLTest::TestOperation> Operations;
	static std::unordered_map<std::string, std::string> TestVars;

	constexpr std::string_view IndexKey = "index_";

	static bool ValidateChild(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& child, ImGuiTestContext* ctx);
	static bool ValidateChildren(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& children, ImGuiTestContext* ctx);
	static bool ValidateComponents(const std::filesystem::path& path, const Entity& entity, YAML::Node& components, ImGuiTestContext* ctx);
	static bool ValidateComponent(const std::filesystem::path& path, const Entity& entity, YAML::Node& component, size_t index, ImGuiTestContext* ctx);
	static bool ValidateProperty(const std::filesystem::path& path, YAML::Node& value, const Utils::Reflect::FieldData* pField, void* data);

	static std::string FindAndReplaceVarsInString(const std::filesystem::path& path, YAML::Node& node, const std::string& str)
	{
		std::string result = str;

		size_t nextVarStartIndex = result.find("%{");
		while (nextVarStartIndex != std::string::npos)
		{
			const size_t actualStart = nextVarStartIndex + 2;
			const size_t varEndIndex = result.find('}', actualStart);

			if (varEndIndex == std::string::npos)
			{
				GLORY_YAMLTEST_CHECK_NODE_MSG_RET(false, node, path,
					("Expression ({}) is invalid", str), "");
			}

			const size_t count = varEndIndex - actualStart;
			const std::string varName = result.substr(actualStart, count);
			const auto iter = TestVars.find(varName);

			GLORY_YAMLTEST_CHECK_NODE_MSG_RET(iter != TestVars.end(), node, path,
				("Test var {} exists == {}", varName, iter != TestVars.end() ? "true" : "false"), "");

			const size_t replaceCount = 2 + varName.size() + 1;
			result.replace(nextVarStartIndex, replaceCount, iter->second);

			nextVarStartIndex = result.find("%{");
		}

		return result;
	}

	static int ResolveExpression(const std::string_view expression)
	{
		std::vector<std::string_view> tokens;
		Utils::Reflect::Reflect::Tokenize(expression, tokens, ' ');

		int result = 0;

		enum LastOperator
		{
			Init,
			Add,
			Subtract,
			Multiply,
			Divide
		} lastOp = Init;

		for (const auto token : tokens)
		{
			if (token == "+")
			{
				lastOp = Add;
				continue;
			}
			if (token == "-")
			{
				lastOp = Subtract;
				continue;
			}
			if (token == "*")
			{
				lastOp = Multiply;
				continue;
			}
			if (token == "/")
			{
				lastOp = Divide;
				continue;
			}

			int num = 0;
			const auto convertResult = std::from_chars(token.data(), token.data() + token.size(), num);
			if (convertResult.ec == std::errc::invalid_argument)
				num = 0;

			switch (lastOp)
			{
			case Init:
				result = num;
				break;
			case Add:
				result += num;
				break;
			case Subtract:
				result -= num;
				break;
			case Multiply:
				result *= num;
				break;
			case Divide:
				result /= num;
				break;
			}
		}

		return result;
	}

	template<typename T>
	static bool GetTestValue(const std::filesystem::path& path, YAML::Node& node, T& value)
	{
		const std::string str = node.as<std::string>();
		if (str.find("%{") != std::string::npos)
		{
			const std::string expression = FindAndReplaceVarsInString(path, node, str);
			if (expression.empty()) return false;
			value = T(ResolveExpression(expression));
			return true;
		}

		value = node.as<T>();
		return true;
	}

	template<>
	static bool GetTestValue<std::string>(const std::filesystem::path& path, YAML::Node& node, std::string& value)
	{
		const std::string str = node.as<std::string>();
		if (str.find("%{") != std::string::npos)
		{
			value = FindAndReplaceVarsInString(path, node, str);
			if (value.empty()) return false;
			return true;
		}
		value = str;
		return true;
	}

	bool ExecuteOperation(const std::filesystem::path& path, YAML::Node& operation, ImGuiTestContext* ctx)
	{
		auto name = operation["op"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("operation", "op", operation, name, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("operation", "op", name, Scalar, path);

		const std::string nameStr = name.as<std::string>();
		auto opIter = Operations.find(nameStr);
		GLORY_YAMLTEST_CHECK_NODE_MSG(opIter != Operations.end(), name, path, ("operation: {}", nameStr));

		return opIter->second(path, operation, ctx);
	}

	TESTOP_IMPLEMENTATION_BODY(setRef)
	{
		auto refPath = operation["path"];
		auto mode = operation["mode"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("setRef", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("setRef", "path", refPath, Scalar, path);

		std::string pathValue;
		if (!GetTestValue<std::string>(path, refPath, pathValue)) return false;

		if (mode.IsDefined())
		{
			ImGuiContext& uiCtx = *ctx->UiContext;
			GLORY_YAMLTEST_CHECK_NODE_TYPE("setRef", "mode", mode, Scalar, path);
			const std::string modeStr = mode.as<std::string>();
			if (modeStr == "popupStack")
			{
				ImGuiWindow* popup = nullptr;
				if(pathValue == "back")
					popup = uiCtx.OpenPopupStack.back().Window;
				else if(pathValue == "front")
					popup = uiCtx.OpenPopupStack.front().Window;
				else if (pathValue.starts_with(IndexKey))
				{
					const size_t index = std::stoull(pathValue.substr(IndexKey.size()));
					popup = uiCtx.OpenPopupStack[index].Window;
				}
				else
					popup = uiCtx.OpenPopupStack.back().Window;

				ctx->SetRef(popup);
				return true;
			}
		}

		ctx->SetRef(pathValue.c_str());
		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(itemClick)
	{
		auto refPath = operation["path"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("itemClick", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("itemClick", "path", refPath, Scalar, path);

		std::string pathValue;
		if (!GetTestValue<std::string>(path, refPath, pathValue)) return false;

		ImGuiMouseButton imguiButton = ImGuiMouseButton_Left;
		auto button = operation["button"];
		if (button.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("itemClick", "button", button, Scalar, path);
			const std::string buttonStr = button.as<std::string>();
			if (buttonStr == "left")
				imguiButton = ImGuiMouseButton_Left;
			if (buttonStr == "right")
				imguiButton = ImGuiMouseButton_Right;
			if (buttonStr == "middle")
				imguiButton = ImGuiMouseButton_Middle;
		}

		ctx->ItemClick(pathValue.c_str(), imguiButton);
		return true;
	}
	
	TESTOP_IMPLEMENTATION_BODY(itemOpen)
	{
		auto refPath = operation["path"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("itemOpen", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("itemOpen", "path", refPath, Scalar, path);

		std::string pathValue;
		if (!GetTestValue<std::string>(path, refPath, pathValue)) return false;

		ctx->ItemOpen(pathValue.c_str());
		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(itemInputValue)
	{
		auto refPath = operation["path"];
		auto value = operation["value"];
		auto mode = operation["mode"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("itemInputValue", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("itemInputValue", "path", refPath, Scalar, path);

		GLORY_YAMLTEST_CHECK_NODE_DEFINED("itemInputValue", "value", operation, value, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("itemInputValue", "value", value, Scalar, path);

		std::string pathValue;
		if (!GetTestValue<std::string>(path, refPath, pathValue)) return false;
		std::string modeStr = "string";
		std::string finalValue;

		if (mode.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("itemInputValue", "mode", mode, Scalar, path);
			modeStr = mode.as<std::string>();
		}

		if (modeStr == "string")
		{
			std::string valueStr;
			if (!GetTestValue<std::string>(path, value, valueStr)) return false;
			finalValue = valueStr;
		}
		else if (modeStr == "number")
		{
			int valueInt;
			if (!GetTestValue<int>(path, value, valueInt)) return false;
			finalValue = std::to_string(valueInt);
		}

		ctx->ItemInputValue(pathValue.c_str(), finalValue.c_str());
		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(mouseMove)
	{
		auto refPath = operation["path"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("mouseMove", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("mouseMove", "path", refPath, Scalar, path);
		std::string pathValue;
		if (!GetTestValue<std::string>(path, refPath, pathValue)) return false;

		ctx->MouseMove(pathValue.c_str());
		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(validatePopupStack)
	{
		auto sizeGreaterThan = operation["sizeGreaterThan"];
		auto sizeLessThan = operation["sizeLessThan"];
		auto sizeEquals = operation["sizeEquals"];

		if (sizeGreaterThan.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validatePopupStack", "sizeGreaterThan", sizeGreaterThan, Scalar, path);
			GLORY_YAMLTEST_CHECK_NODE_MSG(sizeGreaterThan.IsScalar(), sizeGreaterThan, path,
				("sizeGreaterThan in validatePopupStack node type == scalar."));
			ImGuiContext& uiCtx = *ctx->UiContext;
			const size_t ref = sizeGreaterThan.as<size_t>();
			GLORY_YAMLTEST_CHECK_NODE_MSG(uiCtx.OpenPopupStack.Size > ref, operation, path,
				("Popup stack size({}) > ref({})", uiCtx.OpenPopupStack.Size, ref));
		}
		if (sizeLessThan.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validatePopupStack", "sizeLessThan", sizeLessThan, Scalar, path);
			ImGuiContext& uiCtx = *ctx->UiContext;
			const size_t ref = sizeGreaterThan.as<size_t>();
			GLORY_YAMLTEST_CHECK_NODE_MSG(uiCtx.OpenPopupStack.Size < ref, operation, path,
				("Popup stack size({}) < ref({})", uiCtx.OpenPopupStack.Size, ref));
		}
		if (sizeEquals.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validatePopupStack", "sizeEquals", sizeEquals, Scalar, path);
			ImGuiContext& uiCtx = *ctx->UiContext;
			const size_t ref = sizeGreaterThan.as<size_t>();
			GLORY_YAMLTEST_CHECK_NODE_MSG(uiCtx.OpenPopupStack.Size == ref, operation, path,
				("Popup stack size({}) == ref({})", uiCtx.OpenPopupStack.Size, ref));
		}

		return true;
	}

	static bool ValidateScene(EditorSceneManager& sceneManager, const std::filesystem::path& path, YAML::Node& scene, ImGuiTestContext* ctx)
	{
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene", "scene", scene, Map, path);

		auto index = scene["index"];
		auto name = scene["name"];
		auto childCount = scene["childCount"];
		auto isActive = scene["isActive"];
		auto children = scene["children"];
		GLORY_YAMLTEST_CHECK_NODE_MSG(index.IsDefined() || name.IsDefined(), scene, path,
			("validateSceneManager:scene index defined == {} || name defined == {}",
				index.IsDefined() ? "true" : "false", name.IsDefined() ? "true" : "false"));

		GScene* pScene = nullptr;
		if (index.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene", "index", index, Scalar, path);
			pScene = sceneManager.GetOpenScene(index.as<size_t>());
		}
		if (!pScene) return false;

		TestVars["scene.uuid"] = std::to_string(pScene->GetUUID());

		if (name.IsDefined())
		{
			const std::string nameStr = name.as<std::string>();
			if (index.IsDefined())
			{
				GLORY_YAMLTEST_CHECK_NODE_MSG(pScene && pScene->Name() == nameStr, scene, path,
					("Scene name({}) == ref({})", pScene->Name(), nameStr));
			}
			else
			{
				pScene = sceneManager.GetOpenScene(nameStr);
				GLORY_YAMLTEST_CHECK_NODE_MSG(pScene, scene, path,
					("Scene with name {} exists == {}", nameStr, pScene ? "true" : "false"));
			}
		}
		if (childCount.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene", "childCount", childCount, Scalar, path);

			size_t countRef = 0;
			if (!GetTestValue<size_t>(path, childCount, countRef)) return false;

			GLORY_YAMLTEST_CHECK_NODE_MSG(pScene->ChildCount(0) == countRef, scene, path,
				("Scene child count({}) == ref({})", pScene->ChildCount(0), countRef));
		}
		if (isActive.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene", "isActive", isActive, Scalar, path);

			const bool ref = isActive.as<bool>();
			const bool isActiveValue = pScene == sceneManager.GetActiveScene();

			GLORY_YAMLTEST_CHECK_NODE_MSG(ref == isActiveValue, scene, path,
				("Scene is active ({}) == ref({})", isActiveValue ? "true" : "false", ref ? "true" : "false"));
		}
		if (children.IsDefined() && !ValidateChildren(pScene, path, 0, children, ctx))
			return false;

		return true;
	}

	static bool ValidateChildren(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& children, ImGuiTestContext* ctx)
	{
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene", "children", children, Sequence, path);

		for (size_t i = 0; i < children.size(); ++i)
		{
			auto child = children[i];
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:children", "child", child, Map, path);
			if (!ValidateChild(pScene, path, 0, child, ctx))
				return false;
		}

		return true;
	}

	static bool ValidateChild(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& child, ImGuiTestContext* ctx)
	{
		auto index = child["index"];
		auto childCount = child["childCount"];
		auto children = child["children"];
		auto componentCount = child["componentCount"];
		auto components = child["components"];
		auto name = child["name"];

		/* @todo: Make child searchable by name? */
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("validateSceneManager:scene:child", "index", child, index, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child", "index", index, Scalar, path);
		const size_t childIndex = index.as<size_t>();
		const Utils::ECS::EntityID childID = pScene->Child(parent, childIndex);
		const Entity childEntity = pScene->GetEntityByEntityID(childID);

		GLORY_YAMLTEST_CHECK_NODE_MSG(childEntity.IsValid(), child, path,
			("Entity is valid == {}", childEntity.IsValid() ? "true" : "false"));

		TestVars["entity.uuid"] = std::to_string(childEntity.EntityUUID());

		if (name.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child", "name", name, Scalar, path);
			const std::string nameStr = name.as<std::string>();
			GLORY_YAMLTEST_CHECK_NODE_MSG(childEntity.Name() == nameStr, name, path,
				("Entity name({}) == ref({})", childEntity.Name(), nameStr));
		}

		if (childCount.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child", "childCount", childCount, Scalar, path);

			size_t countRef = 0;
			if (!GetTestValue<size_t>(path, childCount, countRef)) return false;

			GLORY_YAMLTEST_CHECK_NODE_MSG(pScene->ChildCount(childID) == countRef, childCount, path,
				("Entity child count({}) == ref({})", pScene->ChildCount(childID), countRef));
		}

		if (componentCount.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child", "componentCount", componentCount, Scalar, path);
			const size_t ref = componentCount.as<size_t>();
			GLORY_YAMLTEST_CHECK_NODE_MSG(childEntity.ComponentCount() == ref, componentCount, path,
				("Entity component count({}) == ref({})", childEntity.ComponentCount(), ref));
		}

		if (components.IsDefined() && !ValidateComponents(path, childEntity, components, ctx))
			return false;

		if (children.IsDefined() && !ValidateChildren(pScene, path, childID, children, ctx))
			return false;

		return true;
	}

	static bool ValidateComponents(const std::filesystem::path& path, const Entity& entity, YAML::Node& components, ImGuiTestContext* ctx)
	{
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child", "components", components, Sequence, path);

		for (size_t i = 0; i < components.size(); ++i)
		{
			auto component = components[i];
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child:component", "component", component, Map, path);
			if (!ValidateComponent(path, entity, component, i, ctx))
				return false;
		}

		return true;
	}

	static bool ValidateComponent(const std::filesystem::path& path, const Entity& entity, YAML::Node& component, size_t index, ImGuiTestContext* ctx)
	{
		auto typeName = component["type"];
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child:component", "type", typeName, Scalar, path);
		const std::string typeNameStr = typeName.as<std::string>();
		const Utils::Reflect::TypeData* pType = Utils::Reflect::Reflect::GetTyeData(typeNameStr);
		GLORY_YAMLTEST_CHECK_NODE_MSG(pType, typeName, path,
			("Type {} exists == {}", typeNameStr, pType ? "true" : "false"));

		const uint32_t actualTypeHash = entity.ComponentType(index);

		TestVars["component.uuid"] = std::to_string(entity.ComponentID(index));

		const Utils::Reflect::TypeData* pActualType = Utils::Reflect::Reflect::GetTyeData(actualTypeHash);
		GLORY_YAMLTEST_CHECK_NODE_MSG(pType == pActualType, typeName, path,
			("Component type at index {} == {}", pActualType->TypeName(), pType->TypeName()));

		auto properties = component["properties"];
		if (properties.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child:component", "properties", properties, Map, path);
			
			void* data = entity.GetRegistry()->GetComponentAddress(entity.GetEntityID(), pType->TypeHash());

			for (auto iter = properties.begin(); iter != properties.end(); ++iter)
			{
				const std::string propName = iter->first.as<std::string>();
				auto prop = iter->second;

				const Utils::Reflect::FieldData* pField = pType->GetFieldData(propName);
				GLORY_YAMLTEST_CHECK_NODE_MSG(pField, prop, path,
					("Property {} exists == {}", propName, pField ? "true" : "false"));

				if (!ValidateProperty(path, prop, pField, pField->GetAddress(data)))
					return false;
			}
		}

		return true;
	}

	static bool ValidateProperty(const std::filesystem::path& path, YAML::Node& value, const Utils::Reflect::FieldData* pField, void* data)
	{
		EditorApplication* pApp = EditorApplication::GetInstance();
		Serializers& serializers = pApp->GetSerializers();

		switch (pField->Type())
		{
			case ST_Value:
			case ST_Basic:
			case ST_String:
			case ST_Object:
			case ST_Asset:
			case ST_Enum:
			case ST_Path:
			{
				return YAMLTest::Comparators::Compare(path, data, value, pField);
			}
			case ST_Struct:
				GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child:component:property", pField->Name(), value, Map, path);
				throw "Not implented";
				break;
			case ST_Array:
				GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child:component:property", pField->Name(), value, Sequence, path);
				throw "Not implented";
				break;
		default:
			break;
		}

		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(validateSceneManager)
	{
		auto count = operation["count"];
		auto scenes = operation["scenes"];
		EditorApplication* pApp = EditorApplication::GetInstance();
		EditorSceneManager& sceneManager = pApp->GetSceneManager();

		if (count.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager", "count", count, Scalar, path);
			
			size_t countRef = 0;
			if (!GetTestValue<size_t>(path, count, countRef)) return false;

			GLORY_YAMLTEST_CHECK_NODE_MSG(sceneManager.OpenScenesCount() == countRef, operation, path,
				("Open scene count({}) == ref({})", sceneManager.OpenScenesCount(), countRef));
		}
		if (scenes.IsDefined())
		{
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager", "scenes", scenes, Sequence, path);
			for (size_t i = 0; i < scenes.size(); ++i)
			{
				auto scene = scenes[i];
				if (!ValidateScene(sceneManager, path, scene, ctx))
					return false;
			}
		}

		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(repeat)
	{
		auto count = operation["count"];
		auto operations = operation["operations"];

		GLORY_YAMLTEST_CHECK_NODE_DEFINED("repeat", "count", operation, count, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("repeat", "count", count, Scalar, path);
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("repeat", "operations", operation, operations, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("repeat", "operations", operations, Sequence, path);

		size_t repeatCount = 0;
		if (!GetTestValue<size_t>(path, count, repeatCount)) return false;

		for (size_t i = 0; i < repeatCount; ++i)
		{
			TestVars["repeat.count"] = std::to_string(repeatCount);
			TestVars["repeat.iteration"] = std::to_string(i);
			if (!Glory::Editor::YAMLTest::RunYAMLTestOperations(path, operations, ctx))
				return false;
		}

		return true;
	}
}

#include <print>

template<>
struct std::formatter<glm::vec2> : std::formatter<float> {
	auto format(const glm::vec2& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<float>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<float>::format(x.y, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::vec3> : std::formatter<float> {
	auto format(const glm::vec3& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<float>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<float>::format(x.y, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<float>::format(x.z, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::vec4> : std::formatter<float> {
	auto format(const glm::vec4& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<float>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<float>::format(x.y, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<float>::format(x.z, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<float>::format(x.w, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::uvec2> : std::formatter<uint32_t> {
	auto format(const glm::uvec2& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<uint32_t>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<uint32_t>::format(x.y, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::uvec3> : std::formatter<uint32_t> {
	auto format(const glm::uvec3& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<uint32_t>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<uint32_t>::format(x.y, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<uint32_t>::format(x.z, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::uvec4> : std::formatter<uint32_t> {
	auto format(const glm::uvec4& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<uint32_t>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<uint32_t>::format(x.y, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<uint32_t>::format(x.z, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<uint32_t>::format(x.w, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::ivec2> : std::formatter<int32_t> {
	auto format(const glm::ivec2& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<int32_t>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<int32_t>::format(x.y, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::ivec3> : std::formatter<int32_t> {
	auto format(const glm::ivec3& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<int32_t>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<int32_t>::format(x.y, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<int32_t>::format(x.z, ctx);
		return std::format_to(out, "]");
	}
};

template<>
struct std::formatter<glm::ivec4> : std::formatter<int32_t> {
	auto format(const glm::ivec4& x, auto& ctx) const {
		auto out = ctx.out();
		out = std::format_to(out, "[");
		out = std::formatter<int32_t>::format(x.x, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<int32_t>::format(x.y, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<int32_t>::format(x.z, ctx);
		out = std::format_to(out, ", ");
		out = std::formatter<int32_t>::format(x.w, ctx);
		return std::format_to(out, "]");
	}
};

namespace Glory::Editor::YAMLTest
{
	class ResourceReferenceComparator : public BaseComparator
	{
	public:
		uint32_t Type() const override { return ST_Asset; };

		bool Compare(const std::filesystem::path& path, void* data,
			YAML::Node& value, std::string_view name) const override
		{
			ResourceReferenceBase* pReference = static_cast<ResourceReferenceBase*>(data);
			return pReference->GetUUID() == value.as<uint64_t>();
		}
	};

	Comparators::Comparators()
	{
		m_Comparators.emplace_back(new TemplatedComparator<int8_t>());
		m_Comparators.emplace_back(new TemplatedComparator<int16_t>());
		m_Comparators.emplace_back(new TemplatedComparator<int32_t>());
		m_Comparators.emplace_back(new TemplatedComparator<int64_t>());
		m_Comparators.emplace_back(new TemplatedComparator<uint8_t>());
		m_Comparators.emplace_back(new TemplatedComparator<uint16_t>());
		m_Comparators.emplace_back(new TemplatedComparator<uint32_t>());
		m_Comparators.emplace_back(new TemplatedComparator<uint64_t>());
		m_Comparators.emplace_back(new TemplatedComparator<bool>());
		m_Comparators.emplace_back(new TemplatedComparator<float>());
		m_Comparators.emplace_back(new TemplatedComparator<double>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::vec2>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::vec3>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::vec4>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::uvec2>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::uvec3>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::uvec4>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::ivec2>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::ivec3>());
		m_Comparators.emplace_back(new TemplatedComparator<glm::ivec4>());
		m_Comparators.emplace_back(new TemplatedComparator<std::string>());
		m_Comparators.emplace_back(new ResourceReferenceComparator());
	}

	bool Comparators::Compare(const std::filesystem::path& path, void* data,
		YAML::Node& value, const Utils::Reflect::FieldData* pField)
	{
		static Comparators comparators;

		auto iter = std::find_if(comparators.m_Comparators.begin(), comparators.m_Comparators.end(), [pField](std::unique_ptr<BaseComparator>& pComp) {
			return pComp->Type() == pField->ArrayElementType();
		});

		if (iter == comparators.m_Comparators.end())
		{
			iter = std::find_if(comparators.m_Comparators.begin(), comparators.m_Comparators.end(), [pField](std::unique_ptr<BaseComparator>& pComp) {
				return pComp->Type() == pField->Type();
			});
		}

		const bool exists = iter != comparators.m_Comparators.end();
		GLORY_YAMLTEST_CHECK_NODE_MSG(exists, value, path,
			("Property {} : Comparator for type {} exists == {}", pField->Name(), pField->TypeName(), exists ? "true" : "false"));

		return (*iter)->Compare(path, data, value, pField->Name());
	}

	void RegisterTestOperation(std::string_view name, TestOperation testOp)
	{
		Operations.emplace(name, testOp);
	}

	void RunYAMLTest(const std::filesystem::path& path, YAML::Node& root, ImGuiTestContext* ctx)
	{
		TestVars.clear();

		auto operations = root["operations"];
		IM_CHECK(operations.IsDefined());

		const std::function<bool()> f = [&]() {
			GLORY_YAMLTEST_CHECK_NODE_DEFINED("root", "operations", operations, root, path);
			GLORY_YAMLTEST_CHECK_NODE_TYPE("root", "operations", operations, Sequence, path);
			return true;
		};

		if (!f()) return;
		Glory::Editor::YAMLTest::RunYAMLTestOperations(path, operations, ctx);
	}

	bool RunYAMLTestOperations(const std::filesystem::path& path, YAML::Node& operations, ImGuiTestContext* ctx)
	{
		for (size_t i = 0; i < operations.size(); ++i)
		{
			auto operation = operations[i];
			if (!ExecuteOperation(path, operation, ctx))
				return false;
		}
		return true;
	}
}
