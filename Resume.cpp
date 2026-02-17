#include "Resume.h"
using namespace std;

void Resume::addSkill(const string& skill){
    skills.push_back(skill);
}

void Resume::addEducation(const string& edu){
    education.push_back(edu);
}

void Resume::addProject(const string& project){
projects.push_back(project);
}

void Resume::addCertification(const string& cert){
    certifications.push_back(cert);
}
void Resume::setPhoneNumber(const string& phone) {
    phoneNumber = phone;
}

vector<string> Resume::getSkills() const {
    return skills;
}

vector<string> Resume::getEducation() const {
    return education;
}

vector<string> Resume::getProjects() const {
    return projects;
}

vector<string> Resume::getCertifications() const {
    return certifications;
}

string Resume::getPhoneNumber() const {
    return phoneNumber;
}
