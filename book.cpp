#include "book.h"

Book::Book(const std::string &title,
           const std::string &author,
           const std::string &isbn)
    : title(title),
    author(author),
    isbn(isbn),
    totalCopies(1),
    borrowedCount(0)
{
}

std::string Book::getTitle() const
{
    return title;
}

std::string Book::getAuthor() const
{
    return author;
}

std::string Book::getISBN() const
{
    return isbn;
}

bool Book::returnOneCopy()
{
    // Check if any copies are borrowed
    if (borrowedCount <= 0) {
        return false;  // No copies to return
    }
    
    // Return one copy (just decrement the counter)
    borrowedCount--;
    
    // Remove first borrower in the list
    if (!borrowers.empty()) {
        borrowers.erase(borrowers.begin());
    }
    
    return true;
}

bool Book::returnOneCopyFrom(const std::string &studentId)
{
    // Check if any copies are borrowed
    if (borrowedCount <= 0 || borrowers.empty()) {
        return false;  // No copies to return
    }
    
    // Find borrower with the specified student ID
    for (auto it = borrowers.begin(); it != borrowers.end(); ++it) {
        if (it->id == studentId) {
            // Remove this specific borrower
            borrowers.erase(it);
            borrowedCount--;
            return true;
        }
    }
    
    // Student not found in borrowers list
    return false;
} 