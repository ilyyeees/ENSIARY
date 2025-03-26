#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QDateEdit>
#include <QComboBox>
#include <QDialog>
#include <QStackedWidget>

// Include your backend classes:
#include "librarymanager.h"  // This header should include definitions for Book, User, Student, and Admin

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Book management
    void onAddBookClicked();
    void onBorrowBookClicked();
    void onReturnBookClicked();
    void onClearInputs();
    void onDeleteBookClicked();
    void onSelectionChanged();
    void onSearchTextChanged(const QString &text);
    void onTabChanged(int index);
    void onSortColumnClicked(int column);
    void onShowBorrowerInfo();
    void updateClock();
    void showHelp();
    void showContextHelp();
    
    // Student management
    void onAddStudentClicked();
    void onEditStudentClicked();
    void onDeleteStudentClicked();
    void onClearStudentInputs();
    void onStudentSelectionChanged();
    void onStudentTabChanged(int index);
    void onStudentYearChanged(const QString &year);
    void populateGroupComboBox();
    void updateGroupFilterOptions();
    
    // Admin authentication
    void onLoginClicked();
    void onLogoutClicked();
    void onAddAdminClicked();
    void onDeleteAdminClicked();
    void showLoginScreen();
    void showMainInterface();
    void updateAdminProfileDisplay();

private:
    // UI setup
    void applyStyleSheet();
    void setupUI();
    void setupLoginScreen();
    void setupMainInterface();
    void setupBookManagement();
    void setupStudentManagement();
    void setupStatusBar();
    void setupToolBar();
    void setupAdminProfile();
    void setupAnimations();
    void setupConnections();
    
    // Data operations
    void onRefreshTable();
    void onRefreshStudentTable();
    void onRefreshAdminTable();
    void updateButtonStates();
    void updateStudentButtonStates();
    void updateStatusMessage(const QString &message);
    void animateStatusMessage(const QString &message);
    bool filterBook(const Book &book, const QString &searchText);
    bool filterStudent(const Student &student, const QString &searchText);
    
    // Display due dates and overdue books
    void updateDueDateDisplay();
    void highlightOverdueBooks();
    
    // Refresh borrowing history for a specific student
    void refreshBorrowHistory(const QString &studentId);
    
    // For logged-in admin access control
    bool checkAdminLoggedIn();

    // UI pointers for the stacked layout
    QStackedWidget   *m_stackedWidget;
    QWidget         *m_loginScreen;
    QWidget         *m_mainInterface;
    
    // Login screen widgets
    QLineEdit       *m_usernameInput;
    QLineEdit       *m_passwordInput;
    QPushButton     *m_loginButton;
    QLabel          *m_loginStatusLabel;
    
    // Admin profile display
    QLabel          *m_adminNameLabel;
    QPushButton     *m_logoutButton;
    QTableWidget    *m_adminActionTable;
    QTableWidget    *m_adminListTable;
    QPushButton     *m_addAdminButton;
    QPushButton     *m_deleteAdminButton;
    
    // Book management UI pointers
    QLineEdit       *m_searchInput;
    QComboBox       *m_bookSearchTypeCombo;
    QLineEdit       *m_titleInput;
    QLineEdit       *m_authorInput;
    QLineEdit       *m_isbnInput;
    QSpinBox        *m_copiesSpin;
    QPushButton     *m_addBookButton;
    QPushButton     *m_borrowBookButton;
    QPushButton     *m_returnBookButton;
    QPushButton     *m_clearButton;
    QPushButton     *m_showBorrowerButton;
    QPushButton     *m_deleteBookButton;
    QTableWidget    *m_allBooksTable;
    QTableWidget    *m_availableBooksTable;
    QTableWidget    *m_borrowedBooksTable;
    QTabWidget      *m_tabWidget;
    QWidget         *m_bookInputForm;
    
    // Student management UI pointers
    QLineEdit       *m_studentSearchInput;
    QComboBox       *m_studentSearchTypeCombo;
    QComboBox       *m_studentYearFilterCombo;
    QComboBox       *m_studentGroupFilterCombo;
    QLineEdit       *m_studentNameInput;
    QLineEdit       *m_studentIdInput;
    QLineEdit       *m_studentEmailInput;
    QDateEdit       *m_studentDobInput;
    QComboBox       *m_studentYearComboBox;
    QComboBox       *m_studentGroupComboBox;
    QPushButton     *m_addStudentButton;
    QPushButton     *m_editStudentButton;
    QPushButton     *m_deleteStudentButton;
    QPushButton     *m_clearStudentButton;
    QTableWidget    *m_allStudentsTable;
    QTableWidget    *m_studentBorrowHistoryTable;
    QTabWidget      *m_studentTabWidget;
    QWidget         *m_studentInputForm;
    
    // Status and sorting variables
    QLabel          *m_statusLabel;
    QLabel          *m_clockLabel;
    QGraphicsOpacityEffect *m_statusOpacity;
    int              m_sortColumn;
    bool             m_sortAscending;
    QString          m_currentSearchText;
    QString          m_currentStudentSearchText;
    
    // Borrowing dialog components
    QComboBox       *m_studentYearFilterComboBox;
    QComboBox       *m_studentGroupFilterComboBox;
    QComboBox       *m_studentNameComboBox;

    // Backend connection
    Library m_library;

    // Update student list for borrowing
    void updateStudentBorrowList(const std::vector<Student>& students);

    // Helper methods
    void refreshBookCache();
    void refreshStudentTable();

    // Toolbar actions and help
    
    // Book refresh
    void refreshBookTables();
};

#endif // MAINWINDOW_H 