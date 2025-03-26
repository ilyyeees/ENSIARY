#include "mainwindow.h"
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QDateTime>
#include <QStyle>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QToolButton>
#include <QIcon>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QApplication>
#include <QScreen>
#include <QDialog>
#include <QSplitter>
#include <QFrame>
#include <QPropertyAnimation>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QStackedWidget>
#include <QDateEdit>
#include <QSplashScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_sortColumn(0), // Default sort by Title (column 0)
    m_sortAscending(true)
{
    // Hide the main window initially
    setWindowOpacity(0.0);
    
    // Create and show splash screen
    QPixmap splashPixmap(":/images/logo.png");
    QSplashScreen *splash = new QSplashScreen(splashPixmap);
    splash->show();
    
    // Display a loading message
    splash->showMessage("Loading application...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    QApplication::processEvents();
    
    // Create a timer to update the splash message to show progress
    QStringList loadingMessages = {
        "Initializing...",
        "Setting up UI...",
        "Loading data...",
        "Connecting signals...",
        "Almost ready..."
    };
    
    // Set window properties
    setWindowTitle("ENSIARY");
    setMinimumSize(1024, 720);

    // Center window on screen
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);

    // Use a sequential timer to perform initialization steps and update splash screen
    QTimer *initTimer = new QTimer(this);
    int step = 0;
    
    // Safety backup timer - ensure the app launches even if something goes wrong with the step timer
    QTimer::singleShot(10000, [this, splash]() {
        // If we're still not showing the main window after 10 seconds, force it to show
        if (!this->isVisible()) {
            splash->finish(this);
            splash->deleteLater();
            this->setWindowOpacity(1.0);
            this->show();
            
            // Reload all data
            refreshBookTables();
            
            animateStatusMessage("Welcome to ENSIARY");
            qDebug("Splash timeout triggered - backup showing main window");
        }
    });
    
    connect(initTimer, &QTimer::timeout, [this, splash, loadingMessages, &step, initTimer]() {
        // Update splash screen message
        if (step < loadingMessages.size()) {
            splash->showMessage(loadingMessages[step], Qt::AlignBottom | Qt::AlignCenter, Qt::white);
            QApplication::processEvents();
        }
        
        // Perform initialization steps
        switch (step) {
            case 0:
    applyStyleSheet();
                break;
            case 1:
    setupUI();
                break;
            case 2:
    setupStatusBar();
    setupToolBar();
                break;
            case 3:
    setupAnimations();
    setupConnections();
                break;
            case 4:
                // Last step - stop timer and show main window
                initTimer->stop();
                initTimer->deleteLater();

                // Close splash and show main window immediately
    QPropertyAnimation *fadeAnim = new QPropertyAnimation(this, "windowOpacity", this);
    fadeAnim->setDuration(600);
    fadeAnim->setStartValue(0.0);
    fadeAnim->setEndValue(1.0);
    fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
                
                this->show();
                splash->finish(this);
                splash->deleteLater();

    // Load the books into the tables
                refreshBookTables();
                animateStatusMessage("Welcome to ENSIARY");
                // Also load students data
                m_currentStudentSearchText = "";
                refreshStudentTable();
                
                // Make sure student tab is visible - try after a short delay
                QTimer::singleShot(500, [this]() {
                    QTabWidget* mainTabWidget = qobject_cast<QTabWidget*>(m_mainInterface->layout()->itemAt(1)->widget());
                    if (mainTabWidget) {
                        // Select student management tab (index 1)
                        mainTabWidget->setCurrentIndex(1);
                        qDebug("Switched to student management tab");
                    }
                });
                
                animateStatusMessage("Welcome to ENSIARY");
                break;
        }
        
        step++;
    });
    
    // Start the initialization timer with steps every 1.2 seconds (total ~6 seconds)
    initTimer->start(1200);
}

MainWindow::~MainWindow()
{
}

void MainWindow::applyStyleSheet()
{
    // Dark/gray theme with orange accents
    QString style = R"(
        QMainWindow {
            background-color: #2C2C3C;
        }
        QGroupBox {
            background-color: #3A3A4A;
            border: 1px solid #444444;
            border-radius: 8px;
            margin-top: 12px;
            padding: 16px;
            font-weight: bold;
            color: #F0F0F0;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 8px;
            color: #FFA000;
        }
        QLabel {
            color: #F0F0F0;
        }
        QLineEdit, QSpinBox {
            background-color: #1F1F2E;
            border: 1px solid #666666;
            border-radius: 4px;
            padding: 6px;
            color: #F0F0F0;
        }
        QLineEdit:focus, QSpinBox:focus {
            border: 2px solid #FFA000;
        }
        QPushButton {
            background-color: #FFA000;
            color: #2C2C3C;
            border: none;
            border-radius: 4px;
            padding: 10px 18px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #FFB300;
        }
        QPushButton:disabled {
            background-color: #555555;
            color: #AAAAAA;
        }
        QTabWidget::pane {
            background-color: #3A3A4A;
            border: 1px solid #444444;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: #3A3A4A;
            color: #F0F0F0;
            padding: 8px 16px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #FFA000;
            color: #2C2C3C;
            font-weight: bold;
        }
        QTableWidget {
            background-color: #2C2C3C;
            border: 1px solid #444444;
            gridline-color: #444444;
            color: #F0F0F0;
        }
        QTableWidget::item {
            padding: 4px;
        }
        QTableWidget::item:selected {
            background-color: #444455;
            color: #F0F0F0;
        }
        QHeaderView::section {
            background-color: #3A3A4A;
            color: #F0F0F0;
            border: none;
            padding: 8px;
        }
        QStatusBar {
            background-color: #3A3A4A;
        }
        QLabel#statusLabel {
            color: #F0F0F0;
        }
        QLabel#clockLabel {
            color: #2C2C3C;
            background-color: #FFA000;
            font-weight: bold;
            border-radius: 4px;
            padding: 2px 8px;
        }
        QToolBar {
            background-color: #3A3A4A;
            spacing: 8px;
            padding: 4px;
        }
        QToolButton {
            background-color: transparent;
            color: #F0F0F0;
            border-radius: 4px;
            padding: 4px;
        }
        QToolButton:hover {
            background-color: #444455;
        }
    )";
    setStyleSheet(style);
}

