#ifndef RESUME_H
#define RESUME_H

#include <vector>
#include <string>
using namespace std;

class Resume
{
private:
    vector<string> skills;
    vector<string> education;
    vector<string> projects;
    vector<string> certifications;
    string phoneNumber;

public:
    void addSkill(const string &skill);
    void addEducation(const string &edu);
    void addProject(const string &project);
    void addCertification(const string &cert);
    void setPhoneNumber(const string &phone);

    vector<string> getSkills() const;
    vector<string> getEducation() const;
    vector<string> getProjects() const;
    vector<string> getCertifications() const;
    string getPhoneNumber() const;
};

#endif