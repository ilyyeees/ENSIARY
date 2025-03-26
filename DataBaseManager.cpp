#include "DatabaseManager.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    initializeDatabase();
}

bool DatabaseManager::initializeDatabase()
{
    // Create (or open) a SQLite database file named "library.db"
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("library.db");

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    return createTables();
}

bool DatabaseManager::createTables()
{
    // Drop old single-copy table if you like, or ignore. For a fresh start:
    // QSqlQuery dropOld(m_db);
    // dropOld.exec("DROP TABLE IF EXISTS Books");
    // dropOld.exec("DROP TABLE IF EXISTS BookCopies");

    // 1) Create the main Books table
    QString createBooksTable = R"(
        CREATE TABLE IF NOT EXISTS Books (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            author TEXT NOT NULL,
            isbn TEXT NOT NULL UNIQUE,
            total_copies INTEGER NOT NULL DEFAULT 1
        )
    )";
    QSqlQuery query(m_db);
    if (!query.exec(createBooksTable)) {
        qWarning() << "Failed to create Books table:" << query.lastError().text();
        return false;
    }

    // 2) Create BookCopies table
    QString createCopiesTable = R"(
        CREATE TABLE IF NOT EXISTS BookCopies (
            copy_id INTEGER PRIMARY KEY AUTOINCREMENT,
            book_id INTEGER NOT NULL,
            borrowed INTEGER NOT NULL DEFAULT 0,
            borrower_name TEXT,
            borrower_id TEXT,
            borrower_year TEXT,
            borrower_group TEXT,
            borrowed_date TEXT,
            FOREIGN KEY (book_id) REFERENCES Books(id)
        )
    )";
    QSqlQuery query2(m_db);
    if (!query2.exec(createCopiesTable)) {
        qWarning() << "Failed to create BookCopies table:" << query2.lastError().text();
        return false;
    }

    // 3) Create Students table
    QString createStudentsTable = R"(
        CREATE TABLE IF NOT EXISTS Students (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            dob TEXT NOT NULL,
            year TEXT NOT NULL,
            group_num TEXT NOT NULL,
            email TEXT
        )
    )";
    QSqlQuery query3(m_db);
    if (!query3.exec(createStudentsTable)) {
        qWarning() << "Failed to create Students table:" << query3.lastError().text();
        return false;
    }

    // 4) Create Student Borrow History table
    QString createBorrowHistoryTable = R"(
        CREATE TABLE IF NOT EXISTS BorrowHistory (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            student_id TEXT NOT NULL,
            book_isbn TEXT NOT NULL,
            book_title TEXT NOT NULL,
            book_author TEXT NOT NULL,
            borrow_date TEXT NOT NULL,
            return_date TEXT,
            FOREIGN KEY (student_id) REFERENCES Students(id)
        )
    )";
    QSqlQuery query4(m_db);
    if (!query4.exec(createBorrowHistoryTable)) {
        qWarning() << "Failed to create BorrowHistory table:" << query4.lastError().text();
        return false;
    }

    // 5) Create Admins table
    QString createAdminsTable = R"(
        CREATE TABLE IF NOT EXISTS Admins (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            password TEXT NOT NULL
        )
    )";
    QSqlQuery query5(m_db);
    if (!query5.exec(createAdminsTable)) {
        qWarning() << "Failed to create Admins table:" << query5.lastError().text();
        return false;
    }

    // 6) Create Admin Action Log table
    QString createActionLogTable = R"(
        CREATE TABLE IF NOT EXISTS AdminActionLog (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            admin_id TEXT NOT NULL,
            admin_name TEXT NOT NULL,
            action_type INTEGER NOT NULL,
            action_details TEXT NOT NULL,
            timestamp TEXT NOT NULL,
            FOREIGN KEY (admin_id) REFERENCES Admins(id)
        )
    )";
    QSqlQuery query6(m_db);
    if (!query6.exec(createActionLogTable)) {
        qWarning() << "Failed to create AdminActionLog table:" << query6.lastError().text();
        return false;
    }

    // Check if root admin exists, if not, create it
    QSqlQuery checkRoot(m_db);
    checkRoot.prepare("SELECT id FROM Admins WHERE id = 'root'");
    if (checkRoot.exec() && !checkRoot.next()) {
        QSqlQuery addRoot(m_db);
        addRoot.prepare("INSERT INTO Admins (id, name, password) VALUES ('root', 'Root Admin', 'root')");
        if (!addRoot.exec()) {
            qWarning() << "Failed to create root admin:" << addRoot.lastError().text();
        }
    }

    return true;
}

