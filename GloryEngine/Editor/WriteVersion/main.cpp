#include "../GloryEditor/Configure.h"
#include <fstream>

int main(int argc, char* argv[])
{
    std::ofstream file{"./version.txt"};
    file << Glory::Editor::GloryEditorVersion;
    return 0;
}