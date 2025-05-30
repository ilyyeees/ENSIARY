#include "librarymanager.h"
#include <QString>

Library::Library()
{
    // Initialize student database from CSV if needed
    initializeFromCSVIfNeeded();
    
    // Load books
    refreshBookCache();
}

void Library::addBook(const std::string &title,
                      const std::string &author,
                      const std::string &isbn,
                      int totalCopies)
{
    // Log the action if an admin is logged in
    if (!m_currentAdmin.getId().empty()) {
        std::string details = "Added book: " + title + " by " + author + " (ISBN: " + isbn + "), " + 
                               std::to_string(totalCopies) + " copies";
        m_currentAdmin.logAction(AdminActionType::ADD_BOOK, details);
    }
    
    Book newBook(title, author, isbn);
    newBook.setTotalCopies(totalCopies);
    m_db.addBook(newBook);
    
    // Refresh book cache after adding
    refreshBookCache();
}

bool Library::borrowOneCopy(const std::string &isbn, const User &user)
{
    // Log the action if an admin is logged in
    if (!m_currentAdmin.getId().empty()) {
        std::string details = "Borrowed book: ISBN " + isbn + " to user " + user.getName() + " (ID: " + user.getId() + ")";
        m_currentAdmin.logAction(AdminActionType::BORROW_BOOK, details);
    }
    
    bool success = m_db.borrowOneCopy(isbn, user);
    
    // If successful, also update the student's borrow history
    if (success) {
        // Get book details
        std::vector<Book> books = m_db.getAllBooks();
        std::string title, author;
        
        for (const auto &book : books) {
            if (book.getISBN() == isbn) {
                title = book.getTitle();
                author = book.getAuthor();
                break;
            }
        }
        
        // Update borrowing history
        m_db.updateBorrowHistory(user.getId(), isbn, title, author, false);
        
        // Refresh book cache after borrowing
        refreshBookCache();
    }
    
    return success;
}

bool Library::returnOneCopy(const std::string &isbn)
{
    // Log the action if an admin is logged in
    if (!m_currentAdmin.getId().empty()) {
        std::string details = "Returned book: ISBN " + isbn;
        m_currentAdmin.logAction(AdminActionType::RETURN_BOOK, details);
    }
    
    bool success = m_db.returnOneCopy(isbn);
    
    if (success) {
        // Refresh book cache after returning
        refreshBookCache();
    }
    
    return success;
}

bool Library::returnOneCopy(const std::string &isbn, const std::string &studentId)
{
    // Get the student name for the log
    Student student = getStudent(studentId);
    std::string studentName = student.getName();
    
    // Log the action if an admin is logged in
    if (!m_currentAdmin.getId().empty()) {
        std::string details = "Returned book: ISBN " + isbn + " from student " + 
                              studentName + " (ID: " + studentId + ")";
        m_currentAdmin.logAction(AdminActionType::RETURN_BOOK, details);
    }
    
    bool success = m_db.returnOneCopy(isbn, studentId);
    
    if (success) {
        // Refresh book cache after returning
        refreshBookCache();
    }
    
    return success;
}

const std::vector<Book>& Library::getAllBooks() const
{
    return m_books;
}

// ---------------------
//  New removeBook() method
// ---------------------
bool Library::removeBook(const std::string &isbn)
{
    // Log the action if an admin is logged in
    if (!m_currentAdmin.getId().empty()) {
        std::string details = "Removed book: ISBN " + isbn;
        m_currentAdmin.logAction(AdminActionType::REMOVE_BOOK, details);
    }
    
    bool success = m_db.removeBook(isbn);
    
    if (success) {
        // Refresh book cache after removing
        refreshBookCache();
    }
    
    return success;
}

// Student operations
bool Library::addStudent(const Student &student)
{
    // Log the action if an admin is logged in
    if (!m_currentAdmin.getId().empty()) {
        std::string details = "Added student: " + student.getName() + " (ID: " + student.getId() + ")";
        m_currentAdmin.logAction(AdminActionType::ADD_STUDENT, details);
    }
    
    return m_db.addStudent(student);
}

bool Library::updateStudent(const Student &student)
{
    // Log the action if an admin is logged in
    if (!m_currentAdmin.getId().empty()) {
        std::string details = "Updated student: " + student.getName() + " (ID: " + student.getId() + ")";
        m_currentAdmin.logAction(AdminActionType::UPDATE_STUDENT, details);
    }

    return m_db.updateStudent(student);
}

bool Library::removeStudent(const std::string &studentId)
{
    // Get student details for the log before removing
    Student student = m_db.getStudent(studentId);
    
    // Log the action if an admin is logged in and the student exists
    if (!m_currentAdmin.getId().empty() && !student.getId().empty()) {
        std::string details = "Removed student: " + student.getName() + " (ID: " + student.getId() + ")";
        m_currentAdmin.logAction(AdminActionType::REMOVE_STUDENT, details);
    }
    
    return m_db.removeStudent(studentId);
}

