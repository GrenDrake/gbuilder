#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

#include "project.h"



std::string readFile(const std::string &file); // defined in main.cpp

ProjectFile* load_project(const char *project_file) {
    std::ifstream inf(project_file);
    if (!inf) {
        std::cerr << "Error opening project file " << project_file << "\n";
        return nullptr;
    }

    ProjectFile *pf = new ProjectFile;
    std::string line;

    // project files
    if (!std::getline(inf, line)) {
        std::cerr << "First line of project file must contain list of source files\n";
        delete pf;
        return nullptr;
    }
    int lastpos = 0;
    int nextpos;
    while (lastpos < line.size() && isspace(line[lastpos])) {
        ++lastpos;
    }
    while ( (nextpos = line.find(" ", lastpos)) != std::string::npos) {
        pf->sourceFiles.push_back(line.substr(lastpos,nextpos-lastpos));
        lastpos = nextpos + 1;
        while (lastpos < line.size() && isspace(line[lastpos])) {
            ++lastpos;
        }
    }
    std::string lastFile = line.substr(lastpos);
    if (!lastFile.empty()) {
        pf->sourceFiles.push_back(lastFile);
    }
    std::cout << "Building:";
    for (auto filename : pf->sourceFiles) {
        std::cout << " \"" << filename << '"';
    }
    std::cout << '\n';

    // output file
    if (!std::getline(inf, line)) {
        std::cerr << "Second line of project file must contain output filename\n";
        delete pf;
        return nullptr;
    }
    pf->outputFile = line;
    std::cout << "Creating: \"" << pf->outputFile << "\"\n";

    return pf;
}