// Create a new book + N copies in BookCopies
bool DatabaseManager::addBook(const std::string &title,
                              const std::string &author,
                              const std::string &isbn,
                              int totalCopies)
{
    // Insert into Books
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO Books (title, author, isbn, total_copies)
        VALUES (:title, :author, :isbn, :tc)
    )");
    query.bindValue(":title",  QString::fromStdString(title));
    query.bindValue(":author", QString::fromStdString(author));
    query.bindValue(":isbn",   QString::fromStdString(isbn));
    query.bindValue(":tc",     totalCopies);

    if (!query.exec()) {
        qWarning() << "Failed to add book (Books table):" << query.lastError().text();
        return false;
    }

    // Get the new book ID
    int bookId = query.lastInsertId().toInt();
    if (bookId <= 0) {
        qWarning() << "Invalid book ID after insert";
        return false;
    }

    // Create N copies in BookCopies
    return createCopiesForBook(bookId, totalCopies);
}

bool DatabaseManager::addBook(const Book &book)
{
    return addBook(book.getTitle(), book.getAuthor(), book.getISBN(), book.getTotalCopies());
}

bool DatabaseManager::createCopiesForBook(int bookId, int totalCopies)
{
    // Insert 'totalCopies' rows into BookCopies with borrowed=0
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO BookCopies (book_id, borrowed)
        VALUES (:book_id, 0)
    )");

    // Use transaction for speed
    m_db.transaction();
    for (int i = 0; i < totalCopies; ++i) {
        query.bindValue(":book_id", bookId);
        if (!query.exec()) {
            qWarning() << "Failed to create copy for book" << bookId
                       << query.lastError().text();
            m_db.rollback();
            return false;
        }
    }
    m_db.commit();
    return true;
}

// Borrow 1 available copy of a book
bool DatabaseManager::borrowOneCopy(const std::string &isbn, const User &user)
{
    // Find the book ID from isbn
    QSqlQuery findBook(m_db);
    findBook.prepare("SELECT id FROM Books WHERE isbn = :isbn");
    findBook.bindValue(":isbn", QString::fromStdString(isbn));
    if (!findBook.exec() || !findBook.next()) {
        qWarning() << "No such book (borrowOneCopy):" << isbn.c_str();
        return false;
    }
    int bookId = findBook.value(0).toInt();

    // Find one un-borrowed copy in BookCopies
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT copy_id FROM BookCopies
        WHERE book_id = :bid AND borrowed = 0
        LIMIT 1
    )");
    query.bindValue(":bid", bookId);
    if (!query.exec() || !query.next()) {
        // No available copies
        return false;
    }
    int copyId = query.value(0).toInt();

    // Mark that copy as borrowed
    QSqlQuery updateCopy(m_db);
    updateCopy.prepare(R"(
        UPDATE BookCopies
        SET borrowed = 1,
            borrower_name = :bname,
            borrower_id   = :bidval,
            borrower_year = :byear,
            borrower_group= :bgroup,
            borrowed_date = :bdate
        WHERE copy_id = :cid
    )");
    updateCopy.bindValue(":bname",  QString::fromStdString(user.getName()));
    updateCopy.bindValue(":bidval", QString::fromStdString(user.getId()));
    updateCopy.bindValue(":byear",  QString::fromStdString(user.getYear()));
    updateCopy.bindValue(":bgroup", QString::fromStdString(user.getGroup()));
    updateCopy.bindValue(":bdate",  QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    updateCopy.bindValue(":cid",    copyId);

    if (!updateCopy.exec()) {
        qWarning() << "Failed to borrow copy:" << updateCopy.lastError().text();
        return false;
    }
    return (updateCopy.numRowsAffected() > 0);
}

