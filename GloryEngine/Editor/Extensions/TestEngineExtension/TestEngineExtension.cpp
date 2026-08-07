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

#define X(x) Glory::Editor::YAMLTest::RegisterTestOperation(TESTOP_NAME_VAR_NAME(x), TESTOP_FUNC_NAME(x))

namespace Glory::Editor
{
	static ImGuiTestEngine* TestEngine = nullptr;
	static std::vector<std::filesystem::path> TestPaths;
	static std::vector<std::string> TestGroups;
	static std::vector<std::string> TestNames;
	static efsw::WatchID TestsWatchID = 0l;
	static bool ShouldRefreshTests = false;

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

		Utils::Reflect::Reflect::SetReflectInstance(&pApp->GetEngine()->Reflection());

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
					Glory::Editor::YAMLTest::RunYAMLTest(path, root, ctx);
				}
				catch (const YAML::ParserException& e)
				{
					ctx->LogError("Failed to parse test at path %s error: %s", path.string().c_str(), e.msg.c_str());
					IM_CHECK_SILENT(false);
				}
			};
			pTest->UserData = reinterpret_cast<void*>(i);
		}
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
	}
}

#undef X
