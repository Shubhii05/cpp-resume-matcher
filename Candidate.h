#ifndef CANDIDATE_H
#define CANDIDATE_H

#include "Person.h"
#include "Resume.h"

class Candidate : public Person
{
private:
    Resume resume;
    vector<string> skills;


public:
    Candidate(string name, string email);
    void setResume(const Resume& r);

    Resume &getResume();
    const Resume &getResume() const;
    vector<string> getSkills() const;
    void displayProfile() const override;
};

#endif
