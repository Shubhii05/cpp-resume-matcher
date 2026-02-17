#include "FileManager.h"
#include "Candidate.h"
#include "Resume.h"

#include <iostream>
#include <fstream> //what is this
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

// ----------------------
// Helper: convert to lowercase
// ----------------------
static string toLowerCase(const string &input)
{ // repeated
    string result = input;
    transform(result.begin(), result.end(), result.begin(),
              [](unsigned char c)
              { return tolower(c); });
    return result;
}

// ---------------------------
// Parse Unstructured Resume
// ---------------------------
Candidate FileManager::parseUnstructuredResume(const string &filename)
{

    ifstream file(filename);
    string line;

    string name;
    string email;
    string phone;

    set<string> detectedSkills;

    Resume resume;

    string currentSection = "";

        map<string, vector<string>> skillOntology = {

    // ========================
    // MACHINE LEARNING / AI
    // ========================
    {"machine learning", {"machine learning", "ml"}},
    {"deep learning", {"deep learning"}},
    {"artificial intelligence", {"artificial intelligence", "ai"}},
    {"data science", {"data science", "data analysis"}},
    {"statistics", {"statistics", "statistical modeling"}},
    {"clustering", {"clustering"}},
    {"classification", {"classification"}},
    {"nlp", {"nlp", "natural language processing"}},
    {"computer vision", {"computer vision"}},
    {"predictive modeling", {"predictive modeling", "predictive analysis"}},

    // ========================
    // PROGRAMMING LANGUAGES
    // ========================
    {"python", {"python"}},
    {"c++", {"c++", "cpp"}},
    {"java", {"java"}},
    {"javascript", {"javascript", "js"}},
    {"sql", {"sql", "mysql", "postgresql"}},
    {"r", {"r language"}},

    // ========================
    // DATA & ML LIBRARIES
    // ========================
    {"pandas", {"pandas"}},
    {"numpy", {"numpy"}},
    {"matplotlib", {"matplotlib"}},
    {"tensorflow", {"tensorflow"}},
    {"pytorch", {"pytorch"}},
    {"scikit-learn", {"scikit-learn", "sklearn"}},

    // ========================
    // BACKEND DEVELOPMENT
    // ========================
    {"node.js", {"node.js", "nodejs"}},
    {"express.js", {"express.js", "express"}},
    {"spring boot", {"spring boot"}},
    {"django", {"django"}},
    {"flask", {"flask"}},
    {"rest api", {"rest api", "api development"}},
    {"microservices", {"microservices"}},
    {"mongodb", {"mongodb"}},
    {"redis", {"redis"}},
    {"data structures",{"dsa","data structure and algorithms",
    "data structures and algorithm","data structure and algorithm"}},

    // ========================
    // FRONTEND DEVELOPMENT
    // ========================
    {"html", {"html"}},
    {"css", {"css"}},
    {"bootstrap", {"bootstrap"}},
    {"react", {"react"}},
    {"angular", {"angular"}},
    {"vue.js", {"vue.js", "vue"}},
    {"typescript", {"typescript"}},
    {"tailwind", {"tailwind", "tailwind css"}},

    // ========================
    // DEVOPS / TOOLS
    // ========================
    {"docker", {"docker"}},
    {"kubernetes", {"kubernetes"}},
    {"git", {"git"}},
    {"github", {"github"}},
    {"aws", {"aws", "amazon web services"}},
    {"azure", {"azure"}},
    {"linux", {"linux"}}
};

    bool nameCaptured = false;

    while (getline(file, line))
    {

        if (line.empty())
            continue;

        string lowerLine = toLowerCase(line);

        // 1 Capture Name (first non-numeric meaningful line)
        if (!nameCaptured)
        { // i dont understand
            if (lowerLine.find("roll") == string::npos &&
                lowerLine.find("@") == string::npos &&
                count_if(line.begin(), line.end(), ::isdigit) < 3)
            {

                name = line;
                nameCaptured = true;
                continue;
            }
        }

        // 2 Detect Email
        if (line.find("@") != string::npos)
        {
            email = line;
        }

        // 3 Detect Phone (10+ digits)
        if (count_if(line.begin(), line.end(), ::isdigit) >= 10)
        {
            phone = line;
        }

        // Detect Sections
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

        // Store Section Data
        if (currentSection == "education")
        {
            resume.addEducation(line);
        }
        else if (currentSection == "project")
        {
            resume.addProject(line);
        }
        else if (currentSection == "certification")
        {
            resume.addCertification(line);
        }

        // 4 Detect Skills (case-insensitive)
        for (const auto &skillGroup : skillOntology)
        {
            const string &canonicalSkill = skillGroup.first;
            for (const auto &synonym : skillGroup.second)
            {
                if (lowerLine.find(synonym) != string::npos)
                {
                    detectedSkills.insert(canonicalSkill);
                    break;
                }
            }
        }
    }

    Candidate candidate(name, email);

    for (const auto &skill : detectedSkills)
    {
        resume.addSkill(skill);
    }
    candidate.setResume(resume);
    resume.setPhoneNumber(phone);

    return candidate;
}

// ----------------------
// Load Job from File
// ----------------------
Job FileManager::loadJobFromFile(const string &filename)
{

    ifstream file(filename);
    string line;

    string title;
    int minExperience = 0;

    Job job;

    bool readingMandatory = false;

    while (getline(file, line))
    {

        if (line.empty())
            continue;

        if (line.find("Title:") == 0)
        {
            title = line.substr(7);
        }
        else if (line.find("MinExperience:") == 0)
        {
            minExperience = stoi(line.substr(14));
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
                string lastPart = line.substr(lastSpace + 1);

                // If last part is a number → required skill
                if (all_of(lastPart.begin(), lastPart.end(), ::isdigit))
                {
                    string skillName = line.substr(0, lastSpace);
                    int weight = stoi(lastPart);

                    skillName = toLowerCase(skillName);

                    job.addRequiredSkills(skillName, weight);

                    readingMandatory = false; // stop mandatory mode
                }
                else if (readingMandatory)
                {
                    job.addMandatorySkills(toLowerCase(line));
                }
            }
            else if (readingMandatory)
            {
                job.addMandatorySkills(toLowerCase(line));
            }
        }
    }

    return job;
}