void MainWindow::setupUI()
{
    // Create a stacked widget to switch between login and main interface
    m_stackedWidget = new QStackedWidget(this);
    
    // Setup login screen
    m_loginScreen = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(m_loginScreen);
    loginLayout->setAlignment(Qt::AlignCenter);
    
    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/images/logo.png").scaled(300, 300, Qt::KeepAspectRatio));
    logoLabel->setAlignment(Qt::AlignCenter);
    
    QGroupBox *loginBox = new QGroupBox("Admin Login");
    QVBoxLayout *loginBoxLayout = new QVBoxLayout(loginBox);
    
    QFormLayout *loginFormLayout = new QFormLayout();
    m_usernameInput = new QLineEdit();
    m_passwordInput = new QLineEdit();
    m_passwordInput->setEchoMode(QLineEdit::Password);
    
    loginFormLayout->addRow("Username:", m_usernameInput);
    loginFormLayout->addRow("Password:", m_passwordInput);
    
    m_loginButton = new QPushButton("Login");
    m_loginStatusLabel = new QLabel();
    m_loginStatusLabel->setStyleSheet("color: red;");
    
    loginBoxLayout->addLayout(loginFormLayout);
    loginBoxLayout->addWidget(m_loginButton);
    loginBoxLayout->addWidget(m_loginStatusLabel);
    
    loginLayout->addWidget(logoLabel);
    loginLayout->addWidget(loginBox);
    loginLayout->addStretch();
    
    // Setup main interface
    m_mainInterface = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainInterface);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    
    // Admin profile display at the top
    QHBoxLayout *adminProfileLayout = new QHBoxLayout();
    m_adminNameLabel = new QLabel("Not logged in");
    m_logoutButton = new QPushButton("Logout");
    
    adminProfileLayout->addWidget(m_adminNameLabel, 1);
    adminProfileLayout->addWidget(m_logoutButton);
    
    // Main tabbed interface for Books and Students
    QTabWidget *mainTabWidget = new QTabWidget();
    
    // Book Management Tab
    QWidget *bookManagementTab = new QWidget();
    QVBoxLayout *bookLayout = new QVBoxLayout(bookManagementTab);

    // Search area with icon + input
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchInput = new QLineEdit();
    m_searchInput->setPlaceholderText("Search books...");
    QLabel *searchIcon = new QLabel();
    searchIcon->setPixmap(style()->standardPixmap(QStyle::SP_FileDialogContentsView));
    
    // Add search type dropdown
    m_bookSearchTypeCombo = new QComboBox();
    m_bookSearchTypeCombo->addItem("All Fields", "all");
    m_bookSearchTypeCombo->addItem("Title", "title");
    m_bookSearchTypeCombo->addItem("Author", "author");
    m_bookSearchTypeCombo->addItem("ISBN", "isbn");
    m_bookSearchTypeCombo->setCurrentIndex(0);
    
    searchLayout->addWidget(searchIcon);
    searchLayout->addWidget(m_bookSearchTypeCombo);
    searchLayout->addWidget(m_searchInput, 1);

    // Group box for book details
    QGroupBox *inputGroupBox = new QGroupBox("Book Details");
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(10);
    shadowEffect->setColor(QColor(0, 0, 0, 120));
    shadowEffect->setOffset(0, 2);
    inputGroupBox->setGraphicsEffect(shadowEffect);

    QGridLayout *inputLayout = new QGridLayout(inputGroupBox);
    m_titleInput = new QLineEdit();
    m_authorInput = new QLineEdit();
    m_isbnInput = new QLineEdit();
    m_copiesSpin = new QSpinBox();
    m_copiesSpin->setRange(1, 9999);
    m_copiesSpin->setValue(1);

    m_titleInput->setPlaceholderText("Enter book title");
    m_authorInput->setPlaceholderText("Enter author name");
    m_isbnInput->setPlaceholderText("Enter ISBN number");

    inputLayout->addWidget(new QLabel("Title:"),  0, 0);
    inputLayout->addWidget(m_titleInput,         0, 1);
    inputLayout->addWidget(new QLabel("Author:"), 1, 0);
    inputLayout->addWidget(m_authorInput,        1, 1);
    inputLayout->addWidget(new QLabel("ISBN:"),   2, 0);
    inputLayout->addWidget(m_isbnInput,          2, 1);
    inputLayout->addWidget(new QLabel("Copies:"), 3, 0);
    inputLayout->addWidget(m_copiesSpin,         3, 1);

    // Action buttons for books
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_addBookButton      = new QPushButton("Add Book");
    m_borrowBookButton   = new QPushButton("Borrow");
    m_returnBookButton   = new QPushButton("Return");
    m_clearButton        = new QPushButton("Clear");
    m_showBorrowerButton = new QPushButton("Borrower Info");
    m_deleteBookButton   = new QPushButton("Delete Book");

    // Optional icons
    m_addBookButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_borrowBookButton->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    m_returnBookButton->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    m_clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    m_deleteBookButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));

    buttonLayout->addWidget(m_addBookButton);
    buttonLayout->addWidget(m_borrowBookButton);
    buttonLayout->addWidget(m_returnBookButton);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_showBorrowerButton);
    buttonLayout->addWidget(m_deleteBookButton);
    buttonLayout->addStretch();

    // Combine input form + action buttons
    m_bookInputForm = new QWidget();
    QVBoxLayout *formLayout = new QVBoxLayout(m_bookInputForm);
    formLayout->addWidget(inputGroupBox);
    formLayout->addLayout(buttonLayout);
    formLayout->addStretch();

    // Tab widget for All / Available / Borrowed
    m_tabWidget = new QTabWidget();

    // All Books tab
    QWidget *allBooksTab = new QWidget();
    QVBoxLayout *allBooksLayout = new QVBoxLayout(allBooksTab);
    m_allBooksTable = new QTableWidget();
    m_allBooksTable->setColumnCount(6);
    m_allBooksTable->setHorizontalHeaderLabels(
        QStringList() << "Title" << "Author" << "ISBN" << "Number of Copies" << "Status" << "Borrower(s)"
        );
    m_allBooksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_allBooksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_allBooksTable->verticalHeader()->setVisible(false);
    m_allBooksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    allBooksLayout->addWidget(m_allBooksTable);

    // Available Books tab
    QWidget *availableTab = new QWidget();
    QVBoxLayout *availableLayout = new QVBoxLayout(availableTab);
    m_availableBooksTable = new QTableWidget();
    m_availableBooksTable->setColumnCount(5);
    m_availableBooksTable->setHorizontalHeaderLabels(
        QStringList() << "Title" << "Author" << "ISBN" << "Number of Copies" << "Status"
        );
    m_availableBooksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_availableBooksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_availableBooksTable->verticalHeader()->setVisible(false);
    m_availableBooksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    availableLayout->addWidget(m_availableBooksTable);

    // Borrowed Books tab
    QWidget *borrowedTab = new QWidget();
    QVBoxLayout *borrowedLayout = new QVBoxLayout(borrowedTab);
    m_borrowedBooksTable = new QTableWidget();
    m_borrowedBooksTable->setColumnCount(6);
    m_borrowedBooksTable->setHorizontalHeaderLabels(
        QStringList() << "Title" << "Author" << "ISBN" << "Number of Copies" << "Status" << "Borrower(s)"
        );
    m_borrowedBooksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_borrowedBooksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_borrowedBooksTable->verticalHeader()->setVisible(false);
    m_borrowedBooksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    borrowedLayout->addWidget(m_borrowedBooksTable);

    // Add tabs
    m_tabWidget->addTab(allBooksTab, style()->standardIcon(QStyle::SP_FileDialogDetailedView), "All Books");
    m_tabWidget->addTab(availableTab, style()->standardIcon(QStyle::SP_DialogApplyButton), "Available");
    m_tabWidget->addTab(borrowedTab, style()->standardIcon(QStyle::SP_DialogCancelButton), "Borrowed");

    // Splitter to separate form + tabs
    QSplitter *bookSplitter = new QSplitter(Qt::Vertical);
    bookSplitter->addWidget(m_bookInputForm);
    bookSplitter->addWidget(m_tabWidget);
    bookSplitter->setStretchFactor(0, 1);
    bookSplitter->setStretchFactor(1, 3);
    
    bookLayout->addLayout(searchLayout);
    bookLayout->addWidget(bookSplitter, 1);
    
    // Student Management Tab
    QWidget *studentManagementTab = new QWidget();
    QVBoxLayout *studentLayout = new QVBoxLayout(studentManagementTab);
    
    // Search area for students
    QHBoxLayout *studentSearchLayout = new QHBoxLayout();
    m_studentSearchInput = new QLineEdit();
    m_studentSearchInput->setPlaceholderText("Search students...");
    QLabel *studentSearchIcon = new QLabel();
    studentSearchIcon->setPixmap(style()->standardPixmap(QStyle::SP_FileDialogContentsView));
    
    // Add student search type dropdown
    m_studentSearchTypeCombo = new QComboBox();
    m_studentSearchTypeCombo->addItem("All Fields", "all");
    m_studentSearchTypeCombo->addItem("Name", "name");
    m_studentSearchTypeCombo->addItem("ID", "id");
    m_studentSearchTypeCombo->addItem("Email", "email");
    m_studentSearchTypeCombo->setCurrentIndex(0);
    
    // Add year and group filters
    m_studentYearFilterCombo = new QComboBox();
    m_studentYearFilterCombo->addItem("All Years", "");
    for (int i = 1; i <= 5; i++) {
        m_studentYearFilterCombo->addItem(QString("Year %1").arg(i), QString::number(i));
    }
    
    m_studentGroupFilterCombo = new QComboBox();
    m_studentGroupFilterCombo->addItem("All Groups", "");
    // Groups will be populated when year filter changes
    
    studentSearchLayout->addWidget(studentSearchIcon);
    studentSearchLayout->addWidget(m_studentSearchTypeCombo);
    studentSearchLayout->addWidget(m_studentSearchInput, 1);
    studentSearchLayout->addWidget(new QLabel("Year:"));
    studentSearchLayout->addWidget(m_studentYearFilterCombo);
    studentSearchLayout->addWidget(new QLabel("Group:"));
    studentSearchLayout->addWidget(m_studentGroupFilterCombo);
    
    // Group box for student details
    QGroupBox *studentInputGroupBox = new QGroupBox("Student Details");
    QGraphicsDropShadowEffect *studentShadowEffect = new QGraphicsDropShadowEffect();
    studentShadowEffect->setBlurRadius(10);
    studentShadowEffect->setColor(QColor(0, 0, 0, 120));
    studentShadowEffect->setOffset(0, 2);
    studentInputGroupBox->setGraphicsEffect(studentShadowEffect);
    
    QGridLayout *studentInputLayout = new QGridLayout(studentInputGroupBox);
    m_studentNameInput = new QLineEdit();
    m_studentIdInput = new QLineEdit();
    m_studentEmailInput = new QLineEdit();
    m_studentDobInput = new QDateEdit(QDate::currentDate());
    m_studentYearComboBox = new QComboBox();
    m_studentGroupComboBox = new QComboBox();
    
    m_studentNameInput->setPlaceholderText("Enter student's name");
    m_studentIdInput->setPlaceholderText("Enter student's ID");
    m_studentEmailInput->setPlaceholderText("Enter student's email");
    m_studentDobInput->setDisplayFormat("yyyy-MM-dd");
    m_studentDobInput->setCalendarPopup(true);
    
    m_studentYearComboBox->addItems(QStringList() << "1" << "2" << "3" << "4");
    
    studentInputLayout->addWidget(new QLabel("Name:"), 0, 0);
    studentInputLayout->addWidget(m_studentNameInput, 0, 1);
    studentInputLayout->addWidget(new QLabel("ID:"), 1, 0);
    studentInputLayout->addWidget(m_studentIdInput, 1, 1);
    studentInputLayout->addWidget(new QLabel("Email:"), 2, 0);
    studentInputLayout->addWidget(m_studentEmailInput, 2, 1);
    studentInputLayout->addWidget(new QLabel("Date of Birth:"), 3, 0);
    studentInputLayout->addWidget(m_studentDobInput, 3, 1);
    studentInputLayout->addWidget(new QLabel("Year:"), 4, 0);
    studentInputLayout->addWidget(m_studentYearComboBox, 4, 1);
    studentInputLayout->addWidget(new QLabel("Group:"), 5, 0);
    studentInputLayout->addWidget(m_studentGroupComboBox, 5, 1);
    
    // Initial population of groups based on default year
    populateGroupComboBox();
    
    // Action buttons for students
    QHBoxLayout *studentButtonLayout = new QHBoxLayout();
    m_addStudentButton = new QPushButton("Add Student");
    m_editStudentButton = new QPushButton("Edit Student");
    m_deleteStudentButton = new QPushButton("Delete Student");
    m_clearStudentButton = new QPushButton("Clear");
    
    // Optional icons
    m_addStudentButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_editStudentButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    m_deleteStudentButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    m_clearStudentButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
    
    studentButtonLayout->addWidget(m_addStudentButton);
    studentButtonLayout->addWidget(m_editStudentButton);
    studentButtonLayout->addWidget(m_deleteStudentButton);
    studentButtonLayout->addWidget(m_clearStudentButton);
    studentButtonLayout->addStretch();
    
    // Combine student input form + action buttons
    m_studentInputForm = new QWidget();
    QVBoxLayout *studentFormLayout = new QVBoxLayout(m_studentInputForm);
    studentFormLayout->addWidget(studentInputGroupBox);
    studentFormLayout->addLayout(studentButtonLayout);
    studentFormLayout->addStretch();
    
    // Tab widget for Students / Borrow History
    m_studentTabWidget = new QTabWidget();
    
    // All Students tab
    QWidget *allStudentsTab = new QWidget();
    QVBoxLayout *allStudentsLayout = new QVBoxLayout(allStudentsTab);
    m_allStudentsTable = new QTableWidget();
    m_allStudentsTable->setColumnCount(7);
    m_allStudentsTable->setHorizontalHeaderLabels(
        QStringList() << "Name" << "ID" << "Email" << "DOB" << "Year" << "Group" << "Active Borrows"
        );
    m_allStudentsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_allStudentsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_allStudentsTable->verticalHeader()->setVisible(false);
    m_allStudentsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    allStudentsLayout->addWidget(m_allStudentsTable);
    
    // Student Borrow History tab
    QWidget *borrowHistoryTab = new QWidget();
    QVBoxLayout *borrowHistoryLayout = new QVBoxLayout(borrowHistoryTab);
    m_studentBorrowHistoryTable = new QTableWidget();
    m_studentBorrowHistoryTable->setColumnCount(5);
    m_studentBorrowHistoryTable->setHorizontalHeaderLabels(
        QStringList() << "Book Title" << "Author" << "ISBN" << "Borrow Date" << "Return Status"
        );
    m_studentBorrowHistoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_studentBorrowHistoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_studentBorrowHistoryTable->verticalHeader()->setVisible(false);
    m_studentBorrowHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    borrowHistoryLayout->addWidget(m_studentBorrowHistoryTable);
    
    // Add tabs to student tabwidget
    m_studentTabWidget->addTab(allStudentsTab, style()->standardIcon(QStyle::SP_FileDialogDetailedView), "All Students");
    m_studentTabWidget->addTab(borrowHistoryTab, style()->standardIcon(QStyle::SP_FileDialogInfoView), "Borrow History");
    
    // Splitter to separate student form + tabs
    QSplitter *studentSplitter = new QSplitter(Qt::Vertical);
    studentSplitter->addWidget(m_studentInputForm);
    studentSplitter->addWidget(m_studentTabWidget);
    studentSplitter->setStretchFactor(0, 1);
    studentSplitter->setStretchFactor(1, 3);
    
    studentLayout->addLayout(studentSearchLayout);
    studentLayout->addWidget(studentSplitter, 1);
    
    // Admin Management Tab (only visible for root admin)
    QWidget *adminManagementTab = new QWidget();
    QVBoxLayout *adminLayout = new QVBoxLayout(adminManagementTab);
    
    // Admin action log
    QGroupBox *adminLogGroupBox = new QGroupBox("Admin Action Log");
    QVBoxLayout *adminLogLayout = new QVBoxLayout(adminLogGroupBox);
    
    m_adminActionTable = new QTableWidget();
    m_adminActionTable->setColumnCount(5);
    m_adminActionTable->setHorizontalHeaderLabels(
        QStringList() << "Timestamp" << "Admin ID" << "Admin Name" << "Action Type" << "Details"
        );
    m_adminActionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_adminActionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_adminActionTable->verticalHeader()->setVisible(false);
    m_adminActionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    adminLogLayout->addWidget(m_adminActionTable);
    
    // Admin list (only for root admin)
    QGroupBox *adminListGroupBox = new QGroupBox("Admin List");
    QVBoxLayout *adminListLayout = new QVBoxLayout(adminListGroupBox);
    
    m_adminListTable = new QTableWidget();
    m_adminListTable->setColumnCount(2);
    m_adminListTable->setHorizontalHeaderLabels(
        QStringList() << "Admin ID" << "Admin Name"
        );
    m_adminListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_adminListTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_adminListTable->verticalHeader()->setVisible(false);
    m_adminListTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    QHBoxLayout *adminButtonLayout = new QHBoxLayout();
    m_addAdminButton = new QPushButton("Add Admin");
    m_deleteAdminButton = new QPushButton("Delete Admin");
    
    m_addAdminButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    m_deleteAdminButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    
    adminButtonLayout->addWidget(m_addAdminButton);
    adminButtonLayout->addWidget(m_deleteAdminButton);
    adminButtonLayout->addStretch();
    
    adminListLayout->addWidget(m_adminListTable);
    adminListLayout->addLayout(adminButtonLayout);
    
    adminLayout->addWidget(adminLogGroupBox);
    adminLayout->addWidget(adminListGroupBox);
    
    // Add tabs to main tabwidget
    mainTabWidget->addTab(bookManagementTab, style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Book Management");
    mainTabWidget->addTab(studentManagementTab, style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Student Management");
    mainTabWidget->addTab(adminManagementTab, style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Admin Management");
    
    // Add all components to main layout
    mainLayout->addLayout(adminProfileLayout);
    mainLayout->addWidget(mainTabWidget, 1);
    
    // Add both screens to the stacked widget
    m_stackedWidget->addWidget(m_loginScreen);
    m_stackedWidget->addWidget(m_mainInterface);
    
    // Initially show the login screen
    m_stackedWidget->setCurrentWidget(m_loginScreen);
    
    setCentralWidget(m_stackedWidget);
}

void MainWindow::setupStatusBar()
{
    statusBar()->setFixedHeight(28);

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setObjectName("statusLabel");
    statusBar()->addPermanentWidget(m_statusLabel, 1);

    m_clockLabel = new QLabel();
    m_clockLabel->setObjectName("clockLabel");
    statusBar()->addPermanentWidget(m_clockLabel);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateClock);
    timer->start(1000);
    updateClock();
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Main Toolbar");
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(24, 24));

    QAction *helpAction = toolBar->addAction(style()->standardIcon(QStyle::SP_DialogHelpButton), "Help");
    connect(helpAction, &QAction::triggered, this, &MainWindow::showContextHelp);
}

