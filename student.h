#ifndef STUDENT_H
#define STUDENT_H

#include "user.h"
#include <vector>
#include <string>

// Class for managing student-specific data and operations
class Student : public User
{
public:
    Student();
    Student(const std::string &name,
            const std::string &id,
            const std::string &dob,
            const std::string &year,
            const std::string &group,
            const std::string &email = "");

    // Borrowed books history
    struct BorrowRecord {
        std::string bookTitle;
        std::string bookAuthor;
        std::string bookIsbn;
        std::string borrowDate;
        std::string returnDate; // Empty if still borrowed
    };

    // Add a borrow record
    void addBorrowRecord(const BorrowRecord &record);
    
    // Get the borrowing history
    const std::vector<BorrowRecord>& getBorrowHistory() const;
    
    // Get all active borrows (not returned yet)
    std::vector<BorrowRecord> getActiveBorrows() const;

private:
    std::vector<BorrowRecord> borrowHistory;
};

#endif // STUDENT_H 