// Return 1 borrowed copy
bool DatabaseManager::returnOneCopy(const std::string &isbn)
{
    // Return one copy of the book with the given ISBN from any borrower
    // First find the book ID from isbn
    QSqlQuery findBook(m_db);
    findBook.prepare("SELECT id FROM Books WHERE isbn = :isbn");
    findBook.bindValue(":isbn", QString::fromStdString(isbn));
    if (!findBook.exec() || !findBook.next()) {
        qWarning() << "No such book (returnOneCopy):" << isbn.c_str();
        return false;
    }
    int bookId = findBook.value(0).toInt();

    // Find any borrowed copy
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT copy_id FROM BookCopies
        WHERE book_id = :bid AND borrowed = 1
        LIMIT 1
    )");
    query.bindValue(":bid", bookId);
    if (!query.exec() || !query.next()) {
        // No borrowed copies
        return false;
    }
    int copyId = query.value(0).toInt();

    // Mark that copy as returned
    QSqlQuery updateCopy(m_db);
    updateCopy.prepare(R"(
        UPDATE BookCopies
        SET borrowed = 0,
            borrower_name = NULL,
            borrower_id   = NULL,
            borrower_year = NULL,
            borrower_group= NULL,
            borrowed_date = NULL
        WHERE copy_id = :cid
    )");
    updateCopy.bindValue(":cid", copyId);
    
    if (!updateCopy.exec()) {
        qWarning() << "Failed to return copy:" << updateCopy.lastError().text();
        return false;
    }
    return (updateCopy.numRowsAffected() > 0);
}

bool DatabaseManager::returnOneCopy(const std::string &isbn, const std::string &studentId)
{
    // Return one copy of the book from a specific student
    // First find the book ID from isbn
    QSqlQuery findBook(m_db);
    findBook.prepare("SELECT id FROM Books WHERE isbn = :isbn");
    findBook.bindValue(":isbn", QString::fromStdString(isbn));
    if (!findBook.exec() || !findBook.next()) {
        qWarning() << "No such book (returnOneCopy):" << isbn.c_str();
        return false;
    }
    int bookId = findBook.value(0).toInt();

    // Find the specific student's borrow
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT copy_id FROM BookCopies
        WHERE book_id = :bid AND borrowed = 1 AND borrower_id = :student_id
        LIMIT 1
    )");
    query.bindValue(":bid", bookId);
    query.bindValue(":student_id", QString::fromStdString(studentId));
    if (!query.exec() || !query.next()) {
        qWarning() << "No borrowed copy found for this student:" << studentId.c_str();
        return false;
    }
    int copyId = query.value(0).toInt();

    // Mark that copy as returned
    QSqlQuery updateCopy(m_db);
    updateCopy.prepare(R"(
        UPDATE BookCopies
        SET borrowed = 0,
            borrower_name = NULL,
            borrower_id   = NULL,
            borrower_year = NULL,
            borrower_group= NULL,
            borrowed_date = NULL
        WHERE copy_id = :cid
    )");
    updateCopy.bindValue(":cid", copyId);
    
    if (!updateCopy.exec()) {
        qWarning() << "Failed to return copy:" << updateCopy.lastError().text();
        return false;
    }
    
    bool success = (updateCopy.numRowsAffected() > 0);
    
    // If successful, update the student's borrow history
    if (success) {
        // Get book info for the history update
        QSqlQuery bookQuery(m_db);
        bookQuery.prepare("SELECT title, author FROM Books WHERE isbn = :isbn");
        bookQuery.bindValue(":isbn", QString::fromStdString(isbn));
        if (bookQuery.exec() && bookQuery.next()) {
            QString title = bookQuery.value(0).toString();
            QString author = bookQuery.value(1).toString();
            
            // Update the student's borrow history
            updateBorrowHistory(studentId, isbn, title.toStdString(), author.toStdString(), true);
        }
    }
    
    return success;
}

