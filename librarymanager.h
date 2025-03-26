#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <vector>
#include <string>
#include "book.h"
#include "user.h"
#include "student.h"
#include "admin.h"
#include "databasemanager.h"

class Library
{
public:
    Library();

    // Book management methods
    void addBook(const std::string &title, const std::string &author, 
                    const std::string &isbn, int totalCopies = 1);
    const std::vector<Book>& getAllBooks() const;
    bool borrowOneCopy(const std::string &isbn, const User &borrower);
    bool returnOneCopy(const std::string &isbn);
    bool returnOneCopy(const std::string &isbn, const std::string &studentId);
    bool removeBook(const std::string &isbn);
    void refreshBookCache();
    
    // Student management methods
    bool addStudent(const Student &student);
    bool updateStudent(const Student &student);
    bool removeStudent(const std::string &studentId);
    Student getStudent(const std::string &studentId) const;
    std::vector<Student> getAllStudents() const;
    
    // CSV Import
    bool initializeFromCSVIfNeeded();
    
    // Admin management methods
    bool authenticateAdmin(const std::string &adminId, const std::string &password);
    bool addAdmin(const Admin &admin);
    bool updateAdmin(const Admin &admin);
    bool removeAdmin(const std::string &adminId);
    Admin getAdmin(const std::string &adminId) const;
    Admin getCurrentAdmin() const;
    bool setCurrentAdmin(const Admin &admin);
    bool logoutCurrentAdmin();
    std::vector<Admin> getAllAdmins() const;
    std::vector<Admin::ActionLog> getAdminActionLog() const;

    // Static validation methods
    static bool isValidStudentData(const std::string &year, const std::string &group,
                                std::string &errorMsg);
    static int getMaxGroupsForYear(const std::string &year);

    // Get students for the book borrowing dropdown
    std::vector<Student> getStudentsForYear(const std::string &year) const;
    std::vector<Student> getStudentsForYearAndGroup(const std::string &year, const std::string &group) const;
    
    // Borrowing history
    std::vector<Student::BorrowRecord> getStudentBorrowHistory(const std::string &studentId) const;

private:
    DatabaseManager m_db;
    std::vector<Book> m_books;
    Admin m_currentAdmin;
    
    // Helper methods
};

#endif // LIBRARYMANAGER_H 