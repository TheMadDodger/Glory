#define IMGUI_DEFINE_MATH_OPERATORS
#include "TestEngineExtension.h"
#include "TestEngineWindow.h"

#include <MenuBar.h>
#include <EditorApplication.h>
#include <MainEditor.h>
#include <EditorSceneManager.h>

#include <Components.h>
#include <TransformManager.h>
#include <MeshRenderManager.h>

#include <imgui_te_engine.h>
#include <imgui_te_context.h>
#include <EditorAssetDatabase.h>

EXTENSION_CPP(TestEngineEditorExtension)

namespace Glory::Editor
{
	static ImGuiTestEngine* TestEngine = nullptr;

	static constexpr const char* Shortcut_Window_TestEngine = "Open Test Engine";

	TestEngineEditorExtension::TestEngineEditorExtension()
	{
	}

	TestEngineEditorExtension::~TestEngineEditorExtension()
	{
		// May block until TestFunc thread/coroutine joins
		ImGuiTestEngine_Stop(TestEngine);

		// We shutdown the Dear ImGui context _before_ the test engine context, so .ini data may be saved.
		//ImGui::DestroyContext();
		ImGuiTestEngine_DestroyContext(TestEngine);
	}
	
	ImGuiTestEngine* TestEngineEditorExtension::GetTestEngine()
	{
		return TestEngine;
	}

	void TestEngineEditorExtension::Initialize()
	{
		// Initialize Test Engine
		TestEngine = ImGuiTestEngine_CreateContext();
		ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(TestEngine);
		test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
		test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;

		// Register Tests
		RegisterTests();

		// Start test engine
		ImGuiTestEngine_Start(TestEngine, ImGui::GetCurrentContext());

		// Optional: use default crash handler. You may use your own crash handler and call ImGuiTestEngine_CrashHandler() from it.
		ImGuiTestEngine_InstallDefaultCrashHandler();

		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();

		MenuBar::AddMenuItem("Window/Test Engine", [&editor]() { editor.GetWindow<TestEngineWindow>(); }, NULL, Shortcut_Window_TestEngine);
	}