void MainWindow::setupAnimations()
{
    // Fade animation for the status label
    m_statusOpacity = new QGraphicsOpacityEffect(m_statusLabel);
    m_statusOpacity->setOpacity(1.0);
    m_statusLabel->setGraphicsEffect(m_statusOpacity);
}

void MainWindow::setupConnections()
{
    // Login screen connections
    connect(m_loginButton, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
    connect(m_passwordInput, &QLineEdit::returnPressed, this, &MainWindow::onLoginClicked);
    
    // Initialize the group filter options
    updateGroupFilterOptions();
    
    // Admin profile connections
    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(m_addAdminButton, &QPushButton::clicked, this, &MainWindow::onAddAdminClicked);
    connect(m_deleteAdminButton, &QPushButton::clicked, this, &MainWindow::onDeleteAdminClicked);
    
    // Book management connections
    connect(m_addBookButton, &QPushButton::clicked, this, &MainWindow::onAddBookClicked);
    connect(m_borrowBookButton, &QPushButton::clicked, this, &MainWindow::onBorrowBookClicked);
    connect(m_returnBookButton, &QPushButton::clicked, this, &MainWindow::onReturnBookClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearInputs);
    connect(m_showBorrowerButton, &QPushButton::clicked, this, &MainWindow::onShowBorrowerInfo);
    connect(m_deleteBookButton, &QPushButton::clicked, this, &MainWindow::onDeleteBookClicked);

    // Book tables
    connect(m_allBooksTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_availableBooksTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_borrowedBooksTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);

    // Book search and tabs
    connect(m_searchInput, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_bookSearchTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int) { onSearchTextChanged(m_searchInput->text()); });
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // Book table sorting
    connect(m_allBooksTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onSortColumnClicked);
    connect(m_availableBooksTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onSortColumnClicked);
    connect(m_borrowedBooksTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onSortColumnClicked);
    
    // Student management connections
    connect(m_addStudentButton, &QPushButton::clicked, this, &MainWindow::onAddStudentClicked);
    connect(m_editStudentButton, &QPushButton::clicked, this, &MainWindow::onEditStudentClicked);
    connect(m_deleteStudentButton, &QPushButton::clicked, this, &MainWindow::onDeleteStudentClicked);
    connect(m_clearStudentButton, &QPushButton::clicked, this, &MainWindow::onClearStudentInputs);
    
    // Student table
    connect(m_allStudentsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onStudentSelectionChanged);
    
    // Student search and tabs
    connect(m_studentSearchInput, &QLineEdit::textChanged, [this](const QString &text) {
        m_currentStudentSearchText = text;
        onRefreshStudentTable();
    });
    connect(m_studentSearchTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int) { onRefreshStudentTable(); });
    connect(m_studentYearFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int index) { 
                // Update the group filter options based on the selected year
                updateGroupFilterOptions();
                onRefreshStudentTable(); 
            });
    connect(m_studentGroupFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            [this](int) { onRefreshStudentTable(); });
    connect(m_studentTabWidget, &QTabWidget::currentChanged, this, &MainWindow::onStudentTabChanged);
    
    // Student year/group relationship
    connect(m_studentYearComboBox, &QComboBox::currentTextChanged, this, &MainWindow::onStudentYearChanged);
}

// ----------------------------------------------------------
//                    BUTTON SLOTS
// ----------------------------------------------------------

void MainWindow::onAddBookClicked()
{
    QString title  = m_titleInput->text().trimmed();
    QString author = m_authorInput->text().trimmed();
    QString isbn   = m_isbnInput->text().trimmed();
    int copies     = m_copiesSpin->value();

    if (title.isEmpty() || author.isEmpty() || isbn.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please fill all book details.");
        return;
    }

    // Check duplicates
    auto allBooks = m_library.getAllBooks();
    for (auto &bk : allBooks) {
        if (bk.getISBN() == isbn.toStdString()) {
            QMessageBox::warning(this, "Duplicate ISBN", "A book with this ISBN already exists.");
            return;
        }
    }

    // Add book
    m_library.addBook(title.toStdString(), author.toStdString(), isbn.toStdString(), copies);
    onClearInputs();
    refreshBookTables();
    animateStatusMessage(QString("Book added: %1 (%2 copies)").arg(title).arg(copies));
    onRefreshAdminTable(); // Update admin history
}

