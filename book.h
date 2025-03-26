#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <vector>
#include "user.h"

class Book
{
public:
    // We'll keep the constructor the same
    Book(const std::string &title, const std::string &author, const std::string &isbn);

    std::string getTitle() const;
    std::string getAuthor() const;
    std::string getISBN() const;

    // For multiple copies
    void setTotalCopies(int total) { totalCopies = total; }
    int getTotalCopies() const { return totalCopies; }

    void setBorrowedCount(int count) { borrowedCount = count; }
    int getBorrowedCount() const { return borrowedCount; }

    // We'll store borrower info in a struct
    struct BorrowerInfo {
        std::string name;
        std::string id;
        std::string year;
        std::string group;
        std::string borrowedDate; // "YYYY-MM-dd HH:mm:ss"
    };

    void setBorrowers(const std::vector<BorrowerInfo> &b) { borrowers = b; }
    const std::vector<BorrowerInfo>& getBorrowers() const { return borrowers; }
    
    // Return one copy from any borrower
    bool returnOneCopy();
    
    // Return a specific copy from a specific borrower
    bool returnOneCopyFrom(const std::string &studentId);

private:
    std::string title;
    std::string author;
    std::string isbn;

    int totalCopies;   // how many copies exist
    int borrowedCount; // how many are borrowed

    std::vector<BorrowerInfo> borrowers; // info for each borrowed copy
};

#endif // BOOK_H 