Student Library::getStudent(const std::string &studentId) const
{
    return m_db.getStudent(studentId);
}

std::vector<Student> Library::getAllStudents() const
{
    return m_db.getAllStudents();
}

std::vector<Student> Library::getStudentsForYear(const std::string &year) const
{
    std::vector<Student> allStudents = m_db.getAllStudents();
    std::vector<Student> filteredStudents;
    
    for (const auto &student : allStudents) {
        if (student.getYear() == year) {
            filteredStudents.push_back(student);
        }
    }
    
    return filteredStudents;
}

std::vector<Student> Library::getStudentsForYearAndGroup(const std::string &year, const std::string &group) const
{
    std::vector<Student> allStudents = m_db.getAllStudents();
    std::vector<Student> filteredStudents;
    
    for (const auto &student : allStudents) {
        if (student.getYear() == year && student.getGroup() == group) {
            filteredStudents.push_back(student);
        }
    }
    
    return filteredStudents;
}

// Admin operations
bool Library::authenticateAdmin(const std::string &adminId, const std::string &password)
{
    bool success = m_db.authenticateAdmin(adminId, password);
    
    if (success) {
        // Set the current admin
        Admin admin = m_db.getAdmin(adminId);
        setCurrentAdmin(admin);
        
        // Log the login action
        m_currentAdmin.logAction(AdminActionType::LOGIN, "Admin logged in");
    }
    
    return success;
}

Admin Library::getCurrentAdmin() const
{
    return m_currentAdmin;
}

bool Library::setCurrentAdmin(const Admin &admin)
{
    m_currentAdmin = admin;
    return true;
}

bool Library::logoutCurrentAdmin()
{
    if (!m_currentAdmin.getId().empty()) {
        m_currentAdmin.logAction(AdminActionType::LOGOUT, "Admin logged out");
    }
    
    m_currentAdmin = Admin(); // Reset to default
    return true;
}

bool Library::addAdmin(const Admin &admin)
{
    // Only the root admin can add new admins
    if (!m_currentAdmin.isRoot()) {
        return false;
    }
    
    // Log the action
    std::string details = "Added admin: " + admin.getName() + " (ID: " + admin.getId() + ")";
    m_currentAdmin.logAction(AdminActionType::ADD_ADMIN, details);
    
    return m_db.addAdmin(admin);
}

bool Library::updateAdmin(const Admin &admin)
{
    // Only the root admin can update admins
    if (!m_currentAdmin.isRoot()) {
        return false;
    }
    
    return m_db.updateAdmin(admin);
}

bool Library::removeAdmin(const std::string &adminId)
{
    // Only the root admin can remove admins
    if (!m_currentAdmin.isRoot()) {
        return false;
    }
    
    return m_db.removeAdmin(adminId);
}

std::vector<Admin> Library::getAllAdmins() const
{
    // Only the root admin can view all admins
    if (!m_currentAdmin.isRoot()) {
        return std::vector<Admin>();
    }
    
    return m_db.getAllAdmins();
}

std::vector<Student::BorrowRecord> Library::getStudentBorrowHistory(const std::string &studentId) const
{
    return m_db.getStudentBorrowHistory(studentId);
}

// Validation helpers
int Library::getMaxGroupsForYear(const std::string &year)
{
    if (year == "1" || year == "2") {
        return 12;
    } else if (year == "3") {
        return 8;
    } else if (year == "4") {
        return 4;
    }
    
    return 0; // Invalid year
}

bool Library::isValidStudentData(const std::string &year, const std::string &group, std::string &errorMsg)
{
    // Check year
    if (year != "1" && year != "2" && year != "3" && year != "4") {
        errorMsg = "Invalid year. Must be 1, 2, 3, or 4.";
        return false;
    }
    
    // Check group based on year
    int maxGroups = getMaxGroupsForYear(year);
    int groupNum = std::stoi(group);
    
    if (groupNum < 1 || groupNum > maxGroups) {
        errorMsg = "Invalid group for year " + year + ". Must be between 1 and " + std::to_string(maxGroups) + ".";
        return false;
    }
    
    return true;
}

bool Library::initializeFromCSVIfNeeded()
{
    // Check if students table is empty
    if (m_db.isStudentTableEmpty()) {
        qInfo() << "Student database is empty. Attempting to initialize from CSV...";
        
        // Try to import from students.csv
        QString csvPath = "students.csv";
        if (m_db.importStudentsFromCSV(csvPath)) {
            qInfo() << "Successfully initialized student database from CSV file";
            return true;
        } else {
            qWarning() << "Failed to initialize student database from CSV file";
            return false;
        }
    }
    
    // Database already has students
    return true;
}

void Library::refreshBookCache()
{
    m_books = m_db.getAllBooks();
} 