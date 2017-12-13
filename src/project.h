#ifndef PROJECT_H
#define PROJECT_H


#include <string>
#include <vector>

class ProjectFile {
public:
    int stackSize;
    int extraMemory;
    std::vector<std::string> sourceFiles;
    std::string outputFile;
};

ProjectFile* load_project(const char *project_file);


#endif