	void TestEngineEditorExtension::RegisterTests()
	{
		ImGuiTest* pTest = nullptr;
		pTest = IM_REGISTER_TEST(TestEngine, "windows", "scene_graph");
		pTest->TestFunc = [](ImGuiTestContext* ctx) // Generally provide a Test function which will drive the test.
		{
			EditorApplication* pApp = EditorApplication::GetInstance();
			EditorSceneManager& sceneManager = pApp->GetSceneManager();
			ImGuiContext& uiCtx = *ctx->UiContext;

			/* Create new scene */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("WINDOWTARGET", ImGuiMouseButton_Right);

			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			ImGuiWindow* sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->MouseMove("Create");
			ctx->ItemClick("//Menu_00/New Scene");

			IM_CHECK(sceneManager.OpenScenesCount() == 1);
			GScene* pScene = sceneManager.GetOpenScene(0ull);
			IM_CHECK(pScene);
			IM_CHECK(pScene->ChildCount(0) == 0);

			/* Create empty object in scene */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("WINDOWTARGET", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->MouseMove("Create");
			ctx->ItemClick("//Menu_00/Empty Object");
			IM_CHECK(pScene->ChildCount(0) == 1);
			{
				const Entity emptyEntity = pScene->ChildEntity(0, 0);
				IM_CHECK(emptyEntity.HasComponent<Transform>());
				IM_CHECK(emptyEntity.ComponentCount() == 1);
				IM_CHECK(emptyEntity.ComponentType(0) == Transform::Hash());
			}

			/* Remove scene */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("$$0/##scenenode", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->ItemClick("Remove Scene");
			pScene = nullptr;
			IM_CHECK(sceneManager.OpenScenesCount() == 0);

			/* Create scene by creating new empty object */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("WINDOWTARGET", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->MouseMove("Create");
			ctx->ItemClick("//Menu_00/Empty Object");
			IM_CHECK(sceneManager.OpenScenesCount() == 1);
			pScene = sceneManager.GetOpenScene(0ull);
			IM_CHECK(pScene->ChildCount(0) == 1);
			{
				const Entity emptyEntity = pScene->ChildEntity(0, 0);
				IM_CHECK(emptyEntity.HasComponent<Transform>());
				IM_CHECK(emptyEntity.ComponentCount() == 1);
				IM_CHECK(emptyEntity.ComponentType(0) == Transform::Hash());
			}

			/* Delete the entity */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("$$0/##scenenode/$$0/##entitynode", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->ItemClick("Delete");
			IM_CHECK(pScene->ChildCount(0) == 0);

			/* Create 2 new scenes */
			for (size_t i = 0; i < 2; ++i)
			{
				ctx->SetRef("//Scene Graph##4");
				ctx->ItemClick("WINDOWTARGET", ImGuiMouseButton_Right);
				IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
				sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
				ctx->SetRef(sceneContextMenu);
				ctx->MouseMove("Create");
				ctx->ItemClick("//Menu_00/New Scene");
				IM_CHECK(sceneManager.OpenScenesCount() == 2 + i);
			}
			IM_CHECK(sceneManager.OpenScenesCount() == 3);
			IM_CHECK(sceneManager.GetActiveScene() == pScene);

			/* Change active scene */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("$$1/##scenenode", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->ItemClick("Set As Active Scene");
			IM_CHECK(sceneManager.OpenScenesCount() == 3);
			IM_CHECK(sceneManager.GetActiveScene() != pScene);
			IM_CHECK(sceneManager.GetActiveScene() == sceneManager.GetOpenScene(1ull));

			/* Remove active scene */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("$$1/##scenenode", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->ItemClick("Remove Scene");
			IM_CHECK(sceneManager.OpenScenesCount() == 2);
			IM_CHECK(sceneManager.GetActiveScene() == pScene);
			IM_CHECK(sceneManager.GetActiveScene() != sceneManager.GetOpenScene(1ull));

			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("$$1/##scenenode", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->ItemClick("Remove Scene");
			IM_CHECK(sceneManager.OpenScenesCount() == 1);
			IM_CHECK(sceneManager.GetActiveScene() == pScene);

			/* Create mesh entity */
			ctx->SetRef("//Scene Graph##4");
			ctx->ItemClick("WINDOWTARGET", ImGuiMouseButton_Right);
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(sceneContextMenu);
			ctx->MouseMove("Create");
			ctx->MouseMove("//Menu_00/New Scene");
			ctx->MouseMove("//Menu_00/Entity Object");
			ctx->ItemClick("//Menu_01/\xef\x86\xb2  Mesh");

			IM_CHECK(sceneManager.OpenScenesCount() == 1);
			pScene = sceneManager.GetOpenScene(0ull);
			IM_CHECK(pScene);
			IM_CHECK(pScene->ChildCount(0) == 1);
			const Entity meshEntity = pScene->ChildEntity(0, 0);
			IM_CHECK(meshEntity.IsValid());
			IM_CHECK(meshEntity.HasComponent<Transform>());
			IM_CHECK(meshEntity.HasComponent<MeshRenderer>());
			IM_CHECK(meshEntity.ComponentCount() == 2);
			IM_CHECK(meshEntity.ComponentType(0) == Transform::Hash());
			IM_CHECK(meshEntity.ComponentType(1) == MeshRenderer::Hash());

			ctx->SetRef("//Scene Graph##4");
			ctx->ItemOpen("$$0/##scenenode");
			ctx->ItemClick("$$0/##scenenode/$$0/##entitynode");

			const UUID meshUUID = meshEntity.EntityUUID();
			ctx->SetRef("//Inspector##3");
			ctx->ItemInputValue(std::format("{}/Object/Name/##value", uint64_t(meshUUID)).c_str(), "MyMesh");
			IM_CHECK(meshEntity.Name() == "MyMesh");

			ctx->ItemInputValue(std::format("{}/Components/$$0/Position/##value/$$0", uint64_t(meshUUID)).c_str(), "10");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Position/##value/$$1", uint64_t(meshUUID)).c_str(), "20");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Position/##value/$$2", uint64_t(meshUUID)).c_str(), "30");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Scale/##value/$$0", uint64_t(meshUUID)).c_str(), "2");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Scale/##value/$$1", uint64_t(meshUUID)).c_str(), "3");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Scale/##value/$$2", uint64_t(meshUUID)).c_str(), "4");
			const Transform& transform = meshEntity.GetComponent<Transform>();
			IM_CHECK(transform.Position == glm::vec3(10.0f, 20.0f, 30.0f));
			IM_CHECK(transform.Scale == glm::vec3(2.0f, 3.0f, 4.0f));

			ctx->ItemInputValue(std::format("{}/Components/$$0/Position/##value/$$0", uint64_t(meshUUID)).c_str(), "0");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Position/##value/$$1", uint64_t(meshUUID)).c_str(), "0");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Position/##value/$$2", uint64_t(meshUUID)).c_str(), "0");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Scale/##value/$$0", uint64_t(meshUUID)).c_str(), "1");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Scale/##value/$$1", uint64_t(meshUUID)).c_str(), "1");
			ctx->ItemInputValue(std::format("{}/Components/$$0/Scale/##value/$$2", uint64_t(meshUUID)).c_str(), "1");
			IM_CHECK(transform.Position == glm::vec3(0.0f, 0.0f, 0.0f));
			IM_CHECK(transform.Scale == glm::vec3(1.0f, 1.0f, 1.0f));

			ctx->ItemClick(std::format("{}/Components/$$1/Mesh/##dropdown", uint64_t(meshUUID)).c_str());
			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);

			ImGuiWindow* assetPickerPopup = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(assetPickerPopup);
			ctx->ItemInputValue("##Search", "Cube");
			ctx->MouseMoveToPos(uiCtx.IO.MousePos + ImVec2(0.0f, 50.0f));
			ctx->MouseClick();

			ctx->SetRef("//Inspector##3");
			ctx->ItemClick(std::format("{}/Components/$$1/Material/##dropdown", uint64_t(meshUUID)).c_str());

			IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
			assetPickerPopup = uiCtx.OpenPopupStack.back().Window;
			ctx->SetRef(assetPickerPopup);
			ctx->ItemInputValue("##Search", "Blue");
			ctx->MouseMoveToPos(uiCtx.IO.MousePos + ImVec2(0.0f, 50.0f));
			ctx->MouseClick();

			const UUID cubeMeshID = EditorAssetDatabase::FindAssetUUIDByName("Cube\\Cube Material 0");
			const UUID blueMaterialID = EditorAssetDatabase::FindAssetUUIDByName("Blue");

			const MeshRenderer& meshRenderer = meshEntity.GetComponent<MeshRenderer>();
			IM_CHECK(meshRenderer.m_Mesh.GetUUID() == cubeMeshID);
			IM_CHECK(meshRenderer.m_Material.GetUUID() == blueMaterialID);

			/* Create other non-module entity objects */
			std::array<std::string, 3> items = {
				"//Menu_01/\xef\x80\xbd  Camera",
				"//Menu_01/\xef\x83\xab  Light",
				"//Menu_01/\xef\x80\xb1  Text",
			};

			for (size_t i = 0; i < items.size(); ++i)
			{
				ctx->SetRef("//Scene Graph##4");
				ctx->ItemClick("WINDOWTARGET", ImGuiMouseButton_Right);
				IM_CHECK(uiCtx.OpenPopupStack.Size > 0);
				sceneContextMenu = uiCtx.OpenPopupStack.back().Window;
				ctx->SetRef(sceneContextMenu);
				ctx->MouseMove("Create");
				ctx->MouseMove("//Menu_00/New Scene");
				ctx->MouseMove("//Menu_00/Entity Object");
				ctx->ItemClick(items[i].c_str());
			}

			pScene = sceneManager.GetOpenScene(0ull);
			IM_CHECK(pScene);
			IM_CHECK(pScene->ChildCount(0) == 4);
			{
				IM_CHECK(meshEntity.IsValid());
				IM_CHECK(meshEntity.HasComponent<Transform>());
				IM_CHECK(meshEntity.HasComponent<MeshRenderer>());
				IM_CHECK(meshEntity.ComponentCount() == 2);
				IM_CHECK(meshEntity.ComponentType(0) == Transform::Hash());
				IM_CHECK(meshEntity.ComponentType(1) == MeshRenderer::Hash());

				const Entity cameraEntity = pScene->ChildEntity(0, 1);
				IM_CHECK(cameraEntity.IsValid());
				IM_CHECK(cameraEntity.HasComponent<Transform>());
				IM_CHECK(cameraEntity.HasComponent<CameraComponent>());
				IM_CHECK(cameraEntity.ComponentCount() == 2);
				IM_CHECK(cameraEntity.ComponentType(0) == Transform::Hash());
				IM_CHECK(cameraEntity.ComponentType(1) == CameraComponent::Hash());

				const Entity lightEntity = pScene->ChildEntity(0, 2);
				IM_CHECK(lightEntity.IsValid());
				IM_CHECK(lightEntity.HasComponent<Transform>());
				IM_CHECK(lightEntity.HasComponent<LightComponent>());
				IM_CHECK(lightEntity.ComponentCount() == 2);
				IM_CHECK(lightEntity.ComponentType(0) == Transform::Hash());
				IM_CHECK(lightEntity.ComponentType(1) == LightComponent::Hash());

				const Entity textEntity = pScene->ChildEntity(0, 3);
				IM_CHECK(textEntity.IsValid());
				IM_CHECK(textEntity.HasComponent<Transform>());
				IM_CHECK(textEntity.HasComponent<TextComponent>());
				IM_CHECK(textEntity.ComponentCount() == 2);
				IM_CHECK(textEntity.ComponentType(0) == Transform::Hash());
				IM_CHECK(textEntity.ComponentType(1) == TextComponent::Hash());
			}
		};
	}
}
