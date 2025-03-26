#include "user.h"

User::User()
    : name(""), id(""), dob(""), year(""), group(""), email("")
{
}

User::User(const std::string &name,
           const std::string &id,
           const std::string &dob,
           const std::string &year,
           const std::string &group,
           const std::string &email)
    : name(name), id(id), dob(dob), year(year), group(group), email(email)
{
}

std::string User::getName() const  { return name; }
std::string User::getId() const    { return id; }
std::string User::getDob() const   { return dob; }
std::string User::getYear() const  { return year; }
std::string User::getGroup() const { return group; }
std::string User::getEmail() const { return email; } 