void MainWindow::onBorrowBookClicked()
{
    QTableWidget *currentTable = nullptr;
    int tab = m_tabWidget->currentIndex();
    if (tab == 0)
        currentTable = m_allBooksTable;
    else if (tab == 1)
        currentTable = m_availableBooksTable;
    else {
        QMessageBox::information(this, "Info", "Borrow from All or Available tab only.");
        return;
    }

    int row = currentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select a book to borrow.");
        return;
    }

    QString isbn  = currentTable->item(row, 2)->text(); // ISBN col
    QString title = currentTable->item(row, 0)->text();
    QString status= currentTable->item(row, 4)->text();

    if (status.startsWith("All copies borrowed")) {
        QMessageBox::warning(this, "Unavailable", "All copies are borrowed.");
        return;
    }

    // Get all students from the library
    auto students = m_library.getAllStudents();
    if (students.empty()) {
        QMessageBox::warning(this, "No Students", "No students are registered in the system. Please add students first.");
        return;
    }

    // Borrower dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Select Student");
    dialog.setMinimumWidth(500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // Student selection section
    QGroupBox *studentGroupBox = new QGroupBox("Select Student");
    QVBoxLayout *studentLayout = new QVBoxLayout(studentGroupBox);
    
    // Filter students section
    QHBoxLayout *filterLayout = new QHBoxLayout();
    QLabel *yearLabel = new QLabel("Year:");
    QLabel *groupLabel = new QLabel("Group:");
    m_studentYearFilterComboBox = new QComboBox();
    m_studentGroupFilterComboBox = new QComboBox();
    m_studentYearFilterComboBox->addItem("All Years");
    m_studentYearFilterComboBox->addItems(QStringList() << "1" << "2" << "3" << "4");
    
    m_studentGroupFilterComboBox->addItem("All Groups");
    
    // Student selector
    m_studentNameComboBox = new QComboBox();
    m_studentNameComboBox->setMinimumWidth(300);
    
    // Connect year filter to update groups and student list
    connect(m_studentYearFilterComboBox, &QComboBox::currentTextChanged, [this, &students]() {
        QString selectedYear = m_studentYearFilterComboBox->currentText();
        m_studentGroupFilterComboBox->clear();
        m_studentGroupFilterComboBox->addItem("All Groups");
        
        if (selectedYear != "All Years") {
            int maxGroups = Library::getMaxGroupsForYear(selectedYear.toStdString());
            for (int i = 1; i <= maxGroups; i++) {
                m_studentGroupFilterComboBox->addItem(QString::number(i));
            }
        }
        
        // Update student list based on selected filters
        updateStudentBorrowList(students);
    });
    
    // Connect group filter to update student list
    connect(m_studentGroupFilterComboBox, &QComboBox::currentTextChanged, [this, &students]() {
        updateStudentBorrowList(students);
    });
    
    filterLayout->addWidget(yearLabel);
    filterLayout->addWidget(m_studentYearFilterComboBox);
    filterLayout->addWidget(groupLabel);
    filterLayout->addWidget(m_studentGroupFilterComboBox);
    
    studentLayout->addLayout(filterLayout);
    studentLayout->addWidget(new QLabel("Student:"));
    studentLayout->addWidget(m_studentNameComboBox);
    
    // Initially populate the student list
    updateStudentBorrowList(students);
    
    layout->addWidget(studentGroupBox);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(box);

    if (dialog.exec() == QDialog::Accepted && m_studentNameComboBox->count() > 0) {
        // Get the selected student data from the combo box (stored as user data)
        int selectedIndex = m_studentNameComboBox->currentIndex();
        if (selectedIndex >= 0) {
            QVariant userData = m_studentNameComboBox->itemData(selectedIndex);
            QStringList studentData = userData.toStringList();
            
            if (studentData.size() >= 5) {
                QString name = studentData[0];
                QString id = studentData[1];
                QString dob = studentData[2];
                QString year = studentData[3];
                QString group = studentData[4];
                
                User borrower(name.toStdString(),
                              id.toStdString(),
                              dob.toStdString(), 
                              year.toStdString(),
                              group.toStdString());

        bool success = m_library.borrowOneCopy(isbn.toStdString(), borrower);
        if (!success) {
            QMessageBox::warning(this, "Borrow Failed", "No available copy or an error occurred.");
        } else {
                    animateStatusMessage("Book borrowed: " + title + " by " + name);
                }
                refreshBookTables();
                // Also refresh student tables to update borrow counts and history
                onRefreshStudentTable();
                // Refresh borrow history if this student is currently selected
                refreshBorrowHistory(id);
        updateButtonStates();
                onRefreshAdminTable(); // Update admin history
            }
        }
    }
    onRefreshAdminTable(); // Update admin history
}

void MainWindow::updateStudentBorrowList(const std::vector<Student>& students)
{
    QString selectedYear = m_studentYearFilterComboBox->currentText();
    QString selectedGroup = m_studentGroupFilterComboBox->currentText();
    
    m_studentNameComboBox->clear();
    
    for (const auto& student : students) {
        QString studentYear = QString::fromStdString(student.getYear());
        QString studentGroup = QString::fromStdString(student.getGroup());
        
        // Apply filters
        if (selectedYear != "All Years" && studentYear != selectedYear)
            continue;
        
        if (selectedGroup != "All Groups" && studentGroup != selectedGroup)
            continue;
        
        QString name = QString::fromStdString(student.getName());
        QString id = QString::fromStdString(student.getId());
        QString dob = QString::fromStdString(student.getDob());
        
        QString displayText = QString("%1 (ID: %2, Year: %3, Group: %4)")
                             .arg(name)
                             .arg(id)
                             .arg(studentYear)
                             .arg(studentGroup);
        
        // Store all student data as user data in the combo box
        QStringList studentData;
        studentData << name << id << dob << studentYear << studentGroup;
        
        m_studentNameComboBox->addItem(displayText, studentData);
    }
}

void MainWindow::onReturnBookClicked()
{
    QTableWidget *currentTable = nullptr;
    int tab = m_tabWidget->currentIndex();
    if (tab == 0)
        currentTable = m_allBooksTable;
    else if (tab == 2)
        currentTable = m_borrowedBooksTable;
    else {
        QMessageBox::information(this, "Info", "Return from All or Borrowed tab only.");
        return;
    }

    int row = currentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Select a book to return.");
        return;
    }

    QString isbn = currentTable->item(row, 2)->text();
    QString title = currentTable->item(row, 0)->text();

    // Get borrowers list
    QString selectedBorrowerId;
    QString selectedBorrowerName;
    std::vector<Book::BorrowerInfo> bookBorrowers;
    
    for (auto &b : m_library.getAllBooks()) {
        if (b.getISBN() == isbn.toStdString()) {
            bookBorrowers = b.getBorrowers();
            break;
        }
    }
    
    if (bookBorrowers.empty()) {
        QMessageBox::warning(this, "Return Failed", "No borrowed copies found to return.");
        return;
    }
    
    // If there's only one borrower, return directly
    if (bookBorrowers.size() == 1) {
        selectedBorrowerId = QString::fromStdString(bookBorrowers[0].id);
        selectedBorrowerName = QString::fromStdString(bookBorrowers[0].name);
    } else {
        // Multiple borrowers, show a selection dialog
        QDialog dialog(this);
        dialog.setWindowTitle("Select Borrower to Return From");
        dialog.setMinimumSize(500, 300);

        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QLabel *instructionLabel = new QLabel("Multiple students have borrowed this book. Select which copy to return:");
        layout->addWidget(instructionLabel);

        QTableWidget *borrowersTable = new QTableWidget(&dialog);
        borrowersTable->setColumnCount(4);
        borrowersTable->setHorizontalHeaderLabels(QStringList() << "Name" << "ID" << "Year/Group" << "Borrow Date");
        borrowersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        borrowersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        borrowersTable->setSelectionMode(QAbstractItemView::SingleSelection);
        borrowersTable->verticalHeader()->setVisible(false);
        
        borrowersTable->setRowCount(static_cast<int>(bookBorrowers.size()));
        for (int i = 0; i < static_cast<int>(bookBorrowers.size()); ++i) {
            auto &info = bookBorrowers[i];
            borrowersTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(info.name)));
            borrowersTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(info.id)));
            borrowersTable->setItem(i, 2, new QTableWidgetItem(QString("%1/%2").arg(
                QString::fromStdString(info.year)).arg(QString::fromStdString(info.group))));
            borrowersTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(info.borrowedDate)));
        }
        
        // Select the first row by default
        borrowersTable->selectRow(0);
        
        layout->addWidget(borrowersTable);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttonBox);

        if (dialog.exec() != QDialog::Accepted) {
            return;  // User canceled
        }
        
        int selectedRow = borrowersTable->currentRow();
        if (selectedRow >= 0 && selectedRow < static_cast<int>(bookBorrowers.size())) {
            selectedBorrowerId = QString::fromStdString(bookBorrowers[selectedRow].id);
            selectedBorrowerName = QString::fromStdString(bookBorrowers[selectedRow].name);
        } else {
            QMessageBox::warning(this, "Selection Required", "Please select a borrower to return from.");
            return;
        }
    }

    // Return the book with the selected student ID
    bool success = m_library.returnOneCopy(isbn.toStdString(), selectedBorrowerId.toStdString());
    if (!success) {
        QMessageBox::warning(this, "Return Failed", "Failed to return the book.");
    } else {
        QString statusMsg = "Book returned: " + title;
        if (!selectedBorrowerName.isEmpty()) {
            statusMsg += " from " + selectedBorrowerName;
        }
        animateStatusMessage(statusMsg);
    }
    
    refreshBookTables();
    // Also refresh student tables to update borrow counts and history
    onRefreshStudentTable();
    // Refresh borrow history if this student is currently selected
    if (!selectedBorrowerId.isEmpty()) {
        refreshBorrowHistory(selectedBorrowerId);
    }
    updateButtonStates();
    onRefreshAdminTable(); // Update admin history
}

