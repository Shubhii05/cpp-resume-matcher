#ifndef PERSON_H               //prevents from including multiple times
#define PERSON_H

#include <string>
using namespace std;

class Person{
protected:
    string name;
    string email;

public:
     Person() {}
     Person(string name,string email){
        this->name=name;
        this->email=email;
     }

     string getName() const{       //const means it cannot be changed be tried.
        return name;
     }

     string getEmail() const {
        return email;
     }
virtual void displayProfile() const = 0;    // virtual enables runtime polymorphism. The function call depends on the actual object, not the pointer type.
};

#endif