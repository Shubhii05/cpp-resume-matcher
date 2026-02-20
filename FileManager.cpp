#include "FileManager.h"
#include "Candidate.h"
#include "Resume.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace std;
std::string cleanString(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    return str;
}

// ----------------------
// Helper: convert to lowercase
// ----------------------
static string toLowerCase(const string &input)
{
    string result = input;
    transform(result.begin(), result.end(), result.begin(),
              [](unsigned char c)
              { return tolower(c); });
    return result;
}

// ----------------------
// Helper: trim whitespace
// ----------------------
static string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");

    if (start == string::npos)
        return "";

    return s.substr(start, end - start + 1);
}

// ===============================
// Parse Resume From FILE (OLD WAY)
// ===============================
Candidate FileManager::parseUnstructuredResume(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cout << "Failed to open resume file: " << filename << endl;
        return Candidate("", "");
    }

    string line;
    string name;
    string email;
    string phone;

    set<string> detectedSkills;
    Resume resume;

    string currentSection = "";
    bool nameCaptured = false;

    map<string, vector<string>> skillOntology = {
        {"machine learning", {"machine learning", "ml"}},
        {"deep learning", {"deep learning"}},
        {"artificial intelligence", {"artificial intelligence", "ai"}},
        {"data science", {"data science", "data analysis"}},
        {"statistics", {"statistics"}},
        {"clustering", {"clustering"}},
        {"classification", {"classification"}},
        {"nlp", {"nlp", "natural language processing"}},
        {"computer vision", {"computer vision"}},
        {"python", {"python"}},
        {"c++", {"c++", "cpp"}},
        {"java", {"java"}},
        {"sql", {"sql"}},
        {"pandas", {"pandas"}},
        {"numpy", {"numpy"}},
        {"tensorflow", {"tensorflow"}},
        {"pytorch", {"pytorch"}},
        {"docker", {"docker"}},
        {"linux", {"linux"}}};

    while (getline(file, line))
    {
        if (line.empty())
            continue;

        string lowerLine = toLowerCase(line);

        if (!nameCaptured &&
            lowerLine.find("@") == string::npos &&
            count_if(line.begin(), line.end(), ::isdigit) < 3)
        {
            name = trim(line);
            nameCaptured = true;
        }

        if (line.find("@") != string::npos)
            email = trim(line);

        // Phone detection (strict: must start with + or digit)
        string trimmedLine = trim(line);

        if ((trimmedLine[0] == '+' || isdigit(trimmedLine[0])) &&
            count_if(trimmedLine.begin(), trimmedLine.end(), ::isdigit) >= 10)
        {
            phone = trimmedLine;
        }

        if (lowerLine.find("education") != string::npos)
        {
            currentSection = "education";
            continue;
        }
        if (lowerLine.find("project") != string::npos)
        {
            currentSection = "project";
            continue;
        }
        if (lowerLine.find("certification") != string::npos)
        {
            currentSection = "certification";
            continue;
        }

        if (currentSection == "education")
            resume.addEducation(line);
        else if (currentSection == "project")
            resume.addProject(line);
        else if (currentSection == "certification")
            resume.addCertification(line);

        for (const auto &skillGroup : skillOntology)
        {
            for (const auto &synonym : skillGroup.second)
            {
                if (lowerLine.find(synonym) != string::npos)
                {
                    detectedSkills.insert(skillGroup.first);
                    break;
                }
            }
        }
    }

    Candidate candidate(name, email);

    for (const auto &skill : detectedSkills)
        resume.addSkill(skill);

    resume.setPhoneNumber(phone);
    candidate.setResume(resume);

    return candidate;
}

// ===============================
// Load Job From FILE (FIXED CLEAN)
// ===============================
Job FileManager::loadJobFromFile(const string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cout << "Failed to open job file: " << filename << endl;
        return Job();
    }

    string line;
    string title;
    int minExperience = 0;

    Job job;
    bool readingMandatory = false;

    while (getline(file, line))
    {
        line = trim(line);

        if (line.empty())
            continue;

        if (line.find("Title:") == 0)
        {
            title = trim(line.substr(6));
        }
        else if (line.find("MinExperience:") == 0)
        {
            minExperience = stoi(trim(line.substr(14)));
            job = Job(title, minExperience);
        }
        else if (line.find("Mandatory:") == 0)
        {
            readingMandatory = true;
        }
        else
        {
            size_t lastSpace = line.find_last_of(' ');

            if (lastSpace != string::npos)
            {
                string lastPart = trim(line.substr(lastSpace + 1));

                try
                {
                    int weight = stoi(lastPart);
                    string skillName = trim(line.substr(0, lastSpace));

                    job.addRequiredSkills(toLowerCase(skillName), weight);
                    readingMandatory = false;
                }
                catch (...)
                {
                    if (readingMandatory)
                    {
                        string cleanLine = trim(toLowerCase(line));

                        // remove leading non alphabetic characters
                        while (!cleanLine.empty() && !isalpha(cleanLine[0]))
                            cleanLine.erase(0, 1);

                        if (!cleanLine.empty())
                            job.addMandatorySkills(cleanLine);
                    }
                }
            }
            else if (readingMandatory)
            {
                string cleanLine = trim(toLowerCase(line));

                while (!cleanLine.empty() && !isalpha(cleanLine[0]))
                    cleanLine.erase(0, 1);

                if (!cleanLine.empty())
                    job.addMandatorySkills(cleanLine);
            }
        }
    }

    return job;
}
