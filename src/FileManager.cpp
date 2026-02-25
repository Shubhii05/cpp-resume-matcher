#include "FileManager.h"
#include "Candidate.h"
#include "Resume.h"
#include "Job.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

// =========================
// Helper: trim
// =========================
static string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");

    if (start == string::npos)
        return "";

    return s.substr(start, end - start + 1);
}

// =========================
// Collapse multiple spaces
// =========================
string collapseSpaces(const string &input)
{
    string result;
    bool prevSpace = false;

    for (char c : input)
    {
        if (isspace(static_cast<unsigned char>(c)))
        {
            if (!prevSpace)
            {
                result += ' ';
                prevSpace = true;
            }
        }
        else
        {
            result += c;
            prevSpace = false;
        }
    }

    return result;
}

// =========================
// Safe lowercase
// =========================
static string toLower(string str)
{
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c)
              {
                  return static_cast<char>(tolower(c));
              });

    return str;
}

// =====================================================
// NAME CLEANER FUNCTION
// =====================================================
static string extractNameFromLine(string line)                                         //have to understard
{
    // remove phone numbers
    line.erase(remove_if(line.begin(), line.end(),
                         [](unsigned char c)
                         { return isdigit(c) || c == '+' || c == '-' || c == '(' || c == ')'; }),
               line.end());

    // remove email if present
    size_t atPos = line.find("@");
    if (atPos != string::npos)
        line = line.substr(0, atPos);

    // remove common keywords
    vector<string> removeWords = {
        "linkedin", "github", "leetcode",
        "portfolio", "instagram", "behance"};

    string lower = toLower(line);

    for (const auto &word : removeWords)
    {
        size_t pos = lower.find(word);
        if (pos != string::npos)
        {
            line = line.substr(0, pos);
            break;
        }
    }

    line = trim(collapseSpaces(line));

    // extract only alphabetic words (max 3 words)
    stringstream ss(line);
    string word;
    vector<string> words;

    while (ss >> word)
    {
        bool valid = true;
        for (char c : word)
        {
            if (!isalpha(static_cast<unsigned char>(c)))
            {
                valid = false;
                break;
            }
        }

        if (valid)
            words.push_back(word);

        if (words.size() == 3)
            break;
    }

    if (words.empty())
        return "";

    string name;
    for (size_t i = 0; i < words.size(); ++i)
    {
        name += words[i];
        if (i != words.size() - 1)
            name += " ";
    }

    return name;
}

// =====================================================
// PARSE RESUME
// =====================================================
Candidate FileManager::parseUnstructuredResume(const std::string &filename)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        cout << "Failed to open resume file\n";
        return Candidate("", "");
    }

    string line;
    string name = "";
    string email = "";
    string phone = "";

    Resume resume;
    set<string> detectedSkills;

    bool inEducation = false;
    bool inProjects = false;
    bool inCertifications = false;

    map<string, vector<string>> skillOntology = {
        {"html", {"html"}},
        {"css", {"css"}},
        {"javascript", {"javascript", "js"}},
        {"bootstrap", {"bootstrap"}},
        {"react", {"react"}},
        {"php", {"php"}},
        {"mysql", {"mysql"}},
        {"machine learning", {"machine learning", "ml"}},
        {"deep learning", {"deep learning"}},
        {"artificial intelligence", {"artificial intelligence", "ai"}},
        {"data science", {"data science"}},
        {"python", {"python"}},
        {"c++", {"c++", "cpp"}},
        {"java", {"java"}},
        {"sql", {"sql"}},
        {"docker", {"docker"}},
        {"linux", {"linux"}},
        {"tensorflow", {"tensorflow"}},
        {"pytorch", {"pytorch"}}};

    int lineNumber = 0;

    while (getline(file, line))
    {
        lineNumber++;

        if (line.empty())
            continue;

        string cleanedLine = collapseSpaces(trim(line));
        string lowerLine = toLower(cleanedLine);

        // =========================
        // NAME DETECTION (FIRST 2 LINES ONLY)
        // =========================
        if (lineNumber <= 2 && name.empty())
        {
            string possibleName = extractNameFromLine(cleanedLine);

            if (!possibleName.empty())
            {
                name = possibleName;
            }
        }

        // =========================
        // EMAIL DETECTION
        // =========================
        size_t atPos = cleanedLine.find('@');
        if (atPos != string::npos)
        {
            size_t start = atPos;
            while (start > 0 &&
                   !isspace(static_cast<unsigned char>(cleanedLine[start - 1])))
            {
                start--;
            }

            size_t end = atPos;
            while (end < cleanedLine.size() &&
                   !isspace(static_cast<unsigned char>(cleanedLine[end])))
            {
                end++;
            }

            email = cleanedLine.substr(start, end - start);
        }

        // =========================
        // PHONE DETECTION
        // =========================
        if (count_if(cleanedLine.begin(), cleanedLine.end(),
                     [](unsigned char c)
                     { return isdigit(c); }) >= 10)
        {
            phone = cleanedLine;
        }

        // =========================
        // SECTION DETECTION
        // =========================
        if (lowerLine.find("education") != string::npos)
        {
            inEducation = true;
            inProjects = false;
            inCertifications = false;
            continue;
        }

        if (lowerLine.find("projects") != string::npos)
        {
            inProjects = true;
            inEducation = false;
            inCertifications = false;
            continue;
        }

        if (lowerLine.find("certification") != string::npos)
        {
            inCertifications = true;
            inEducation = false;
            inProjects = false;
            continue;
        }

        if (inEducation)
            resume.addEducation(cleanedLine);

        if (inProjects)
            resume.addProject(cleanedLine);

        if (inCertifications)
            resume.addCertification(cleanedLine);

        // =========================
        // SKILL DETECTION
        // =========================
        for (const auto &group : skillOntology)
        {
            for (const auto &synonym : group.second)
            {
                if (lowerLine.find(synonym) != string::npos)
                {
                    detectedSkills.insert(group.first);
                    break;
                }
            }
        }
    }

    if (name.empty())
        name = "Name Not Detected";

    Candidate candidate(name, email);

    for (const auto &skill : detectedSkills)
        resume.addSkill(skill);

    resume.setPhoneNumber(phone);
    candidate.setResume(resume);

    return candidate;
}

// =====================================================
// LOAD JOB PROFILE (UNCHANGED)
// =====================================================
Job FileManager::loadJobFromFile(const std::string &filename)
{
    cout << "Job file successfully opened!" << endl;

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

                    job.addRequiredSkills(toLower(skillName), weight);
                    readingMandatory = false;
                }
                catch (...)
                {
                    if (readingMandatory)
                    {
                        string cleanLine = toLower(line);

                        while (!cleanLine.empty() &&
                               !isalpha(static_cast<unsigned char>(cleanLine[0])))
                        {
                            cleanLine.erase(0, 1);
                        }

                        if (!cleanLine.empty())
                            job.addMandatorySkills(cleanLine);
                    }
                }
            }
        }
    }

    return job;
}