void MainWindow::onClearInputs()
{
    m_titleInput->clear();
    m_authorInput->clear();
    m_isbnInput->clear();
    m_copiesSpin->setValue(1);
    m_titleInput->setFocus();
}

void MainWindow::onDeleteBookClicked()
{
    // Allow deleting from any tab. If you'd prefer only "All Books," then restrict to tab == 0.
    QTableWidget *currentTable = nullptr;
    int tab = m_tabWidget->currentIndex();
    if (tab == 0)
        currentTable = m_allBooksTable;
    else if (tab == 1)
        currentTable = m_availableBooksTable;
    else
        currentTable = m_borrowedBooksTable;

    int row = currentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select a book to delete.");
        return;
    }

    QString isbn = currentTable->item(row, 2)->text(); // ISBN is column 2
    QString title = currentTable->item(row, 0)->text();

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete this book?\n\nTitle: %1\nISBN: %2").arg(title, isbn),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        // NOTE: You need to implement removeBook() in your Library class:
        // bool Library::removeBook(const std::string &isbn) { ... }
        bool success = m_library.removeBook(isbn.toStdString());
        if (!success) {
            QMessageBox::warning(this, "Delete Failed", "Failed to delete the book with ISBN: " + isbn);
            return;
        }
        refreshBookTables();
        animateStatusMessage("Book deleted: " + title);
    }
    onRefreshAdminTable(); // Update admin history
}

// ----------------------------------------------------------
//                    TABLE REFRESH + SEARCH
// ----------------------------------------------------------

void MainWindow::refreshBookTables()
{
    auto books = m_library.getAllBooks();

    // Clear existing rows
    m_allBooksTable->setRowCount(0);
    m_availableBooksTable->setRowCount(0);
    m_borrowedBooksTable->setRowCount(0);

    for (auto &b : books) {
        if (!filterBook(b, m_currentSearchText))
            continue;

        int total    = b.getTotalCopies();
        int borrowed = b.getBorrowedCount();

        // Build status string
        QString status;
        if (borrowed == 0) {
            status = "All copies available";
        } else if (borrowed < total) {
            int available = total - borrowed;
            status = QString("%1/%2 copies available").arg(available).arg(total);
        } else {
            status = "All copies borrowed";
        }

        // Build borrower info text
        QString borrowersText;
        if (borrowed > 0) {
            for (auto &info : b.getBorrowers()) {
                QString line = QString("%1 (ID:%2, borrowed:%3)")
                .arg(QString::fromStdString(info.name))
                    .arg(QString::fromStdString(info.id))
                    .arg(QString::fromStdString(info.borrowedDate));
                borrowersText += line + "\n";
            }
            borrowersText = borrowersText.trimmed();
        } else {
            borrowersText = "-";
        }

        // Insert row in All Books tab
        int newRowAll = m_allBooksTable->rowCount();
        m_allBooksTable->insertRow(newRowAll);
        m_allBooksTable->setItem(newRowAll, 0, new QTableWidgetItem(QString::fromStdString(b.getTitle())));
        m_allBooksTable->setItem(newRowAll, 1, new QTableWidgetItem(QString::fromStdString(b.getAuthor())));
        m_allBooksTable->setItem(newRowAll, 2, new QTableWidgetItem(QString::fromStdString(b.getISBN())));
        m_allBooksTable->setItem(newRowAll, 3, new QTableWidgetItem(QString::number(b.getTotalCopies())));
        m_allBooksTable->setItem(newRowAll, 4, new QTableWidgetItem(status));
        m_allBooksTable->setItem(newRowAll, 5, new QTableWidgetItem(borrowersText));

        // Insert row in Available tab if not fully borrowed
        if (borrowed < total) {
            int newRowAvail = m_availableBooksTable->rowCount();
            m_availableBooksTable->insertRow(newRowAvail);
            m_availableBooksTable->setItem(newRowAvail, 0, new QTableWidgetItem(QString::fromStdString(b.getTitle())));
            m_availableBooksTable->setItem(newRowAvail, 1, new QTableWidgetItem(QString::fromStdString(b.getAuthor())));
            m_availableBooksTable->setItem(newRowAvail, 2, new QTableWidgetItem(QString::fromStdString(b.getISBN())));
            m_availableBooksTable->setItem(newRowAvail, 3, new QTableWidgetItem(QString::number(b.getTotalCopies())));
            m_availableBooksTable->setItem(newRowAvail, 4, new QTableWidgetItem(status));
        }

        // Insert row in Borrowed tab if any are borrowed
        if (borrowed > 0) {
            int newRowBorrow = m_borrowedBooksTable->rowCount();
            m_borrowedBooksTable->insertRow(newRowBorrow);
            m_borrowedBooksTable->setItem(newRowBorrow, 0, new QTableWidgetItem(QString::fromStdString(b.getTitle())));
            m_borrowedBooksTable->setItem(newRowBorrow, 1, new QTableWidgetItem(QString::fromStdString(b.getAuthor())));
            m_borrowedBooksTable->setItem(newRowBorrow, 2, new QTableWidgetItem(QString::fromStdString(b.getISBN())));
            m_borrowedBooksTable->setItem(newRowBorrow, 3, new QTableWidgetItem(QString::number(b.getTotalCopies())));
            m_borrowedBooksTable->setItem(newRowBorrow, 4, new QTableWidgetItem(status));
            m_borrowedBooksTable->setItem(newRowBorrow, 5, new QTableWidgetItem(borrowersText));
        }
    }

    updateStatusMessage(QString("Library contains %1 books").arg(books.size()));
    updateButtonStates();
}

void MainWindow::onSelectionChanged()
{
    updateButtonStates();
}

void MainWindow::updateButtonStates()
{
    QTableWidget *currentTable = nullptr;
    int tab = m_tabWidget->currentIndex();
    if (tab == 0)
        currentTable = m_allBooksTable;
    else if (tab == 1)
        currentTable = m_availableBooksTable;
    else
        currentTable = m_borrowedBooksTable;

    int row = currentTable->currentRow();
    bool hasSel = (row >= 0);

    // Default: disable all
    m_borrowBookButton->setEnabled(false);
    m_returnBookButton->setEnabled(false);
    m_showBorrowerButton->setEnabled(false);
    m_deleteBookButton->setEnabled(false);

    if (hasSel) {
        QString status = currentTable->item(row, 4)->text();
        // Borrow is enabled if status != "All copies borrowed"
        if (!status.startsWith("All copies borrowed")) {
            m_borrowBookButton->setEnabled(true);
        }
        // Return is enabled if status != "All copies available"
        // (means at least 1 copy is borrowed)
        if (!status.startsWith("All copies available")) {
            m_returnBookButton->setEnabled(true);
            m_showBorrowerButton->setEnabled(true);
        }
        // Delete is always enabled if something is selected
        m_deleteBookButton->setEnabled(true);
    }
}

void MainWindow::updateStatusMessage(const QString &message)
{
    if (m_statusLabel)
        m_statusLabel->setText(message);
}

