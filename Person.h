#ifndef PERSON_H
#define PERSON_H

#include <string>
using namespace std;

class Person {
protected:
    string name;
    string email;

public:
    Person();
    Person(string name, string email);

    string getName() const;
    string getEmail() const;
     virtual void displayProfile() const; 
};

#endif
