#include "Candidate.h"
#include "Resume.h"
#include <iostream>
using namespace std;

Candidate::Candidate(string name,string email):Person(name,email){}

Resume& Candidate::getResume(){
return resume;
}

const Resume& Candidate::getResume() const{
return resume;
}

void Candidate::setResume(const Resume& r) {
    resume = r;
}

vector<string> Candidate::getSkills() const {
    return resume.getSkills();
}

void Candidate::displayProfile() const {
    cout << "Candidate Name: " << name << endl;
    cout << "Email: " << email << endl;
}
