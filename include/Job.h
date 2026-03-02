#ifndef JOB_H
#define JOB_H

#include <string>
#include <map>
#include <set>

class Job
{
private:
    int minExperience;
    std::string jobTitle;

    // Mandatory skills (must exist)
    std::set<std::string> mandatorySkills;

    // Required skills with weight
    std::map<std::string, int> requiredSkills;

public:
    // Constructors
    Job();
    Job(const std::string& title, int experience);

    // Getters
    int getMinExperience() const;
    const std::string& getJobTitle() const;

    const std::set<std::string>& getMandatorySkills() const;
    const std::map<std::string, int>& getRequiredSkills() const;

    // Add skills
    void addMandatorySkills(const std::string& skill);
    void addRequiredSkills(const std::string& skill, int weight);
};

#endif