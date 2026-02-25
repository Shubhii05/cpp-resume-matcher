#ifndef JOB_H
#define JOB_H

#include <string>
#include <map>
#include <set>

using namespace std;

class Job {
private:
    int minExperience;
    string jobTitle;
    set<string> mandatorySkills;
    map<string, int> requiredSkills;       //why have we made required skills in map -> for weight

public:
    Job();
    Job(string title, int experience);

    int getMinExperience() const;
    string getJobTitle() const;

    void addMandatorySkills(const string& skill);
    void addRequiredSkills(const string& skill, int weight);

    
    set<string> getMandatorySkills() const;
    map<string, int> getRequiredSkills() const;

};

#endif