// Return a vector<Book> with borrowed info
std::vector<Book> DatabaseManager::getAllBooks() const
{
    std::vector<Book> books;

    // 1) Query all from Books
    QSqlQuery qBooks("SELECT id, title, author, isbn, total_copies FROM Books", m_db);
    while (qBooks.next()) {
        int bookId      = qBooks.value(0).toInt();
        QString title   = qBooks.value(1).toString();
        QString author  = qBooks.value(2).toString();
        QString isbn    = qBooks.value(3).toString();
        int totalCopies = qBooks.value(4).toInt();

        // Build a Book object
        Book book(title.toStdString(), author.toStdString(), isbn.toStdString());
        book.setTotalCopies(totalCopies);

        // 2) Query how many copies are borrowed & gather borrower info
        QSqlQuery qCopies(m_db);
        qCopies.prepare(R"(
            SELECT copy_id, borrowed, borrower_name, borrower_id, borrower_year,
                   borrower_group, borrowed_date
            FROM BookCopies
            WHERE book_id = :bid
        )");
        qCopies.bindValue(":bid", bookId);
        if (!qCopies.exec()) {
            qWarning() << "Failed to get BookCopies:" << qCopies.lastError().text();
            continue;
        }

        int borrowedCount = 0;
        std::vector<Book::BorrowerInfo> borrowers;
        while (qCopies.next()) {
            bool isBorrowed = qCopies.value(1).toBool();
            if (isBorrowed) {
                borrowedCount++;
                Book::BorrowerInfo info;
                info.name         = qCopies.value(2).toString().toStdString();
                info.id           = qCopies.value(3).toString().toStdString();
                info.year         = qCopies.value(4).toString().toStdString();
                info.group        = qCopies.value(5).toString().toStdString();
                info.borrowedDate = qCopies.value(6).toString().toStdString();
                borrowers.push_back(info);
            }
        }
        book.setBorrowedCount(borrowedCount);
        book.setBorrowers(borrowers);

        books.push_back(book);
    }

    return books;
}
bool DatabaseManager::removeBook(const std::string &isbn)
{
    // 1) Find the book's ID from the Books table
    QSqlQuery findBook(m_db);
    findBook.prepare("SELECT id FROM Books WHERE isbn = :isbn");
    findBook.bindValue(":isbn", QString::fromStdString(isbn));
    if (!findBook.exec() || !findBook.next()) {
        qWarning() << "No such book (removeBook):" << isbn.c_str();
        return false;
    }
    int bookId = findBook.value(0).toInt();

    // 2) Use a transaction for atomic removal
    if (!m_db.transaction()) {
        qWarning() << "Failed to start transaction:" << m_db.lastError().text();
        return false;
    }

    // 3) Delete all copies from BookCopies for that book_id
    QSqlQuery deleteCopies(m_db);
    deleteCopies.prepare("DELETE FROM BookCopies WHERE book_id = :bid");
    deleteCopies.bindValue(":bid", bookId);
    if (!deleteCopies.exec()) {
        qWarning() << "Failed to delete copies:" << deleteCopies.lastError().text();
        m_db.rollback();
        return false;
    }

    // 4) Delete the book row from Books
    QSqlQuery deleteBook(m_db);
    deleteBook.prepare("DELETE FROM Books WHERE id = :bid");
    deleteBook.bindValue(":bid", bookId);
    if (!deleteBook.exec()) {
        qWarning() << "Failed to delete book:" << deleteBook.lastError().text();
        m_db.rollback();
        return false;
    }

    // 5) Commit the transaction
    if (!m_db.commit()) {
        qWarning() << "Failed to commit transaction:" << m_db.lastError().text();
        return false;
    }

    return true;
}

