#include "Job.h"

using namespace std;

Job::Job() {                                    //default constructor
    minExperience = 0;
}

Job::Job(string title, int experience) {         //parameterized constructor
    jobTitle = title;
    minExperience = experience;
}

string Job::getJobTitle() const {                 //const?
    return jobTitle;
}

int Job::getMinExperience() const {
    return minExperience;
}

void Job::addMandatorySkills(const string& skill) {
    mandatorySkills.insert(skill);
}

void Job::addRequiredSkills(const string& skill, int weight) {
    requiredSkills[skill] = weight;
}

set<string> Job::getMandatorySkills() const {
    return mandatorySkills;
}

map<string, int> Job::getRequiredSkills() const {
    return requiredSkills;
}


