#include <Tester.h>

#include <filesystem>

namespace Glory::Test
{
	constexpr std::string_view TestProjectsPath = "./TestProjects";
	constexpr std::string_view TestProjectsPathFromTestApp = "./Tests/TestProjects";
	constexpr std::string_view TestRunnerName = "GloriousTestApp.exe";

	class GloriousEditorTest : public Utils::Tester
	{
	public:
		GloriousEditorTest();
		virtual ~GloriousEditorTest();

	private:
		void FindTests();

		void RunNextTest();

	private:
		size_t m_CurrentTestIndex = 0;
		std::vector<std::string> m_TestNames;
	};

	GloriousEditorTest::GloriousEditorTest()
	{
		FindTests();
	}

	GloriousEditorTest::~GloriousEditorTest()
	{
	}

	void GloriousEditorTest::FindTests()
	{
		const std::filesystem::path testProjectsPath = TestProjectsPath;

		for (const auto& entry : std::filesystem::directory_iterator(testProjectsPath))
		{
			if (!entry.is_directory()) continue;
			const std::string name = entry.path().lexically_relative(testProjectsPath).string();
			m_TestNames.push_back(name);
			AddTests({ &GloriousEditorTest::RunNextTest });
		}
	}

	void GloriousEditorTest::RunNextTest()
	{
		const std::string_view name = m_TestNames[m_CurrentTestIndex];
		SetFunctionName(name);

		std::filesystem::path testProjectPath = TestProjectsPathFromTestApp;
		testProjectPath.append(name).append(name).replace_extension(".gproj");

		std::stringstream str;
		str << "cd \"..\" && " << TestRunnerName << " -projectPath=\"" << testProjectPath.string() << "\"";
		const int errors = system(str.str().c_str());

		GLORY_TEST_COMPARE(errors, 0);

		++m_CurrentTestIndex;
	}
}

GLORY_TEST_MAIN(Glory::Test::GloriousEditorTest)
