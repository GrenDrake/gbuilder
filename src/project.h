#ifndef PROJECT_H
#define PROJECT_H


#include <string>
#include <vector>

class ProjectFile {
public:
    ProjectFile()
    : stackSize(2048), extraMemory(0), outputFile("output.ulx")
    { }

    int stackSize;
    int extraMemory;
    std::vector<std::string> sourceFiles;
    std::string outputFile;
};

ProjectFile* load_project(const char *project_file);


#endif