void MainWindow::animateStatusMessage(const QString &message)
{
    updateStatusMessage(message);

    // Fade in, pause, fade out
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(this);

    QPropertyAnimation *fadeIn = new QPropertyAnimation(m_statusOpacity, "opacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    QPauseAnimation *pause = new QPauseAnimation(3000, this);

    QPropertyAnimation *fadeOut = new QPropertyAnimation(m_statusOpacity, "opacity");
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    group->addAnimation(fadeIn);
    group->addAnimation(pause);
    group->addAnimation(fadeOut);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::updateClock()
{
        m_clockLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void MainWindow::showHelp()
{
    // Reuse the existing contextual help implementation
    showContextHelp();
}

void MainWindow::showContextHelp()
{
    QString helpTitle = "ENSIARY Help";
    QString helpText;
    
    // First check if we're in login screen or main interface
    if (m_stackedWidget->currentWidget() == m_loginScreen) {
        helpTitle = "Login Help";
        helpText = "Welcome to ENSIARY Library Management System.\n\n"
                   "To log in, enter your administrator credentials:\n"
                   "- Default admin: 'root'\n"
                   "- Default password: 'root'\n\n"
                   "After login, you'll have access to the full system functionality.";
    } else {
        // We're in the main interface, check which tab is active
        QTabWidget* mainTabWidget = qobject_cast<QTabWidget*>(m_mainInterface->layout()->itemAt(1)->widget());
        if (mainTabWidget) {
            int currentMainTab = mainTabWidget->currentIndex();
            
            if (currentMainTab == 0) { // Book Management
                helpTitle = "Book Management Help";
                int bookSubTab = m_tabWidget->currentIndex();
                
                if (bookSubTab == 0) { // All Books
                    helpText = "All Books Tab:\n\n"
                               "• View all books in the library\n"
                               "• Search books by title, author, or ISBN using the search bar\n"
                               "• Add new books using the form at the top\n"
                               "• Select a book to borrow, return, or delete it\n"
                               "• View borrower information for borrowed books";
                } else if (bookSubTab == 1) { // Available Books
                    helpText = "Available Books Tab:\n\n"
                               "• View books that have at least one copy available\n"
                               "• Search books by title, author, or ISBN using the search bar\n"
                               "• Select a book to borrow it to a student";
                } else { // Borrowed Books
                    helpText = "Borrowed Books Tab:\n\n"
                               "• View books that have at least one copy borrowed\n"
                               "• Search books by title, author, or ISBN using the search bar\n"
                               "• Select a book to return it or view borrower information";
                }
            } else if (currentMainTab == 1) { // Student Management
                helpTitle = "Student Management Help";
                int studentSubTab = m_studentTabWidget->currentIndex();
                
                if (studentSubTab == 0) { // All Students
                    helpText = "All Students Tab:\n\n"
                               "• View all students in the system\n"
                               "• Search students by name, ID, or email\n"
                               "• Filter students by year and group\n"
                               "• Add new students using the form at the top\n"
                               "• Select a student to edit or delete\n"
                               "• View how many books each student has borrowed";
                } else { // Borrow History
                    helpText = "Borrow History Tab:\n\n"
                               "• View borrowing history for the selected student\n"
                               "• See which books are currently borrowed and which have been returned\n"
                               "• First select a student in the All Students tab to view their history";
                }
            } else if (currentMainTab == 2) { // Admin Management
                helpTitle = "Admin Management Help";
                helpText = "Admin Management Tab:\n\n"
                           "• View system admin accounts (root admin only)\n"
                           "• Add or delete admin accounts (root admin only)\n"
                           "• View admin action logs to track activity\n"
                           "• Only the root admin can see and manage other admin accounts";
            }
        }
    }
    
    QMessageBox::information(this, helpTitle, helpText);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_currentSearchText = text;
    refreshBookTables();
}

bool MainWindow::filterBook(const Book &book, const QString &searchText)
{
    if (searchText.isEmpty() || searchText.trimmed().isEmpty())
        return true;
        
    QString trimmedSearch = searchText.trimmed().toLower();
    
    QString title = QString::fromStdString(book.getTitle()).toLower();
    QString author = QString::fromStdString(book.getAuthor()).toLower();
    QString isbn = QString::fromStdString(book.getISBN()).toLower();
    
    // Get the selected search type
    QString searchType = m_bookSearchTypeCombo->currentData().toString();
    
    if (searchType == "title") {
        return title.contains(trimmedSearch);
    } else if (searchType == "author") {
        return author.contains(trimmedSearch);
    } else if (searchType == "isbn") {
        return isbn.contains(trimmedSearch);
    } else {
        // "all" or any other value - search all fields
        return (title.contains(trimmedSearch) || 
                author.contains(trimmedSearch) || 
                isbn.contains(trimmedSearch));
    }
}

void MainWindow::onTabChanged(int /*index*/)
{
    m_allBooksTable->clearSelection();
    m_availableBooksTable->clearSelection();
    m_borrowedBooksTable->clearSelection();
    updateButtonStates();
}

void MainWindow::onSortColumnClicked(int column)
{
    // Toggle sort order if same column
    m_sortAscending = (m_sortColumn == column) ? !m_sortAscending : true;
    m_sortColumn = column;

    QTableWidget *currentTable = nullptr;
    int tab = m_tabWidget->currentIndex();
    if (tab == 0)
        currentTable = m_allBooksTable;
    else if (tab == 1)
        currentTable = m_availableBooksTable;
    else
        currentTable = m_borrowedBooksTable;

    if (currentTable)
        currentTable->sortItems(m_sortColumn, m_sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

void MainWindow::onShowBorrowerInfo()
{
    QTableWidget *currentTable = nullptr;
    int tab = m_tabWidget->currentIndex();
    if (tab == 0)
        currentTable = m_allBooksTable;
    else if (tab == 1)
        currentTable = m_availableBooksTable;
    else
        currentTable = m_borrowedBooksTable;

    int row = currentTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Select a borrowed book first.");
        return;
    }

    QString isbn = currentTable->item(row, 2)->text(); // ISBN col
    auto books = m_library.getAllBooks();

    for (auto &b : books) {
        if (b.getISBN() == isbn.toStdString()) {
            if (b.getBorrowedCount() == 0) {
                QMessageBox::information(this, "No Borrowers", "No copies are currently borrowed.");
                return;
            }
            // Borrower info dialog
            QDialog dialog(this);
            dialog.setWindowTitle("Borrower Information");
            dialog.setMinimumSize(500, 300);

            QVBoxLayout *layout = new QVBoxLayout(&dialog);

            QTableWidget *table = new QTableWidget(&dialog);
            table->setColumnCount(5);
            table->setHorizontalHeaderLabels(QStringList()
                                             << "Name" << "ID" << "Year" << "Group" << "Borrowed Date");
            table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            table->verticalHeader()->setVisible(false);
            table->setSortingEnabled(true);

            auto borrowers = b.getBorrowers();
            table->setRowCount(static_cast<int>(borrowers.size()));
            for (int i = 0; i < static_cast<int>(borrowers.size()); ++i) {
                auto &info = borrowers[i];
                table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(info.name)));
                table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(info.id)));
                table->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(info.year)));
                table->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(info.group)));
                table->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(info.borrowedDate)));
            }
            layout->addWidget(table);

            QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
            connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
            layout->addWidget(box);

            dialog.exec();
            return;
        }
    }
}

// ----------------------------------------------------------
//                  STUDENT MANAGEMENT
// ----------------------------------------------------------

void MainWindow::onAddStudentClicked()
{
    QString name = m_studentNameInput->text().trimmed();
    QString id = m_studentIdInput->text().trimmed();
    QString email = m_studentEmailInput->text().trimmed();
    QDate dob = m_studentDobInput->date();
    QString year = m_studentYearComboBox->currentText();
    QString group = m_studentGroupComboBox->currentText();

    if (name.isEmpty() || id.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please fill all student details.");
        return;
    }

    // Check if year and group combination is valid
    std::string errorMsg;
    if (!Library::isValidStudentData(year.toStdString(), group.toStdString(), errorMsg)) {
        QMessageBox::warning(this, "Invalid Data", QString::fromStdString(errorMsg));
        return;
    }

    // Check if student already exists
    auto allStudents = m_library.getAllStudents();
    for (auto &student : allStudents) {
        if (student.getId() == id.toStdString()) {
            QMessageBox::warning(this, "Duplicate ID", "A student with this ID already exists.");
            return;
        }
    }

    // Add student
    Student newStudent(name.toStdString(), 
                     id.toStdString(), 
                     dob.toString("yyyy-MM-dd").toStdString(), 
                     year.toStdString(), 
                     group.toStdString(),
                     email.toStdString());
    
    bool success = m_library.addStudent(newStudent);
    
    if (success) {
        onClearStudentInputs();
        onRefreshStudentTable();
        animateStatusMessage(QString("Student added: %1 (ID: %2)").arg(name).arg(id));
        onRefreshAdminTable(); // Update admin history
    } else {
        QMessageBox::warning(this, "Error", "Failed to add student to the database.");
    }
}

void MainWindow::onEditStudentClicked()
{
    // First check if any student is selected
    int row = m_allStudentsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select a student to edit first.");
        return;
    }
    
    // Get the original student ID (which we won't allow to be changed)
    QString originalId = m_allStudentsTable->item(row, 1)->text();
    
    // Get the student data from the form
    QString name = m_studentNameInput->text().trimmed();
    QString id = m_studentIdInput->text().trimmed();
    QString email = m_studentEmailInput->text().trimmed();
    QDate dob = m_studentDobInput->date();
    QString year = m_studentYearComboBox->currentText();
    QString group = m_studentGroupComboBox->currentText();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Student name cannot be empty.");
        return;
    }

    // If user tried to change the ID, warn them it's not allowed
    if (id != originalId) {
        QMessageBox::warning(this, "ID Cannot Be Changed", 
            "Student ID cannot be changed. Other details have been updated.");
        
        // Update the ID field back to the original value
        m_studentIdInput->setText(originalId);
        id = originalId;
    }

    // Check if year and group combination is valid
    std::string errorMsg;
    if (!Library::isValidStudentData(year.toStdString(), group.toStdString(), errorMsg)) {
        QMessageBox::warning(this, "Invalid Data", QString::fromStdString(errorMsg));
        return;
    }

    // Confirm edit
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Edit",
        QString("Are you sure you want to update this student?\n\nName: %1\nID: %2")
            .arg(name).arg(id),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // Update student
    Student updatedStudent(
        name.toStdString(), 
        id.toStdString(), 
        dob.toString("yyyy-MM-dd").toStdString(), 
        year.toStdString(), 
        group.toStdString(),
        email.toStdString()
    );
    
    bool success = m_library.updateStudent(updatedStudent);
    
    if (success) {
        onRefreshStudentTable();
        
        // Also update the borrow history for this student
        refreshBorrowHistory(id);
        
        animateStatusMessage(QString("Student updated: %1 (ID: %2)").arg(name).arg(id));
        onRefreshAdminTable(); // Update admin history
    } else {
        QMessageBox::warning(this, "Error", "Failed to update student information.");
    }
}

void MainWindow::onDeleteStudentClicked()
{
    int row = m_allStudentsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select a student to delete.");
        return;
    }

    QString id = m_allStudentsTable->item(row, 1)->text();  // Assuming ID is in column 1
    QString name = m_allStudentsTable->item(row, 0)->text(); // Assuming Name is in column 0

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete this student?\n\nName: %1\nID: %2").arg(name, id),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        bool success = m_library.removeStudent(id.toStdString());
        if (!success) {
            QMessageBox::warning(this, "Delete Failed", 
                               "Failed to delete the student. The student may have active borrows.");
            return;
        }
        onClearStudentInputs();
        onRefreshStudentTable();
        animateStatusMessage("Student deleted: " + name);
        onRefreshAdminTable(); // Update admin history
    }
}

void MainWindow::onClearStudentInputs()
{
    m_studentNameInput->clear();
    m_studentIdInput->clear();
    m_studentEmailInput->clear();
    m_studentDobInput->setDate(QDate::currentDate());
    m_studentYearComboBox->setCurrentIndex(0);
    populateGroupComboBox(); // Reset groups based on year
    m_studentNameInput->setFocus();
}

