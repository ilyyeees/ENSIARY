#ifndef USER_H
#define USER_H

#include <string>

class User
{
public:
    User();
    User(const std::string &name,
         const std::string &id,
         const std::string &dob,
         const std::string &year,
         const std::string &group,
         const std::string &email = "");

    std::string getName() const;
    std::string getId() const;
    std::string getDob() const;
    std::string getYear() const;
    std::string getGroup() const;
    std::string getEmail() const;

private:
    std::string name;
    std::string id;
    std::string dob;
    std::string year;
    std::string group;
    std::string email;
};

#endif // USER_H 