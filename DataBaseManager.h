#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QString>
#include <vector>
#include "book.h"
#include "user.h"
#include "student.h"
#include "admin.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    // Open / initialize DB
    bool initializeDatabase();

    // Book management
    bool addBook(const Book &book);
    bool addBook(const std::string &title, const std::string &author, const std::string &isbn, int totalCopies = 1);
    bool updateBook(const Book &book);
    bool borrowOneCopy(const std::string &isbn, const User &user);
    bool returnOneCopy(const std::string &isbn);
    bool returnOneCopy(const std::string &isbn, const std::string &studentId);
    bool removeBook(const std::string &isbn);

    std::vector<Book> getAllBooks() const;

    // Student management
    bool addStudent(const Student &student);
    bool updateStudent(const Student &student);
    bool removeStudent(const std::string &studentId);
    Student getStudent(const std::string &studentId) const;
    std::vector<Student> getAllStudents() const;
    
    // Admin management & authentication
    bool authenticateAdmin(const std::string &adminId, const std::string &password);
    bool addAdmin(const Admin &admin);
    bool updateAdmin(const Admin &admin);
    bool removeAdmin(const std::string &adminId);
    Admin getAdmin(const std::string &adminId) const;
    std::vector<Admin> getAllAdmins() const;
    
    // Borrowing history
    std::vector<Student::BorrowRecord> getStudentBorrowHistory(const std::string &studentId) const;
    void updateBorrowHistory(const std::string &studentId, const std::string &isbn, 
                             const std::string &title, const std::string &author, bool isReturn);
    
    // CSV Import
    bool isStudentTableEmpty() const;
    bool importStudentsFromCSV(const QString &filename);

    // Admin management
    bool logAdminAction(const std::string &adminId, AdminActionType actionType, 
                        const std::string &details);
    std::vector<Admin::ActionLog> getAdminActionLog() const;

private:
    QSqlDatabase m_db;
    bool createTables();

    // Helper to create N copies for a newly added book
    bool createCopiesForBook(int bookId, int totalCopies);
};

#endif // DATABASEMANAGER_H 