// Student management methods
bool DatabaseManager::addStudent(const Student &student)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO Students (id, name, dob, year, group_num, email)
        VALUES (:id, :name, :dob, :year, :group, :email)
    )");
    
    query.bindValue(":id", QString::fromStdString(student.getId()));
    query.bindValue(":name", QString::fromStdString(student.getName()));
    query.bindValue(":dob", QString::fromStdString(student.getDob()));
    query.bindValue(":year", QString::fromStdString(student.getYear()));
    query.bindValue(":group", QString::fromStdString(student.getGroup()));
    query.bindValue(":email", QString::fromStdString(student.getEmail()));
    
    if (!query.exec()) {
        qWarning() << "Failed to add student:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::updateStudent(const Student &student)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE Students
        SET name = :name, dob = :dob, year = :year, group_num = :group, email = :email
        WHERE id = :id
    )");
    
    query.bindValue(":id", QString::fromStdString(student.getId()));
    query.bindValue(":name", QString::fromStdString(student.getName()));
    query.bindValue(":dob", QString::fromStdString(student.getDob()));
    query.bindValue(":year", QString::fromStdString(student.getYear()));
    query.bindValue(":group", QString::fromStdString(student.getGroup()));
    query.bindValue(":email", QString::fromStdString(student.getEmail()));
    
    if (!query.exec()) {
        qWarning() << "Failed to update student:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::removeStudent(const std::string &studentId)
{
    // First, check if the student has any active borrows
    QSqlQuery checkBorrows(m_db);
    checkBorrows.prepare(R"(
        SELECT COUNT(*) FROM BorrowHistory
        WHERE student_id = :id AND return_date IS NULL
    )");
    checkBorrows.bindValue(":id", QString::fromStdString(studentId));
    
    if (!checkBorrows.exec() || !checkBorrows.next()) {
        qWarning() << "Failed to check student borrows:" << checkBorrows.lastError().text();
        return false;
    }
    
    int activeBorrows = checkBorrows.value(0).toInt();
    if (activeBorrows > 0) {
        qWarning() << "Cannot remove student with active borrows";
        return false;
    }
    
    // Delete the student
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM Students WHERE id = :id");
    query.bindValue(":id", QString::fromStdString(studentId));
    
    if (!query.exec()) {
        qWarning() << "Failed to remove student:" << query.lastError().text();
        return false;
    }
    
    return true;
}

Student DatabaseManager::getStudent(const std::string &studentId) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, dob, year, group_num, email FROM Students WHERE id = :id");
    query.bindValue(":id", QString::fromStdString(studentId));
    
    if (!query.exec() || !query.next()) {
        // Student not found
        return Student();
    }
    
    Student student(
        query.value("name").toString().toStdString(),
        query.value("id").toString().toStdString(),
        query.value("dob").toString().toStdString(),
        query.value("year").toString().toStdString(),
        query.value("group_num").toString().toStdString(),
        query.value("email").toString().toStdString()
    );
    
    // Load the student's borrow history
    std::vector<Student::BorrowRecord> history = getStudentBorrowHistory(studentId);
    for (const auto &record : history) {
        student.addBorrowRecord(record);
    }
    
    return student;
}

std::vector<Student> DatabaseManager::getAllStudents() const
{
    std::vector<Student> students;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, dob, year, group_num, email FROM Students");
    
    if (!query.exec()) {
        qWarning() << "Failed to get students:" << query.lastError().text();
        return students;
    }
    
    while (query.next()) {
        Student student(
            query.value("name").toString().toStdString(),
            query.value("id").toString().toStdString(),
            query.value("dob").toString().toStdString(),
            query.value("year").toString().toStdString(),
            query.value("group_num").toString().toStdString(),
            query.value("email").toString().toStdString()
        );
        
        students.push_back(student);
    }
    
    qDebug("DatabaseManager::getAllStudents() retrieved %d students", students.size());
    return students;
}

