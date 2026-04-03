#include "Job.h"
#include <algorithm>

using namespace std;

// ===================================
// Helper: Convert string to lowercase
// ===================================
static string toLowerCase(string str)
{
    transform(str.begin(), str.end(), str.begin(),
              [](unsigned char c) { return tolower(c); });          //unsigned char is a 1-byte integer type that stores values from 0 to 255.
    return str;
}

// ==============================
// Constructors
// ==============================

Job::Job()
{
    minExperience = 0;
}

Job::Job(const string& title, int experience)
{
    jobTitle = title;
    minExperience = experience;
}

// ==============================
// Getters
// ==============================

int Job::getMinExperience() const
{
    return minExperience;
}

const string& Job::getJobTitle() const
{
    return jobTitle;
}

const set<string>& Job::getMandatorySkills() const
{
    return mandatorySkills;
}

const map<string, int>& Job::getRequiredSkills() const
{
    return requiredSkills;
}

// ==============================
// Add Skills (Lowercase Normalization)
// ==============================

void Job::addMandatorySkills(const string& skill)
{
    string lowerSkill = toLowerCase(skill);
    mandatorySkills.insert(lowerSkill);
}

void Job::addRequiredSkills(const string& skill, int weight)
{
    string lowerSkill = toLowerCase(skill);
    requiredSkills[lowerSkill] = weight;
}