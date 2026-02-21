#include <filesystem>
#include <string>
#include <format>

#ifdef _WIN64
static constexpr std::string_view TestExtension = ".exe";
#endif

int main(int argc, char* argv[])
{
    int errorCount = 0;
    std::filesystem::path testsLocation = "Tests";
    for (const auto& entry : std::filesystem::directory_iterator(testsLocation))
    {
        if (entry.is_directory()) continue;
        if (entry.path().extension().compare(TestExtension) != 0) continue;
        const std::string cmd = std::format("cd \"{}\" && \"{}\"", testsLocation.string(), entry.path().filename().string());
        errorCount += system(cmd.c_str());
    }
    return errorCount;
}