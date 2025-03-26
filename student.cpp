#include "student.h"

Student::Student()
    : User()
{
}

Student::Student(const std::string &name,
                const std::string &id,
                const std::string &dob,
                const std::string &year,
                const std::string &group,
                const std::string &email)
    : User(name, id, dob, year, group, email)
{
}

void Student::addBorrowRecord(const BorrowRecord &record)
{
    borrowHistory.push_back(record);
}

const std::vector<Student::BorrowRecord>& Student::getBorrowHistory() const
{
    return borrowHistory;
}

std::vector<Student::BorrowRecord> Student::getActiveBorrows() const
{
    std::vector<BorrowRecord> activeBorrows;
    
    for (const auto &record : borrowHistory) {
        // If returnDate is empty, the book is still borrowed
        if (record.returnDate.empty()) {
            activeBorrows.push_back(record);
        }
    }
    
    return activeBorrows;
} 