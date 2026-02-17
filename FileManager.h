#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Candidate.h"
#include "Job.h"
#include <string>

class FileManager {
public:
    static Candidate parseUnstructuredResume(const std::string& filename);
    static Job loadJobFromFile(const std::string& filename);
};

#endif