#include "../GloryEditor/Configure.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <sstream>

constexpr size_t PatchNoteCount = 4;
constexpr char* PatchNoteFiles[PatchNoteCount] = {
    "general.txt",
    "windows.txt",
    "linux.txt",
    "mac.txt",
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "WriteVersion: Could not find patch notes";
        return -1;
    }

    std::filesystem::path outDir = argv[1];
    std::filesystem::path versionFilePath = outDir;
    versionFilePath.append("version.txt");
    std::ofstream versionFile{versionFilePath};
    versionFile << Glory::Editor::GloryEditorVersion;
    versionFile.close();

    std::cout << "Written version file to " << versionFilePath << std::endl;

    std::filesystem::path patchNotesPath = "../../Patchnotes/";
    patchNotesPath.append(Glory::Editor::GloryEditorVersion);

    std::filesystem::path patchNoteDst = outDir;
    patchNoteDst.append("Patchnotes");

    if (!std::filesystem::exists(patchNotesPath))
    {
        std::cerr << "WriteVersion: Could not find patch notes" << std::endl;
        return -1;
    }

    std::filesystem::create_directory(patchNoteDst);

    for (size_t i = 0; i < PatchNoteCount; i++)
    {
        const char* file = PatchNoteFiles[i];
        std::filesystem::path patchNotesFile = patchNotesPath;
        patchNotesFile.append(file);
        if (!std::filesystem::exists(patchNotesFile))
        {
            std::cout << "No " << file << " patchnotes found... Skipping patch note generation" << std::endl;
            continue;
        }
        std::filesystem::path patchFileDst = patchNoteDst;
        patchFileDst.append(file);

        std::ofstream outPatchNote{patchFileDst};
        std::ifstream inPatchNote{patchNotesFile};
        std::stringstream buffer;
        std::string str;
        while (std::getline(inPatchNote, str))
        {
            outPatchNote << str;
            if (!inPatchNote.eof())
                outPatchNote << "\\n";
        }

        outPatchNote.close();
        inPatchNote.close();

        std::cout << "Written " << file << " patchnotes to " << patchFileDst << std::endl;
    }
    return 0;
}