std::vector<Student::BorrowRecord> DatabaseManager::getStudentBorrowHistory(const std::string &studentId) const
{
    std::vector<Student::BorrowRecord> history;
    
    QSqlQuery query(m_db);
    query.prepare(R"(
        SELECT book_isbn, book_title, book_author, borrow_date, return_date
        FROM BorrowHistory
        WHERE student_id = :id
        ORDER BY borrow_date DESC
    )");
    query.bindValue(":id", QString::fromStdString(studentId));
    
    if (!query.exec()) {
        qWarning() << "Failed to get borrow history:" << query.lastError().text();
        return history;
    }
    
    while (query.next()) {
        Student::BorrowRecord record;
        record.bookIsbn = query.value("book_isbn").toString().toStdString();
        record.bookTitle = query.value("book_title").toString().toStdString();
        record.bookAuthor = query.value("book_author").toString().toStdString();
        record.borrowDate = query.value("borrow_date").toString().toStdString();
        
        // Return date may be NULL for active borrows
        if (!query.value("return_date").isNull()) {
            record.returnDate = query.value("return_date").toString().toStdString();
        } else {
            record.returnDate = "";
        }
        
        history.push_back(record);
    }
    
    return history;
}

void DatabaseManager::updateBorrowHistory(const std::string &studentId, const std::string &isbn, 
                                         const std::string &title, const std::string &author, bool isReturn)
{
    if (isReturn) {
        // Update the return date for the borrow record
        QSqlQuery query(m_db);
        query.prepare(R"(
            UPDATE BorrowHistory
            SET return_date = :return_date
            WHERE student_id = :student_id AND book_isbn = :isbn AND return_date IS NULL
        )");
        query.bindValue(":return_date", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        query.bindValue(":student_id", QString::fromStdString(studentId));
        query.bindValue(":isbn", QString::fromStdString(isbn));
        
        if (!query.exec()) {
            qWarning() << "Failed to update return date:" << query.lastError().text();
        }
    } else {
        // Add a new borrow record
        QSqlQuery query(m_db);
        query.prepare(R"(
            INSERT INTO BorrowHistory (student_id, book_isbn, book_title, book_author, borrow_date)
            VALUES (:student_id, :isbn, :title, :author, :borrow_date)
        )");
        query.bindValue(":student_id", QString::fromStdString(studentId));
        query.bindValue(":isbn", QString::fromStdString(isbn));
        query.bindValue(":title", QString::fromStdString(title));
        query.bindValue(":author", QString::fromStdString(author));
        query.bindValue(":borrow_date", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        
        if (!query.exec()) {
            qWarning() << "Failed to add borrow record:" << query.lastError().text();
        }
    }
}

// Admin management methods
bool DatabaseManager::isStudentTableEmpty() const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM Students");
    
    if (!query.exec() || !query.next()) {
        qWarning() << "Failed to check if Students table is empty:" << query.lastError().text();
        return true; // Assume empty if there's an error
    }
    
    return query.value(0).toInt() == 0;
}

bool DatabaseManager::importStudentsFromCSV(const QString &filename)
{
    QFile file(filename);
    if (!file.exists()) {
        qWarning() << "CSV file does not exist:" << filename;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open CSV file:" << filename;
        return false;
    }
    
    QTextStream in(&file);
    QString headerLine = in.readLine(); // Skip header line
    
    // Start a transaction for better performance
    m_db.transaction();
    
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO Students (id, name, dob, year, group_num, email) VALUES (:id, :name, :dob, :year, :group, :email)");
    
    int lineNumber = 1; // Header is line 1
    int successCount = 0;
    
    while (!in.atEnd()) {
        lineNumber++;
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        QStringList fields = line.split(",");
        if (fields.size() < 4) {
            qWarning() << "Line" << lineNumber << "has fewer than 4 fields - skipping";
            continue;
        }
        
        QString name = fields[0].trimmed();
        QString id = fields[1].trimmed();
        
        // Parse the group-section field
        QString groupSection = fields[2].trimmed();
        QString group = "1"; // Default group
        
        // If group-section is in format "x-y", extract y as the group
        if (groupSection.contains("-")) {
            QStringList parts = groupSection.split("-");
            if (parts.size() >= 2) {
                group = parts[1].trimmed();
            }
        }
        
        QString email = fields.size() > 3 ? fields[3].trimmed() : "";
        
        // All students are assumed to be first year with empty DOB
        query.bindValue(":id", id);
        query.bindValue(":name", name);
        query.bindValue(":dob", ""); // Empty DOB as per requirements
        query.bindValue(":year", "1"); // All treated as first year
        query.bindValue(":group", group);
        query.bindValue(":email", email);
        
        if (query.exec()) {
            successCount++;
        } else {
            qWarning() << "Failed to insert student from line" << lineNumber << ":" << query.lastError().text();
            // Continue with next student even if this one fails
        }
    }
    
    file.close();
    
    if (!m_db.commit()) {
        qWarning() << "Failed to commit CSV import transaction:" << m_db.lastError().text();
        m_db.rollback();
        return false;
    }
    
    qInfo() << "Successfully imported" << successCount << "students from CSV file";
    return successCount > 0;
}

bool DatabaseManager::authenticateAdmin(const std::string &adminId, const std::string &password)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT password FROM Admins WHERE id = :id");
    query.bindValue(":id", QString::fromStdString(adminId));
    
    if (!query.exec() || !query.next()) {
        qWarning() << "Admin not found:" << adminId.c_str();
        return false;
    }
    
    QString storedPassword = query.value("password").toString();
    return storedPassword.toStdString() == password;
}

