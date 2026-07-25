#include "TestImpementations.h"

#include <yaml-cpp/yaml.h>
#include <EditorApplication.h>
#include <EditorSceneManager.h>

#include <Reflection.h>

namespace Glory::Editor
{
	constexpr std::string_view IndexKey = "index_";

	bool ValidateChild(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& child, ImGuiTestContext* ctx);
	bool ValidateChildren(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& children, ImGuiTestContext* ctx);
	bool ValidateComponents(const std::filesystem::path& path, const Entity& entity, YAML::Node& components, ImGuiTestContext* ctx);
	bool ValidateComponent(const std::filesystem::path& path, const Entity& entity, YAML::Node& component, size_t index, ImGuiTestContext* ctx);

	TESTOP_IMPLEMENTATION_BODY(setRef)
	{
		auto refPath = operation["path"];
		auto mode = operation["mode"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("setRef", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("setRef", "path", refPath, Scalar, path);

		const std::string pathStr = refPath.as<std::string>();

		if (mode.IsDefined())
		{
			ImGuiContext& uiCtx = *ctx->UiContext;
			GLORY_YAMLTEST_CHECK_NODE_TYPE("setRef", "mode", mode, Scalar, path);
			const std::string modeStr = mode.as<std::string>();
			if (modeStr == "popupStack")
			{
				ImGuiWindow* popup = nullptr;
				if(pathStr == "back")
					popup = uiCtx.OpenPopupStack.back().Window;
				else if(pathStr == "front")
					popup = uiCtx.OpenPopupStack.front().Window;
				else if (pathStr.starts_with(IndexKey))
				{
					const size_t index = std::stoull(pathStr.substr(IndexKey.size()));
					popup = uiCtx.OpenPopupStack[index].Window;
				}
				else
					popup = uiCtx.OpenPopupStack.back().Window;

				ctx->SetRef(popup);
				return true;
			}
		}

		ctx->SetRef(pathStr.c_str());
		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(itemClick)
	{
		auto refPath = operation["path"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("itemClick", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("itemClick", "path", refPath, Scalar, path);

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

		ctx->ItemClick(refPath.as<std::string>().c_str(), imguiButton);
		return true;
	}

	TESTOP_IMPLEMENTATION_BODY(mouseMove)
	{
		auto refPath = operation["path"];
		GLORY_YAMLTEST_CHECK_NODE_DEFINED("mouseMove", "path", operation, refPath, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("mouseMove", "path", refPath, Scalar, path);
		ctx->MouseMove(refPath.as<std::string>().c_str());
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

	bool ValidateScene(EditorSceneManager& sceneManager, const std::filesystem::path& path, YAML::Node& scene, ImGuiTestContext* ctx)
	{
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene", "scene", scene, Map, path);

		auto index = scene["index"];
		auto name = scene["name"];
		auto childCount = scene["childCount"];
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
			const size_t ref = childCount.as<size_t>();
			GLORY_YAMLTEST_CHECK_NODE_MSG(pScene->ChildCount(0) == ref, scene, path,
				("Scene child count({}) == ref({})", pScene->ChildCount(0), ref));
		}
		if (children.IsDefined() && !ValidateChildren(pScene, path, 0, children, ctx))
			return false;

		return true;
	}

	bool ValidateChildren(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& children, ImGuiTestContext* ctx)
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

	bool ValidateChild(GScene* pScene, const std::filesystem::path& path, Utils::ECS::EntityID parent, YAML::Node& child, ImGuiTestContext* ctx)
	{
		auto index = child["index"];
		auto childCount = child["childCount"];
		auto children = child["children"];
		auto componentCount = child["componentCount"];
		auto components = child["components"];
		auto name = child["name"];

		GLORY_YAMLTEST_CHECK_NODE_DEFINED("validateSceneManager:scene:child", "index", child, index, path);
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child", "index", index, Scalar, path);
		const size_t childIndex = index.as<size_t>();
		const Utils::ECS::EntityID childID = pScene->Child(parent, childIndex);
		const Entity childEntity = pScene->GetEntityByEntityID(childID);

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
			const size_t ref = childCount.as<size_t>();
			GLORY_YAMLTEST_CHECK_NODE_MSG(pScene->ChildCount(childID) == ref, childCount, path,
				("Entity child count({}) == ref({})", pScene->ChildCount(childID), ref));
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

	bool ValidateComponents(const std::filesystem::path& path, const Entity& entity, YAML::Node& components, ImGuiTestContext* ctx)
	{
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child", "components", components, Sequence, path);

		for (size_t i = 0; i < components.size(); ++i)
		{
			auto component = components[i];
			GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child:component", "component", component, Map, path);
			if (!ValidateComponent(path, entity, component, i, ctx))
				return false;
		}
	}

	bool ValidateComponent(const std::filesystem::path& path, const Entity& entity, YAML::Node& component, size_t index, ImGuiTestContext* ctx)
	{
		auto typeName = component["type"];
		GLORY_YAMLTEST_CHECK_NODE_TYPE("validateSceneManager:scene:child:component", "type", typeName, Scalar, path);
		const std::string typeNameStr = typeName.as<std::string>();
		const Utils::Reflect::TypeData* pType = Utils::Reflect::Reflect::GetTyeData(typeNameStr);
		GLORY_YAMLTEST_CHECK_NODE_MSG(pType, typeName, path,
			("Type {} exists == {}", typeNameStr, pType ? "true" : "false"));

		/* TODO! */
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
			const size_t countRef = count.as<size_t>();
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
}
