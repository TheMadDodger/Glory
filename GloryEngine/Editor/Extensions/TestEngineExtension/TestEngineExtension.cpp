#define IMGUI_DEFINE_MATH_OPERATORS
#include "TestEngineExtension.h"
#include "TestEngineWindow.h"

#include "TestImpementations.h"

#include <MenuBar.h>
#include <EditorApplication.h>
#include <MainEditor.h>
#include <EditorSceneManager.h>

#include <Debug.h>
#include <Components.h>
#include <TransformManager.h>
#include <MeshRenderManager.h>
#include <EditorAssetDatabase.h>

#include <imgui_te_engine.h>
#include <imgui_te_context.h>

#include <yaml-cpp/yaml.h>
#include <imgui_te_utils.h>
#include <imgui_te_internal.h>

#include <efsw/efsw.hpp>

EXTENSION_CPP(TestEngineEditorExtension)

#define TEST_CHECK(msg, expr, debug)\
if(!expr)\
	debug.LogError(std::format msg );\
IM_CHECK(expr);

#define X(x) Operations.emplace(TESTOP_NAME_VAR_NAME(x), TESTOP_FUNC_NAME(x))

namespace Glory::Editor
{
	static ImGuiTestEngine* TestEngine = nullptr;
	static std::vector<std::filesystem::path> TestPaths;
	static std::vector<std::string> TestGroups;
	static std::vector<std::string> TestNames;
	static efsw::WatchID TestsWatchID = 0l;
	static bool ShouldRefreshTests = false;

	static std::unordered_map<std::string_view, TestEngineEditorExtension::Operation> Operations;

	static constexpr const char* Shortcut_Window_TestEngine = "Open Test Engine";

	class TestFilesWatcher : public efsw::FileWatchListener
	{
	private:
		virtual void handleFileAction(efsw::WatchID watchid, const std::string& dir,
			const std::string& filename, efsw::Action action,
			std::string oldFilename = "") override
		{
			switch (action)
			{
			case efsw::Action::Add:
			case efsw::Action::Delete:
			case efsw::Action::Moved:
				ShouldRefreshTests = true;
			}
		}
	};

	static TestFilesWatcher Watcher;

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

	void TestEngineEditorExtension::RegisterOperation(std::string&& name, Operation operation)
	{
		
	}

	void TestEngineEditorExtension::Initialize()
	{
		// Initialize Test Engine
		TestEngine = ImGuiTestEngine_CreateContext();
		ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(TestEngine);
		test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
		test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;

		// Start test engine
		ImGuiTestEngine_Start(TestEngine, ImGui::GetCurrentContext());

		// Optional: use default crash handler. You may use your own crash handler and call ImGuiTestEngine_CrashHandler() from it.
		ImGuiTestEngine_InstallDefaultCrashHandler();

		EditorApplication* pApp = EditorApplication::GetInstance();
		MainEditor& editor = pApp->GetMainEditor();

		MenuBar::AddMenuItem("Window/Test Engine", [&editor]() { editor.GetWindow<TestEngineWindow>(); }, NULL, Shortcut_Window_TestEngine);

		m_OnOpenProjectCallback = ProjectSpace::RegisterCallback(ProjectCallback::OnOpen, &TestEngineEditorExtension::RegisterTests);

		BUILT_IN_TESTOPS
	}

	void TestEngineEditorExtension::Update()
	{
		if (!ShouldRefreshTests) return;
		TestEngineEditorExtension::RegisterTests(ProjectSpace::GetOpenProject());
		ShouldRefreshTests = false;
	}

	void TestEngineEditorExtension::RegisterTests(ProjectSpace* pProject)
	{
		ImGuiTestEngine_UnregisterAllTests(TestEngine);
		TestPaths.clear();
		TestGroups.clear();
		TestNames.clear();

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

		std::filesystem::path testsPath = pProject->RootPath();
		testsPath.append("Tests");
		if (!std::filesystem::exists(testsPath))
		{
			std::filesystem::create_directories(testsPath);
		}
		if (!TestsWatchID)
		{
			EditorApplication* pApp = EditorApplication::GetInstance();
			TestsWatchID = pApp->FileWatch().addWatch(testsPath.string(), &Watcher);
		}

		FindTestsRecursive(testsPath, testsPath);
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

	void TestEngineEditorExtension::FindTestsRecursive(const std::filesystem::path& rootPath, const std::filesystem::path& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				FindTestsRecursive(rootPath, entry.path());
				continue;
			}

			const std::filesystem::path file = entry.path();
			if (file.extension().compare(".yaml") != 0) continue;
			const size_t index = TestPaths.size();
			TestPaths.push_back(file);
			std::string name = file.filename().replace_extension("").string();
			std::string group = file.parent_path().lexically_relative(rootPath).string();
			if (group.empty() || group == ".") group = "general";

			TestGroups.push_back(std::move(group));
			TestNames.push_back(std::move(name));
		}

		for (size_t i = 0; i < TestPaths.size(); ++i)
		{
			ImGuiTest* pTest = nullptr;
			pTest = IM_REGISTER_TEST(TestEngine, TestGroups[i].c_str(), TestNames[i].c_str());
			pTest->TestFunc = [](ImGuiTestContext* ctx) {
				EditorApplication* pApp = EditorApplication::GetInstance();
				EditorSceneManager& sceneManager = pApp->GetSceneManager();
				Debug& debug = pApp->GetEngine()->GetDebug();
				ImGuiContext& uiCtx = *ctx->UiContext;
				const size_t testIndex = reinterpret_cast<size_t>(ctx->Test->UserData);
				const std::filesystem::path& path = TestPaths[testIndex];

				TEST_CHECK(("Missing test at path {}", path.string()), std::filesystem::exists(path), debug);

				try
				{
					YAML::Node root = YAML::LoadFile(path.string());
					auto operations = root["operations"];
					IM_CHECK(operations.IsDefined());

					const std::function<bool()> f = [&]() {
						GLORY_YAMLTEST_CHECK_NODE_DEFINED("root", "operations", operations, root, path);
						GLORY_YAMLTEST_CHECK_NODE_TYPE("root", "operations", operations, Sequence, path);
						return true;
					};

					if (!f()) return;

					for (size_t i = 0; i < operations.size(); ++i)
					{
						auto operation = operations[i];
						if (!ExecuteOperation(path, operation, ctx))
							return;
					}
				}
				catch (const YAML::ParserException& e)
				{
					ctx->LogError("Failed to parse test at path %s error: %s", path.string().c_str(), e.msg.c_str());
					IM_CHECK_SILENT(false);
				}
			};
		}
	}
}

#undef X
