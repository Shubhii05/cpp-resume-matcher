#include "Person.h"
using namespace std;
#include <iostream>
Person::Person() {}

Person::Person(string name, string email) {
    this->name = name;
    this->email = email;
}

string Person::getName() const {
    return name;
}

string Person::getEmail() const {
    return email;
}
void Person::displayProfile() const {
    cout << "Name: " << name << endl;
    cout << "Email: " << email << endl;
}