void MainWindow::onStudentSelectionChanged()
{
    updateStudentButtonStates();
    
    // Get the selected student's ID
    int row = m_allStudentsTable->currentRow();
    if (row < 0) {
        // No selection, clear the borrow history table
        refreshBorrowHistory("");
        return;
    }
    
    // Populate form fields with selected student data
    QString id = m_allStudentsTable->item(row, 1)->text();     // ID
    QString name = m_allStudentsTable->item(row, 0)->text();   // Name
    QString email = m_allStudentsTable->item(row, 2)->text();  // Email
    QString dob = m_allStudentsTable->item(row, 3)->text();    // DOB
    QString year = m_allStudentsTable->item(row, 4)->text();   // Year
    QString group = m_allStudentsTable->item(row, 5)->text();  // Group

    m_studentNameInput->setText(name);
    m_studentIdInput->setText(id);
    m_studentEmailInput->setText(email);
    m_studentDobInput->setDate(QDate::fromString(dob, "yyyy-MM-dd"));
    
    int yearIndex = m_studentYearComboBox->findText(year);
    if (yearIndex >= 0) {
        m_studentYearComboBox->setCurrentIndex(yearIndex);
    }
    
    int groupIndex = m_studentGroupComboBox->findText(group);
    if (groupIndex >= 0) {
        m_studentGroupComboBox->setCurrentIndex(groupIndex);
    }
    
    // Refresh the borrow history for this student
    refreshBorrowHistory(id);
}

void MainWindow::onStudentTabChanged(int index)
{
    if (index == 0) {
        // Student list tab
        m_allStudentsTable->clearSelection();
        updateStudentButtonStates();
        
        // Make search area visible when on the Students tab
        if (m_studentSearchInput && m_studentSearchTypeCombo) {
            m_studentSearchInput->setVisible(true);
            m_studentSearchTypeCombo->setVisible(true);
            m_studentYearFilterCombo->setVisible(true);
            m_studentGroupFilterCombo->setVisible(true);
            
            // Also show the labels for year and group filters
            QLayout* searchLayout = m_studentSearchInput->parentWidget()->layout();
            for (int i = 0; i < searchLayout->count(); i++) {
                QWidget* w = searchLayout->itemAt(i)->widget();
                if (w && w->inherits("QLabel")) {
                    w->setVisible(true);
                }
            }
        }
    } else {
        // Borrow history tab
        // Hide search area when on the Borrow History tab
        if (m_studentSearchInput && m_studentSearchTypeCombo) {
            m_studentSearchInput->setVisible(false);
            m_studentSearchTypeCombo->setVisible(false);
            m_studentYearFilterCombo->setVisible(false);
            m_studentGroupFilterCombo->setVisible(false);
            
            // Also hide the labels for year and group filters
            QLayout* searchLayout = m_studentSearchInput->parentWidget()->layout();
            for (int i = 0; i < searchLayout->count(); i++) {
                QWidget* w = searchLayout->itemAt(i)->widget();
                if (w && w->inherits("QLabel")) {
                    w->setVisible(false);
                }
            }
        }
    }
}

void MainWindow::onStudentYearChanged(const QString &year)
{
    populateGroupComboBox();
}

void MainWindow::populateGroupComboBox()
{
    QString year = m_studentYearComboBox->currentText();
    m_studentGroupComboBox->clear();
    
    int maxGroups = Library::getMaxGroupsForYear(year.toStdString());
    for (int i = 1; i <= maxGroups; i++) {
        m_studentGroupComboBox->addItem(QString::number(i));
    }
}

void MainWindow::onRefreshStudentTable()
{
    qDebug("onRefreshStudentTable called - loading students...");
    auto students = m_library.getAllStudents();
    
    m_allStudentsTable->clearContents();
    m_allStudentsTable->setRowCount(0);
    
    for (auto &student : students) {
        if (!filterStudent(student, m_currentStudentSearchText))
            continue;
            
        int newRow = m_allStudentsTable->rowCount();
        m_allStudentsTable->insertRow(newRow);
        
        m_allStudentsTable->setItem(newRow, 0, new QTableWidgetItem(QString::fromStdString(student.getName())));
        m_allStudentsTable->setItem(newRow, 1, new QTableWidgetItem(QString::fromStdString(student.getId())));
        m_allStudentsTable->setItem(newRow, 2, new QTableWidgetItem(QString::fromStdString(student.getEmail())));
        m_allStudentsTable->setItem(newRow, 3, new QTableWidgetItem(QString::fromStdString(student.getDob())));
        m_allStudentsTable->setItem(newRow, 4, new QTableWidgetItem(QString::fromStdString(student.getYear())));
        m_allStudentsTable->setItem(newRow, 5, new QTableWidgetItem(QString::fromStdString(student.getGroup())));
        
        // Count active borrows - get a fresh copy from the database to ensure it's updated
        Student freshStudent = m_library.getStudent(student.getId());
        std::vector<Student::BorrowRecord> activeBorrows = freshStudent.getActiveBorrows();
        m_allStudentsTable->setItem(newRow, 6, new QTableWidgetItem(QString::number(activeBorrows.size())));
    }
    
    qDebug("Student table updated with %d students", students.size());
    updateStatusMessage(QString("Library contains %1 students").arg(students.size()));
    updateStudentButtonStates();
}

void MainWindow::updateStudentButtonStates()
{
    int row = m_allStudentsTable->currentRow();
    bool hasSelection = (row >= 0);
    
    m_editStudentButton->setEnabled(hasSelection);
    m_deleteStudentButton->setEnabled(hasSelection);
}

bool MainWindow::filterStudent(const Student &student, const QString &searchText)
{
    // Skip empty searches
    if (searchText.isEmpty() || searchText.trimmed().isEmpty()) {
        // Still apply year/group filters even if search text is empty
        QString selectedYear = m_studentYearFilterCombo->currentData().toString();
        QString selectedGroup = m_studentGroupFilterCombo->currentData().toString();
        
        if (!selectedYear.isEmpty() && QString::fromStdString(student.getYear()) != selectedYear) {
            return false;
        }
        
        if (!selectedGroup.isEmpty() && QString::fromStdString(student.getGroup()) != selectedGroup) {
            return false;
        }
        
        return true;
    }
        
    QString trimmedSearch = searchText.trimmed().toLower();
    
    QString name = QString::fromStdString(student.getName()).toLower();
    QString id = QString::fromStdString(student.getId()).toLower();
    QString email = QString::fromStdString(student.getEmail()).toLower();
    QString year = QString::fromStdString(student.getYear()).toLower();
    QString group = QString::fromStdString(student.getGroup()).toLower();
    
    // Apply year/group filters first
    QString selectedYear = m_studentYearFilterCombo->currentData().toString();
    QString selectedGroup = m_studentGroupFilterCombo->currentData().toString();
    
    if (!selectedYear.isEmpty() && year != selectedYear) {
        return false;
    }
    
    if (!selectedGroup.isEmpty() && group != selectedGroup) {
        return false;
    }
    
    // Then apply text search
    QString searchType = m_studentSearchTypeCombo->currentData().toString();
    
    if (searchType == "name") {
        return name.contains(trimmedSearch);
    } else if (searchType == "id") {
        return id.contains(trimmedSearch);
    } else if (searchType == "email") {
        return email.contains(trimmedSearch);
    } else {
        // "all" or any other value - search all fields
        return (name.contains(trimmedSearch) || 
                id.contains(trimmedSearch) ||
                email.contains(trimmedSearch) ||
                year.contains(trimmedSearch) ||
                group.contains(trimmedSearch));
    }
}

// ----------------------------------------------------------
//                  ADMIN AUTHENTICATION
// ----------------------------------------------------------

void MainWindow::onLoginClicked()
{
    QString username = m_usernameInput->text().trimmed();
    QString password = m_passwordInput->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        m_loginStatusLabel->setText("Please enter both username and password");
        return;
    }
    
    bool success = m_library.authenticateAdmin(username.toStdString(), password.toStdString());
    
    if (success) {
        showMainInterface();
        updateAdminProfileDisplay();
    } else {
        m_loginStatusLabel->setText("Invalid username or password");
        m_passwordInput->clear();
    }
}

void MainWindow::onLogoutClicked()
{
    m_library.logoutCurrentAdmin();
    showLoginScreen();
    m_usernameInput->clear();
    m_passwordInput->clear();
    m_loginStatusLabel->clear();
}

void MainWindow::onAddAdminClicked()
{
    if (!m_library.getCurrentAdmin().isRoot()) {
        QMessageBox::warning(this, "Permission Denied", "Only the root admin can add new admins.");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Add New Admin");
    dialog.setMinimumWidth(300);
    
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QFormLayout *formLayout = new QFormLayout();
    
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QLineEdit *idEdit = new QLineEdit(&dialog);
    QLineEdit *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    formLayout->addRow("Name:", nameEdit);
    formLayout->addRow("ID:", idEdit);
    formLayout->addRow("Password:", passwordEdit);
    layout->addLayout(formLayout);
    
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(box);
    connect(box, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        QString id = idEdit->text().trimmed();
        QString password = passwordEdit->text();
        
        if (name.isEmpty() || id.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Missing Information", "Please fill all admin details.");
            return;
        }
        
        Admin newAdmin(name.toStdString(), id.toStdString(), password.toStdString());
        bool success = m_library.addAdmin(newAdmin);
        
        if (success) {
            onRefreshAdminTable();
            animateStatusMessage(QString("Admin added: %1 (ID: %2)").arg(name).arg(id));
        } else {
            QMessageBox::warning(this, "Error", "Failed to add admin. The ID may already exist.");
        }
    }
}

void MainWindow::onDeleteAdminClicked()
{
    if (!m_library.getCurrentAdmin().isRoot()) {
        QMessageBox::warning(this, "Permission Denied", "Only the root admin can delete admins.");
        return;
    }
    
    int row = m_adminListTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Selection Required", "Please select an admin to delete.");
        return;
    }
    
    QString id = m_adminListTable->item(row, 0)->text();
    QString name = m_adminListTable->item(row, 1)->text();
    
    if (id == "root") {
        QMessageBox::warning(this, "Permission Denied", "The root admin cannot be deleted.");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        QString("Are you sure you want to delete this admin?\n\nName: %1\nID: %2").arg(name, id),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        bool success = m_library.removeAdmin(id.toStdString());
        
        if (success) {
            onRefreshAdminTable();
            animateStatusMessage(QString("Admin deleted: %1").arg(name));
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete admin.");
        }
    }
}

