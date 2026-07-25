#include "TestImpementations.h"

#include <yaml-cpp/yaml.h>
#include <EditorApplication.h>
#include <EditorSceneManager.h>

namespace Glory::Editor
{
	constexpr std::string_view IndexKey = "index_";

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