bool DatabaseManager::addAdmin(const Admin &admin)
{
    // Only root can add admins
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO Admins (id, name, password)
        VALUES (:id, :name, :password)
    )");
    
    query.bindValue(":id", QString::fromStdString(admin.getId()));
    query.bindValue(":name", QString::fromStdString(admin.getName()));
    query.bindValue(":password", QString::fromStdString(admin.getPassword()));
    
    if (!query.exec()) {
        qWarning() << "Failed to add admin:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::updateAdmin(const Admin &admin)
{
    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE Admins
        SET name = :name
        WHERE id = :id
    )");
    
    query.bindValue(":id", QString::fromStdString(admin.getId()));
    query.bindValue(":name", QString::fromStdString(admin.getName()));
    
    if (!query.exec()) {
        qWarning() << "Failed to update admin:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::removeAdmin(const std::string &adminId)
{
    // Cannot remove root admin
    if (adminId == "root") {
        qWarning() << "Cannot remove root admin";
        return false;
    }
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM Admins WHERE id = :id");
    query.bindValue(":id", QString::fromStdString(adminId));
    
    if (!query.exec()) {
        qWarning() << "Failed to remove admin:" << query.lastError().text();
        return false;
    }
    
    return true;
}

Admin DatabaseManager::getAdmin(const std::string &adminId) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name FROM Admins WHERE id = :id");
    query.bindValue(":id", QString::fromStdString(adminId));
    
    if (!query.exec() || !query.next()) {
        // Admin not found
        return Admin();
    }
    
    // We don't include the password here for security reasons
    Admin admin(
        query.value("name").toString().toStdString(),
        query.value("id").toString().toStdString(),
        "" // Empty password
    );
    
    return admin;
}

std::vector<Admin> DatabaseManager::getAllAdmins() const
{
    std::vector<Admin> admins;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name FROM Admins");
    
    if (!query.exec()) {
        qWarning() << "Failed to get admins:" << query.lastError().text();
        return admins;
    }
    
    while (query.next()) {
        Admin admin(
            query.value("name").toString().toStdString(),
            query.value("id").toString().toStdString(),
            "" // Empty password
        );
        
        admins.push_back(admin);
    }
    
    return admins;
} 