void MainWindow::showLoginScreen()
{
    if (m_stackedWidget) {
        m_stackedWidget->setCurrentWidget(m_loginScreen);
    }
}

void MainWindow::showMainInterface()
{
    if (m_stackedWidget) {
        m_stackedWidget->setCurrentWidget(m_mainInterface);
    }
}

void MainWindow::updateAdminProfileDisplay()
{
    Admin currentAdmin = m_library.getCurrentAdmin();
    QString adminName = QString::fromStdString(currentAdmin.getName());
    QString adminId = QString::fromStdString(currentAdmin.getId());
    
    if (m_adminNameLabel) {
        m_adminNameLabel->setText(QString("Welcome, %1 (%2)").arg(adminName).arg(adminId));
    }
    
    // Hide/show admin management tab based on root status
    QTabWidget* mainTabWidget = qobject_cast<QTabWidget*>(m_mainInterface->layout()->itemAt(1)->widget());
    if (mainTabWidget) {
        int adminTabIndex = 2; // Assuming admin tab is the third tab (index 2)
        
        // Hide admin tab for non-root users
        if (!currentAdmin.isRoot()) {
            if (mainTabWidget->count() > adminTabIndex) {
                mainTabWidget->setTabVisible(adminTabIndex, false);
            }
        } else {
            if (mainTabWidget->count() > adminTabIndex) {
                mainTabWidget->setTabVisible(adminTabIndex, true);
            }
        }
    }
    
    // Hide/show admin management UI components based on root status
    if (currentAdmin.isRoot()) {
        if (m_adminListTable) {
            m_adminListTable->setVisible(true);
        }
        if (m_addAdminButton) {
            m_addAdminButton->setVisible(true);
        }
        if (m_deleteAdminButton) {
            m_deleteAdminButton->setVisible(true);
        }
        
        onRefreshAdminTable();
    } else {
        if (m_adminListTable) {
            m_adminListTable->setVisible(false);
        }
        if (m_addAdminButton) {
            m_addAdminButton->setVisible(false);
        }
        if (m_deleteAdminButton) {
            m_deleteAdminButton->setVisible(false);
        }
    }
    
    // Always refresh the action log to show the current admin's actions
    onRefreshAdminTable();
}

void MainWindow::onRefreshAdminTable()
{
    Admin currentAdmin = m_library.getCurrentAdmin();
    
    // Populate admin list (if root)
    if (currentAdmin.isRoot() && m_adminListTable) {
        m_adminListTable->clearContents();
        m_adminListTable->setRowCount(0);
        
        auto admins = m_library.getAllAdmins();
        for (auto &admin : admins) {
            int newRow = m_adminListTable->rowCount();
            m_adminListTable->insertRow(newRow);
            
            m_adminListTable->setItem(newRow, 0, new QTableWidgetItem(QString::fromStdString(admin.getId())));
            m_adminListTable->setItem(newRow, 1, new QTableWidgetItem(QString::fromStdString(admin.getName())));
        }
    }
    
    // Populate action log
    if (m_adminActionTable) {
        m_adminActionTable->clearContents();
        m_adminActionTable->setRowCount(0);
        
        std::vector<Admin::ActionLog> actionLog = Admin::getActionLog();
        
        for (auto &log : actionLog) {
            // If not root, only show this admin's actions
            if (!currentAdmin.isRoot() && log.adminId != currentAdmin.getId()) {
                continue;
            }
            
            int newRow = m_adminActionTable->rowCount();
            m_adminActionTable->insertRow(newRow);
            
            QString actionType;
            switch (log.actionType) {
                case AdminActionType::LOGIN: actionType = "Login"; break;
                case AdminActionType::LOGOUT: actionType = "Logout"; break;
                case AdminActionType::ADD_BOOK: actionType = "Add Book"; break;
                case AdminActionType::REMOVE_BOOK: actionType = "Remove Book"; break;
                case AdminActionType::ADD_STUDENT: actionType = "Add Student"; break;
                case AdminActionType::UPDATE_STUDENT: actionType = "Update Student"; break;
                case AdminActionType::REMOVE_STUDENT: actionType = "Remove Student"; break;
                case AdminActionType::BORROW_BOOK: actionType = "Borrow Book"; break;
                case AdminActionType::RETURN_BOOK: actionType = "Return Book"; break;
                case AdminActionType::ADD_ADMIN: actionType = "Add Admin"; break;
                default: actionType = "Unknown"; break;
            }
            
            m_adminActionTable->setItem(newRow, 0, new QTableWidgetItem(QString::fromStdString(log.timestamp)));
            m_adminActionTable->setItem(newRow, 1, new QTableWidgetItem(QString::fromStdString(log.adminId)));
            m_adminActionTable->setItem(newRow, 2, new QTableWidgetItem(QString::fromStdString(log.adminName)));
            m_adminActionTable->setItem(newRow, 3, new QTableWidgetItem(actionType));
            m_adminActionTable->setItem(newRow, 4, new QTableWidgetItem(QString::fromStdString(log.actionDetails)));
        }
        
        // Sort by timestamp descending (newest first)
        m_adminActionTable->sortItems(0, Qt::DescendingOrder);
    }
}

bool MainWindow::checkAdminLoggedIn()
{
    Admin currentAdmin = m_library.getCurrentAdmin();
    if (currentAdmin.getId().empty()) {
        QMessageBox::warning(this, "Authentication Required", "You must be logged in to perform this action.");
        showLoginScreen();
        return false;
    }
    return true;
}

// Additional required UI setup methods 
void MainWindow::setupLoginScreen()
{
    // Setup is done in setupUI()
}

void MainWindow::setupMainInterface()
{
    // Setup is done in setupUI()
}

void MainWindow::setupBookManagement()
{
    // Setup is done in setupUI()
}

void MainWindow::setupStudentManagement()
{
    // Setup is done in setupUI()
}

void MainWindow::setupAdminProfile()
{
    // Setup is done in setupUI()
}

void MainWindow::updateDueDateDisplay()
{
    // Implementation pending
}

void MainWindow::highlightOverdueBooks()
{
    // Implementation pending
}

void MainWindow::refreshBorrowHistory(const QString &studentId)
{
    if (studentId.isEmpty()) {
        m_studentBorrowHistoryTable->clearContents();
        m_studentBorrowHistoryTable->setRowCount(0);
        return;
    }
    
    // Get fresh data from the database
    Student student = m_library.getStudent(studentId.toStdString());
    if (student.getId().empty()) {
        // No valid student found
        return;
    }
    
    // Update the borrow history table
    m_studentBorrowHistoryTable->clearContents();
    m_studentBorrowHistoryTable->setRowCount(0);
    
    const auto &borrowHistory = student.getBorrowHistory();
    for (const auto &record : borrowHistory) {
        int newRow = m_studentBorrowHistoryTable->rowCount();
        m_studentBorrowHistoryTable->insertRow(newRow);
        
        m_studentBorrowHistoryTable->setItem(newRow, 0, new QTableWidgetItem(QString::fromStdString(record.bookTitle)));
        m_studentBorrowHistoryTable->setItem(newRow, 1, new QTableWidgetItem(QString::fromStdString(record.bookAuthor)));
        m_studentBorrowHistoryTable->setItem(newRow, 2, new QTableWidgetItem(QString::fromStdString(record.bookIsbn)));
        m_studentBorrowHistoryTable->setItem(newRow, 3, new QTableWidgetItem(QString::fromStdString(record.borrowDate)));
        
        QString returnStatus = record.returnDate.empty() ? 
                              "Not returned" : 
                              QString::fromStdString(record.returnDate);
        m_studentBorrowHistoryTable->setItem(newRow, 4, new QTableWidgetItem(returnStatus));
    }
    
    // Update the currently selected student's active borrow count
    int selectedRow = m_allStudentsTable->currentRow();
    if (selectedRow >= 0) {
        QString selectedId = m_allStudentsTable->item(selectedRow, 1)->text();
        if (selectedId == studentId) {
            std::vector<Student::BorrowRecord> activeBorrows = student.getActiveBorrows();
            m_allStudentsTable->setItem(selectedRow, 6, new QTableWidgetItem(QString::number(activeBorrows.size())));
        }
    }
}

void MainWindow::refreshStudentTable()
{
    onRefreshStudentTable();
}

void MainWindow::updateGroupFilterOptions()
{
    // Save the current selection if possible
    QString currentGroup = m_studentGroupFilterCombo->currentData().toString();
    
    // Clear and repopulate the group filter
    m_studentGroupFilterCombo->clear();
    m_studentGroupFilterCombo->addItem("All Groups", "");
    
    // Get the selected year
    QString selectedYear = m_studentYearFilterCombo->currentData().toString();
    if (selectedYear.isEmpty()) {
        // If "All Years" is selected, add groups for all years
        for (int year = 1; year <= 5; year++) {
            int maxGroups = Library::getMaxGroupsForYear(QString::number(year).toStdString());
            for (int group = 1; group <= maxGroups; group++) {
                QString groupText = QString("Y%1-G%2").arg(year).arg(group);
                m_studentGroupFilterCombo->addItem(groupText, QString::number(group));
            }
        }
    } else {
        // Add groups only for the selected year
        int year = selectedYear.toInt();
        int maxGroups = Library::getMaxGroupsForYear(selectedYear.toStdString());
        for (int group = 1; group <= maxGroups; group++) {
            QString groupText = QString("Group %1").arg(group);
            m_studentGroupFilterCombo->addItem(groupText, QString::number(group));
        }
    }
    
    // Restore the previous selection if it exists in the new items
    if (!currentGroup.isEmpty()) {
        int index = m_studentGroupFilterCombo->findData(currentGroup);
        if (index >= 0) {
            m_studentGroupFilterCombo->setCurrentIndex(index);
        }
    }
}