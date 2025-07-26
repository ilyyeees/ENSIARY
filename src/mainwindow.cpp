#include "mainwindow.h"
#include "services/library_manager.h"
#include "services/persistence_service.h"
#include "models/resource.h"
#include "models/book.h"
#include "models/article.h"
#include "models/user.h"
#include "models/loan.h"
#include "models/reservation.h"
#include "dialogs/resource_dialog.h"
#include "dialogs/user_dialog.h"
#include "dialogs/user_loans_dialog.h"
#include "dialogs/reservation_management_dialog.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QAction>
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_selectedResourceId(""), m_selectedUserId(""), m_selectedLoanId("") {
    m_libraryManager = std::make_unique<LibraryManager>(this);
    m_persistenceService = std::make_unique<PersistenceService>();
    setupUI();
    setupConnections();
    setupTimer();
    loadData();
    setWindowTitle("ENSIARY - Library Management System");
    setWindowIcon(QIcon(":/icon.png"));
    resize(1200, 800);
    showMessage("Welcome to ENSIARY Library Management System!");
}
MainWindow::~MainWindow() {
    saveData();
}
void MainWindow::closeEvent(QCloseEvent* event) {
    if (confirmAction("Do you want to save your changes before closing?")) {
        saveData();
        showMessage("Data saved successfully.");
    }
    event->accept();
}
void MainWindow::setupUI() {
    QString styleSheet = R"(
        QMainWindow {
            background-color: #1e3a8a;
            color: #ffffff;
        }
        QTabWidget::pane {
            border: 2px solid #2563eb;
            background-color: #f97316;
            border-radius: 8px;
        }
        QTabBar::tab {
            background-color: #3b82f6;
            color: #ffffff;
            padding: 12px 20px;
            margin: 2px;
            border-radius: 6px;
            font-weight: bold;
            min-width: 120px;
        }
        QTabBar::tab:selected {
            background-color: #f97316;
            color: #1e3a8a;
            font-weight: bold;
        }
        QTabBar::tab:hover:!selected {
            background-color: #2563eb;
            color: #fed7aa;
        }
        QTableWidget {
            background-color: #ffffff;
            alternate-background-color: #fef3c7;
            selection-background-color: #f97316;
            selection-color: #ffffff;
            border: 2px solid #2563eb;
            border-radius: 8px;
            gridline-color: #d1d5db;
            font-size: 12px;
            color: #1e3a8a;
        }
        QTableWidget::item {
            color: #1e3a8a;
            padding: 8px;
        }
        QTableWidget::item:selected {
            background-color: #f97316;
            color: #ffffff;
        }
        QTableWidget::item:alternate {
            background-color: #fef3c7;
            color: #1e3a8a;
        }
        QHeaderView::section {
            background-color: #1e3a8a;
            color: #ffffff;
            padding: 10px;
            border: 1px solid #2563eb;
            font-weight: bold;
            font-size: 13px;
        }
        QPushButton {
            background-color: #f97316;
            color: #ffffff;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 12px;
            min-height: 30px;
        }
        QPushButton:hover {
            background-color: #ea580c;
            transform: translateY(-1px);
        }
        QPushButton:pressed {
            background-color: #c2410c;
        }
        QPushButton:disabled {
            background-color: #9ca3af;
            color: #6b7280;
        }
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #1e3a8a;
            border: 2px solid #2563eb;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: #fefefe;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
            color: #1e3a8a;
            background-color: #fed7aa;
            border-radius: 4px;
        }
        QLabel {
            color: #1e3a8a;
            font-size: 12px;
        }
        QLineEdit, QSpinBox, QDateEdit, QComboBox {
            background-color: #ffffff;
            border: 2px solid #d1d5db;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
            color: #1e3a8a;
        }
        QLineEdit:focus, QSpinBox:focus, QDateEdit:focus, QComboBox:focus {
            border-color: #f97316;
            outline: none;
        }
        QComboBox::drop-down {
            border-left: 1px solid #d1d5db;
            background-color: #f97316;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #ffffff;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #1e3a8a;
            selection-background-color: #f97316;
            selection-color: #ffffff;
            border: 1px solid #2563eb;
        }
        QToolBar {
            background-color: #1e3a8a;
            border: none;
            spacing: 5px;
            padding: 5px;
        }
        QToolButton {
            background-color: #f97316;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 8px;
            margin: 2px;
            font-weight: bold;
        }
        QToolButton:hover {
            background-color: #ea580c;
        }
        QMenuBar {
            background-color: #1e3a8a;
            color: #ffffff;
            font-weight: bold;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
        }
        QMenuBar::item:selected {
            background-color: #f97316;
            border-radius: 4px;
        }
        QMenu {
            background-color: #ffffff;
            border: 2px solid #2563eb;
            border-radius: 6px;
            color: #1e3a8a;
        }
        QMenu::item {
            padding: 8px 16px;
        }
        QMenu::item:selected {
            background-color: #fed7aa;
        }
        QStatusBar {
            background-color: #1e3a8a;
            color: #ffffff;
            font-weight: bold;
        }
        QScrollBar:vertical {
            background-color: #e5e7eb;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #f97316;
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #ea580c;
        }
        QDialog {
            background-color: #ffffff;
            color: #1e3a8a;
        }
        QMessageBox {
            background-color: #ffffff;
            color: #1e3a8a;
        }
        QMessageBox QPushButton {
            min-width: 80px;
        }
        QListWidget {
            background-color: #ffffff;
            alternate-background-color: #fef3c7;
            selection-background-color: #f97316;
            selection-color: #ffffff;
            border: 2px solid #2563eb;
            border-radius: 8px;
            font-size: 12px;
            color: #1e3a8a;
        }
        QListWidget::item {
            color: #1e3a8a;
            padding: 8px;
            border-bottom: 1px solid #e5e7eb;
        }
        QListWidget::item:selected {
            background-color: #f97316;
            color: #ffffff;
        }
        QListWidget::item:hover {
            background-color: #fed7aa;
            color: #1e3a8a;
        }
    )";
    this->setStyleSheet(styleSheet);
    setWindowTitle("ENSIARY - Library Management System");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);
    setupResourceTab();
    setupUserTab();
    setupLoanTab();
    setupInfoTab();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
}
void MainWindow::setupMenuBar() {
    m_menuBar = menuBar();
    QMenu* fileMenu = m_menuBar->addMenu("&File");
    QAction* saveAction = fileMenu->addAction("&Save Data");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveData);
    QAction* loadAction = fileMenu->addAction("&Load Data");
    loadAction->setShortcut(QKeySequence::Open);
    connect(loadAction, &QAction::triggered, this, &MainWindow::loadData);
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    QMenu* viewMenu = m_menuBar->addMenu("&View");
    QAction* refreshAction = viewMenu->addAction("&Refresh All");
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshAllData);    
    QMenu* helpMenu = m_menuBar->addMenu("&Help");
    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About ENSIARY",
                          "ENSIARY Library Management System\n"
                          "Version 1.0\n\n"
                          "A comprehensive library management solution\n"
                          "built with Qt 6 and modern C++.");
    });
}
void MainWindow::setupToolBar() {
    m_toolBar = addToolBar("Main Toolbar");
    QAction* saveToolAction = m_toolBar->addAction("Save");
    connect(saveToolAction, &QAction::triggered, this, &MainWindow::saveData);
    QAction* refreshToolAction = m_toolBar->addAction("Refresh");
    connect(refreshToolAction, &QAction::triggered, this, &MainWindow::refreshAllData);
    m_toolBar->addSeparator();
    QAction* addResourceToolAction = m_toolBar->addAction("Add Resource");
    connect(addResourceToolAction, &QAction::triggered, this, &MainWindow::onAddResourceClicked);
    QAction* addUserToolAction = m_toolBar->addAction("Add User");
    connect(addUserToolAction, &QAction::triggered, this, &MainWindow::onAddUserClicked);
}
void MainWindow::setupStatusBar() {
    m_statusBar = statusBar();
    m_statusBar->showMessage("Ready");
}
void MainWindow::setupResourceTab() {
    m_resourceTab = new QWidget();
    m_tabWidget->addTab(m_resourceTab, "Resource Management");
    QVBoxLayout* layout = new QVBoxLayout(m_resourceTab);
    QGroupBox* searchGroup = new QGroupBox("Search and Filter");
    QHBoxLayout* searchLayout = new QHBoxLayout(searchGroup);
    searchLayout->addWidget(new QLabel("Search:"));
    m_resourceSearchEdit = new QLineEdit();
    m_resourceSearchEdit->setPlaceholderText("Enter title, author, or keywords...");
    searchLayout->addWidget(m_resourceSearchEdit);
    searchLayout->addWidget(new QLabel("Category:"));
    m_resourceCategoryFilter = new QComboBox();
    m_resourceCategoryFilter->addItem("All Categories", -1);
    m_resourceCategoryFilter->addItem("Book", static_cast<int>(Resource::Category::Book));
    m_resourceCategoryFilter->addItem("Article", static_cast<int>(Resource::Category::Article));
    m_resourceCategoryFilter->addItem("Thesis", static_cast<int>(Resource::Category::Thesis));
    m_resourceCategoryFilter->addItem("Digital Content", static_cast<int>(Resource::Category::DigitalContent));
    searchLayout->addWidget(m_resourceCategoryFilter);
    searchLayout->addWidget(new QLabel("Status:"));
    m_resourceStatusFilter = new QComboBox();
    m_resourceStatusFilter->addItem("All Status", -1);
    m_resourceStatusFilter->addItem("Available", static_cast<int>(Resource::Status::Available));
    m_resourceStatusFilter->addItem("Borrowed", static_cast<int>(Resource::Status::Borrowed));
    m_resourceStatusFilter->addItem("Reserved", static_cast<int>(Resource::Status::Reserved));
    searchLayout->addWidget(m_resourceStatusFilter);
    layout->addWidget(searchGroup);
    m_resourceTable = new QTableWidget(m_resourceTab);
    m_resourceTable->setColumnCount(6);
    QStringList headers = {"ID", "Title", "Author", "Year", "Category", "Status"};
    m_resourceTable->setHorizontalHeaderLabels(headers);
    m_resourceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resourceTable->setAlternatingRowColors(true);
    m_resourceTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_resourceTable);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_addResourceBtn = new QPushButton("Add Resource");
    m_editResourceBtn = new QPushButton("Edit Resource");
    m_removeResourceBtn = new QPushButton("Remove Resource");
    m_borrowResourceBtn = new QPushButton("Borrow Resource");
    m_reserveResourceBtn = new QPushButton("Reserve Resource");
    m_refreshResourcesBtn = new QPushButton("Refresh");
    buttonLayout->addWidget(m_addResourceBtn);
    buttonLayout->addWidget(m_editResourceBtn);
    buttonLayout->addWidget(m_removeResourceBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_borrowResourceBtn);
    buttonLayout->addWidget(m_reserveResourceBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_refreshResourcesBtn);
    layout->addLayout(buttonLayout);
    m_editResourceBtn->setEnabled(false);
    m_removeResourceBtn->setEnabled(false);
    m_borrowResourceBtn->setEnabled(false);
    m_reserveResourceBtn->setEnabled(false);
}
void MainWindow::setupUserTab() {
    m_userTab = new QWidget();
    m_tabWidget->addTab(m_userTab, "User Management");
    QVBoxLayout* layout = new QVBoxLayout(m_userTab);
    QGroupBox* searchGroup = new QGroupBox("Search and Filter");
    QHBoxLayout* searchLayout = new QHBoxLayout(searchGroup);
    searchLayout->addWidget(new QLabel("Search:"));
    m_userSearchEdit = new QLineEdit();
    m_userSearchEdit->setPlaceholderText("Enter name, email, or user ID...");
    searchLayout->addWidget(m_userSearchEdit);
    searchLayout->addWidget(new QLabel("Type:"));
    m_userTypeFilter = new QComboBox();
    m_userTypeFilter->addItem("All Types", -1);
    m_userTypeFilter->addItem("Student", static_cast<int>(User::UserType::Student));    m_userTypeFilter->addItem("Teacher", static_cast<int>(User::UserType::Teacher));
    m_userTypeFilter->addItem("Staff", static_cast<int>(User::UserType::Staff));
    m_userTypeFilter->addItem("Administrator", static_cast<int>(User::UserType::Administrator));
    m_userTypeFilter->addItem("Guest", static_cast<int>(User::UserType::Guest));
    searchLayout->addWidget(m_userTypeFilter);
    searchLayout->addWidget(new QLabel("Status:"));
    m_userStatusFilter = new QComboBox();
    m_userStatusFilter->addItem("All Status", -1);
    m_userStatusFilter->addItem("Active", static_cast<int>(User::Status::Active));
    m_userStatusFilter->addItem("Inactive", static_cast<int>(User::Status::Inactive));
    m_userStatusFilter->addItem("Suspended", static_cast<int>(User::Status::Suspended));
    searchLayout->addWidget(m_userStatusFilter);
    layout->addWidget(searchGroup);
    m_userTable = new QTableWidget(m_userTab);
    m_userTable->setColumnCount(6);
    QStringList headers = {"ID", "Name", "Email", "Type", "Status", "Loans"};
    m_userTable->setHorizontalHeaderLabels(headers);
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_userTable->setAlternatingRowColors(true);
    m_userTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_userTable);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_addUserBtn = new QPushButton("Add User");
    m_editUserBtn = new QPushButton("Edit User");    m_removeUserBtn = new QPushButton("Remove User");
    m_viewUserLoansBtn = new QPushButton("View Loans");
    m_manageReservationsBtn = new QPushButton("Manage Reservations");
    m_refreshUsersBtn = new QPushButton("Refresh");
    buttonLayout->addWidget(m_addUserBtn);
    buttonLayout->addWidget(m_editUserBtn);
    buttonLayout->addWidget(m_removeUserBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_viewUserLoansBtn);
    buttonLayout->addWidget(m_manageReservationsBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_refreshUsersBtn);
    layout->addLayout(buttonLayout);
    m_editUserBtn->setEnabled(false);
    m_removeUserBtn->setEnabled(false);
    m_viewUserLoansBtn->setEnabled(false);
}
void MainWindow::setupLoanTab() {
    m_loanTab = new QWidget();
    m_tabWidget->addTab(m_loanTab, "Loan Management");
    QVBoxLayout* layout = new QVBoxLayout(m_loanTab);
    QSplitter* splitter = new QSplitter(Qt::Vertical);
    QWidget* activeLoansWidget = new QWidget();
    QVBoxLayout* activeLayout = new QVBoxLayout(activeLoansWidget);
    activeLayout->addWidget(new QLabel("Active Loans"));
      m_activeLoanTable = new QTableWidget(activeLoansWidget);
    m_activeLoanTable->setColumnCount(6);
    QStringList activeHeaders = {"Loan ID", "User", "Resource", "Borrow Date", "Due Date", "Status"};
    m_activeLoanTable->setHorizontalHeaderLabels(activeHeaders);
    m_activeLoanTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_activeLoanTable->setAlternatingRowColors(true);
    activeLayout->addWidget(m_activeLoanTable);
    splitter->addWidget(activeLoansWidget);
    QWidget* overdueLoansWidget = new QWidget();
    QVBoxLayout* overdueLayout = new QVBoxLayout(overdueLoansWidget);
    overdueLayout->addWidget(new QLabel("Overdue Loans"));
    m_overdueLoanTable = new QTableWidget(overdueLoansWidget);
    m_overdueLoanTable->setColumnCount(6);
    QStringList overdueHeaders = {"Loan ID", "User", "Resource", "Due Date", "Days Overdue", "Status"};
    m_overdueLoanTable->setHorizontalHeaderLabels(overdueHeaders);
    m_overdueLoanTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_overdueLoanTable->setAlternatingRowColors(true);
    overdueLayout->addWidget(m_overdueLoanTable);
    splitter->addWidget(overdueLoansWidget);
    layout->addWidget(splitter);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_returnBookBtn = new QPushButton("Return Book");
    m_renewLoanBtn = new QPushButton("Renew Loan");
    m_refreshLoansBtn = new QPushButton("Refresh");
    buttonLayout->addWidget(m_returnBookBtn);
    buttonLayout->addWidget(m_renewLoanBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_refreshLoansBtn);
    layout->addLayout(buttonLayout);
    m_returnBookBtn->setEnabled(false);
    m_renewLoanBtn->setEnabled(false);
}
void MainWindow::setupInfoTab() {
    m_infoTab = new QWidget();
    m_tabWidget->addTab(m_infoTab, "Library Information");
    QHBoxLayout* mainLayout = new QHBoxLayout(m_infoTab);
    QVBoxLayout* leftLayout = new QVBoxLayout();
    QGroupBox* infoGroup = new QGroupBox("Library Information");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    m_libraryNameLabel = new QLabel();
    m_libraryNameLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    infoLayout->addWidget(m_libraryNameLabel);
    m_operatingHoursLabel = new QLabel();
    infoLayout->addWidget(m_operatingHoursLabel);
    leftLayout->addWidget(infoGroup);
    QGroupBox* eventsGroup = new QGroupBox("Upcoming Events");
    QVBoxLayout* eventsLayout = new QVBoxLayout(eventsGroup);
    m_upcomingEventsList = new QListWidget();
    eventsLayout->addWidget(m_upcomingEventsList);
    QHBoxLayout* eventButtonLayout = new QHBoxLayout();
    m_addEventBtn = new QPushButton("Add Event");
    m_removeEventBtn = new QPushButton("Remove Event");
    eventButtonLayout->addWidget(m_addEventBtn);
    eventButtonLayout->addWidget(m_removeEventBtn);
    eventButtonLayout->addStretch();
    eventsLayout->addLayout(eventButtonLayout);
    leftLayout->addWidget(eventsGroup);
    mainLayout->addLayout(leftLayout);
    QVBoxLayout* rightLayout = new QVBoxLayout();
    QGroupBox* statsGroup = new QGroupBox("Library Statistics");
    QVBoxLayout* statsLayout = new QVBoxLayout(statsGroup);
    m_statisticsText = new QTextEdit();
    m_statisticsText->setReadOnly(true);
    statsLayout->addWidget(m_statisticsText);
    m_refreshStatsBtn = new QPushButton("Refresh Statistics");
    statsLayout->addWidget(m_refreshStatsBtn);
    rightLayout->addWidget(statsGroup);
    mainLayout->addLayout(rightLayout);
    mainLayout->setStretch(0, 1);
    mainLayout->setStretch(1, 1);
}
void MainWindow::setupConnections() {
    connect(m_resourceSearchEdit, &QLineEdit::textChanged, this, &MainWindow::onResourceSearchChanged);
    connect(m_resourceCategoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onResourceFilterChanged);
    connect(m_resourceStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onResourceFilterChanged);
    connect(m_resourceTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onResourceSelectionChanged);
    connect(m_addResourceBtn, &QPushButton::clicked, this, &MainWindow::onAddResourceClicked);
    connect(m_editResourceBtn, &QPushButton::clicked, this, &MainWindow::onEditResourceClicked);
    connect(m_removeResourceBtn, &QPushButton::clicked, this, &MainWindow::onRemoveResourceClicked);
    connect(m_borrowResourceBtn, &QPushButton::clicked, this, &MainWindow::onBorrowResourceClicked);
    connect(m_reserveResourceBtn, &QPushButton::clicked, this, &MainWindow::onReserveResourceClicked);
    connect(m_refreshResourcesBtn, &QPushButton::clicked, this, &MainWindow::onRefreshResourcesClicked);
    connect(m_userSearchEdit, &QLineEdit::textChanged, this, &MainWindow::onUserSearchChanged);
    connect(m_userTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUserFilterChanged);
    connect(m_userStatusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUserFilterChanged);
    connect(m_userTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onUserSelectionChanged);
    connect(m_addUserBtn, &QPushButton::clicked, this, &MainWindow::onAddUserClicked);
    connect(m_editUserBtn, &QPushButton::clicked, this, &MainWindow::onEditUserClicked);
    connect(m_removeUserBtn, &QPushButton::clicked, this, &MainWindow::onRemoveUserClicked);    connect(m_viewUserLoansBtn, &QPushButton::clicked, this, &MainWindow::onViewUserLoansClicked);
    connect(m_manageReservationsBtn, &QPushButton::clicked, this, &MainWindow::onManageReservationsClicked);
    connect(m_refreshUsersBtn, &QPushButton::clicked, this, &MainWindow::onRefreshUsersClicked);
    connect(m_activeLoanTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onLoanSelectionChanged);
    connect(m_overdueLoanTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onLoanSelectionChanged);
    connect(m_returnBookBtn, &QPushButton::clicked, this, &MainWindow::onReturnBookClicked);
    connect(m_renewLoanBtn, &QPushButton::clicked, this, &MainWindow::onRenewLoanClicked);
    connect(m_refreshLoansBtn, &QPushButton::clicked, this, &MainWindow::onRefreshLoansClicked);
    connect(m_addEventBtn, &QPushButton::clicked, this, &MainWindow::onAddEventClicked);
    connect(m_removeEventBtn, &QPushButton::clicked, this, &MainWindow::onRemoveEventClicked);
    connect(m_refreshStatsBtn, &QPushButton::clicked, this, &MainWindow::onRefreshStatsClicked);
    connect(m_libraryManager.get(), &LibraryManager::resourceAdded, this, &MainWindow::onResourceAdded);
    connect(m_libraryManager.get(), &LibraryManager::resourceRemoved, this, &MainWindow::onResourceRemoved);
    connect(m_libraryManager.get(), &LibraryManager::userAdded, this, &MainWindow::onUserAdded);
    connect(m_libraryManager.get(), &LibraryManager::userRemoved, this, &MainWindow::onUserRemoved);
    connect(m_libraryManager.get(), &LibraryManager::resourceBorrowed, this, &MainWindow::onResourceBorrowed);
    connect(m_libraryManager.get(), &LibraryManager::resourceReturned, this, &MainWindow::onResourceReturned);
    connect(m_libraryManager.get(), &LibraryManager::loanRenewed, this, &MainWindow::onLoanRenewed);    connect(m_libraryManager.get(), &LibraryManager::itemOverdue, this, &MainWindow::onItemOverdue);
    connect(m_libraryManager.get(), &LibraryManager::resourceReserved, this, &MainWindow::onResourceReserved);
    connect(m_libraryManager.get(), &LibraryManager::reservationCancelled, this, &MainWindow::onReservationCancelled);
    connect(m_libraryManager.get(), &LibraryManager::reservationExpired, this, &MainWindow::onReservationExpired);
    connect(m_libraryManager.get(), &LibraryManager::reservedResourceAvailable, this, &MainWindow::onReservedResourceAvailable);
}
void MainWindow::setupTimer() {
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::onAutoRefresh);
    m_refreshTimer->start(60000); 
}
void MainWindow::onResourceSearchChanged() {
    updateResourceTable();
}
void MainWindow::onResourceFilterChanged() {
    updateResourceTable();
}
void MainWindow::onResourceSelectionChanged() {
    int currentRow = m_resourceTable->currentRow();
    bool hasSelection = currentRow >= 0;
    m_editResourceBtn->setEnabled(hasSelection);
    m_removeResourceBtn->setEnabled(hasSelection);
    m_borrowResourceBtn->setEnabled(hasSelection);
    m_reserveResourceBtn->setEnabled(hasSelection);
    if (hasSelection) {
        QTableWidgetItem* idItem = m_resourceTable->item(currentRow, 0);
        m_selectedResourceId = idItem ? idItem->text() : "";
    } else {
        m_selectedResourceId.clear();
    }
}
void MainWindow::onAddResourceClicked() {
    showAddResourceDialog();
}
void MainWindow::onEditResourceClicked() {
    if (!validateResourceSelection()) return;
    Resource* resource = m_libraryManager->findResourceById(m_selectedResourceId);
    if (resource) {
        showEditResourceDialog(resource);
    }
}
void MainWindow::onRemoveResourceClicked() {
    if (!validateResourceSelection()) return;
    if (confirmAction("Are you sure you want to remove this resource?")) {
        try {
            if (m_libraryManager->removeResource(m_selectedResourceId)) {
                showSuccess("Resource removed successfully!");
                updateResourceTable();
            } else {
                showError("Failed to remove resource.");
            }
        } catch (const std::exception& e) {
            showError(QString("Error removing resource: %1").arg(e.what()));
        }
    }
}
void MainWindow::onBorrowResourceClicked() {
    if (!validateResourceSelection()) return;
    bool ok;
    QString userId = QInputDialog::getText(this, "Borrow Resource",
                                          "Enter User ID:", QLineEdit::Normal,
                                          "", &ok);
    if (ok && !userId.isEmpty()) {
        try {
            QString loanId = m_libraryManager->borrowResource(userId, m_selectedResourceId);
            showSuccess(QString("Resource borrowed successfully! Loan ID: %1").arg(loanId));
            updateResourceTable();
            updateLoanTables();
        } catch (const std::exception& e) {
            showError(QString("Error borrowing resource: %1").arg(e.what()));
        }
    }
}
void MainWindow::onReserveResourceClicked() {
    QModelIndexList selectedIndexes = m_resourceTable->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "No Selection", "Please select a resource to reserve.");
        return;
    }
    int row = selectedIndexes.first().row();
    QString resourceId = m_resourceTable->item(row, 0)->text();
    QString resourceTitle = m_resourceTable->item(row, 1)->text();
    QString availabilityText = m_resourceTable->item(row, 3)->text(); 
    if (availabilityText == "Available") {
        QMessageBox::information(this, "Resource Available", 
                                QString("'%1' is currently available for borrowing.\n\n"
                                       "Use 'Borrow Resource' instead of 'Reserve Resource' "
                                       "when the book is available.").arg(resourceTitle));
        return;
    }
    bool ok;
    QString userId = QInputDialog::getText(this, "Reserve Resource",
                                          QString("Enter User ID to reserve '%1':").arg(resourceTitle),
                                          QLineEdit::Normal, "", &ok);
    if (!ok || userId.isEmpty()) {
        return;
    }
      try {
        QString reservationId = m_libraryManager->reserveResource(userId, resourceId);
        if (!reservationId.isEmpty()) {
            showMessage(QString("Resource reserved successfully! Reservation ID: %1").arg(reservationId));
            updateResourceTable(); 
        } else {
            QMessageBox::warning(this, "Reservation Failed", "Failed to create reservation.");
        }
    } catch (const LibraryManagerException& e) {
        QMessageBox::information(this, "Reservation Info", e.getMessage());
    } catch (const ReservationException& e) {
        QMessageBox::warning(this, "Reservation Failed", e.getMessage());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString("An error occurred: %1").arg(e.what()));
    }
}
void MainWindow::onRefreshResourcesClicked() {
    updateResourceTable();
    showMessage("Resource list refreshed.");
}
void MainWindow::loadData() {
    if (m_persistenceService->loadLibraryData(*m_libraryManager)) {
        int userCount = m_libraryManager->getTotalUserCount();
        int resourceCount = m_libraryManager->getTotalResourceCount();
        showMessage(QString("Data loaded successfully. Users: %1, Resources: %2").arg(userCount).arg(resourceCount));
        refreshAllData();
    } else {
        showMessage("Using default data - " + m_persistenceService->getLastError());
    }
}
void MainWindow::saveData() {
    if (m_persistenceService->saveLibraryData(*m_libraryManager)) {
        showMessage("Data saved successfully.");
    } else {
        showError("Failed to save data: " + m_persistenceService->getLastError());
    }
}
void MainWindow::refreshAllData() {
    loadResourceData();
    loadUserData();
    loadLoanData();
    loadLibraryInfo();
    updateStatistics();
}
void MainWindow::loadResourceData() {
    auto resources = m_libraryManager->getAllResources();
    populateResourceTable(resources);
}
void MainWindow::populateResourceTable(const std::vector<Resource*>& resources) {
    m_resourceTable->setRowCount(resources.size());
    for (size_t i = 0; i < resources.size(); ++i) {
        const Resource* resource = resources[i];
        m_resourceTable->setItem(i, 0, new QTableWidgetItem(resource->getId()));
        m_resourceTable->setItem(i, 1, new QTableWidgetItem(resource->getTitle()));
        m_resourceTable->setItem(i, 2, new QTableWidgetItem(resource->getAuthor()));
        m_resourceTable->setItem(i, 3, new QTableWidgetItem(QString::number(resource->getPublicationYear())));
        m_resourceTable->setItem(i, 4, new QTableWidgetItem(Resource::categoryToString(resource->getCategory())));
        m_resourceTable->setItem(i, 5, new QTableWidgetItem(Resource::statusToString(resource->getStatus())));
    }
}
void MainWindow::updateResourceTable() {
    QString searchText = m_resourceSearchEdit->text();
    auto allResources = m_libraryManager->getAllResources();
    std::vector<Resource*> filteredResources;
    for (Resource* resource : allResources) {
        bool matchesSearch = searchText.isEmpty() || 
                           resource->getTitle().contains(searchText, Qt::CaseInsensitive) ||
                           resource->getAuthor().contains(searchText, Qt::CaseInsensitive);
        bool matchesCategory = m_resourceCategoryFilter->currentData().toInt() == -1 ||
                              static_cast<int>(resource->getCategory()) == m_resourceCategoryFilter->currentData().toInt();
        bool matchesStatus = m_resourceStatusFilter->currentData().toInt() == -1 ||
                            static_cast<int>(resource->getStatus()) == m_resourceStatusFilter->currentData().toInt();
        if (matchesSearch && matchesCategory && matchesStatus) {
            filteredResources.push_back(resource);
        }
    }
    populateResourceTable(filteredResources);
}
void MainWindow::loadUserData() {
    auto allUsers = m_libraryManager->getAllUsers();
    std::vector<User*> filteredUsers;
    QString searchText = m_userSearchEdit->text().toLower();
    int userTypeFilter = m_userTypeFilter->currentData().toInt();
    int statusFilter = m_userStatusFilter->currentData().toInt();
    for (User* user : allUsers) {
        bool matchesSearch = true;
        bool matchesTypeFilter = true;
        bool matchesStatusFilter = true;
        if (!searchText.isEmpty()) {
            QString userName = user->getName().toLower();
            QString userEmail = user->getEmail().toLower();
            QString userId = user->getId().toLower();
            matchesSearch = userName.contains(searchText) || 
                           userEmail.contains(searchText) || 
                           userId.contains(searchText);
        }
        if (userTypeFilter != -1) {
            matchesTypeFilter = static_cast<int>(user->getUserType()) == userTypeFilter;
        }
        if (statusFilter != -1) {
            matchesStatusFilter = static_cast<int>(user->getStatus()) == statusFilter;
        }
        if (matchesSearch && matchesTypeFilter && matchesStatusFilter) {
            filteredUsers.push_back(user);
        }
    }
    populateUserTable(filteredUsers);
}
void MainWindow::populateUserTable(const std::vector<User*>& users) {
    m_userTable->setRowCount(users.size());
    for (size_t i = 0; i < users.size(); ++i) {
        const User* user = users[i];
        m_userTable->setItem(i, 0, new QTableWidgetItem(user->getId()));
        m_userTable->setItem(i, 1, new QTableWidgetItem(user->getName()));
        m_userTable->setItem(i, 2, new QTableWidgetItem(user->getEmail()));        m_userTable->setItem(i, 3, new QTableWidgetItem(User::userTypeToString(user->getUserType())));
        m_userTable->setItem(i, 4, new QTableWidgetItem(User::statusToString(user->getStatus())));
        m_userTable->setItem(i, 5, new QTableWidgetItem(QString::number(user->getCurrentLoanCount())));
    }
}
void MainWindow::loadLoanData() {
    auto activeLoans = m_libraryManager->getActiveLoans();
    auto overdueLoans = m_libraryManager->getOverdueLoans();
    populateActiveLoanTable(activeLoans);
    populateOverdueLoanTable(overdueLoans);
}
void MainWindow::populateActiveLoanTable(const std::vector<Loan*>& loans) {
    m_activeLoanTable->setRowCount(loans.size());
    for (size_t i = 0; i < loans.size(); ++i) {
        const Loan* loan = loans[i];
        m_activeLoanTable->setItem(i, 0, new QTableWidgetItem(loan->getLoanId()));
        m_activeLoanTable->setItem(i, 1, new QTableWidgetItem(loan->getUserId()));
        m_activeLoanTable->setItem(i, 2, new QTableWidgetItem(loan->getResourceTitle()));
        m_activeLoanTable->setItem(i, 3, new QTableWidgetItem(loan->getBorrowDate().toString("yyyy-MM-dd")));
        m_activeLoanTable->setItem(i, 4, new QTableWidgetItem(loan->getDueDate().toString("yyyy-MM-dd")));
        m_activeLoanTable->setItem(i, 5, new QTableWidgetItem(loan->getStatusString()));
    }
}
void MainWindow::populateOverdueLoanTable(const std::vector<Loan*>& loans) {
    m_overdueLoanTable->setRowCount(loans.size());
    for (size_t i = 0; i < loans.size(); ++i) {
        const Loan* loan = loans[i];
        m_overdueLoanTable->setItem(i, 0, new QTableWidgetItem(loan->getLoanId()));
        m_overdueLoanTable->setItem(i, 1, new QTableWidgetItem(loan->getUserId()));
        m_overdueLoanTable->setItem(i, 2, new QTableWidgetItem(loan->getResourceTitle()));
        m_overdueLoanTable->setItem(i, 3, new QTableWidgetItem(loan->getDueDate().toString("yyyy-MM-dd")));
        m_overdueLoanTable->setItem(i, 4, new QTableWidgetItem(QString::number(loan->getDaysOverdue())));
        m_overdueLoanTable->setItem(i, 5, new QTableWidgetItem(loan->getStatusString()));
    }
}
void MainWindow::loadLibraryInfo() {
    m_libraryNameLabel->setText(m_libraryManager->getLibraryName());
    m_operatingHoursLabel->setText(m_libraryManager->getOperatingHours());
    m_upcomingEventsList->clear();
    for (const QString& event : m_libraryManager->getUpcomingEvents()) {
        m_upcomingEventsList->addItem(event);
    }
}
void MainWindow::updateStatistics() {
    QString stats;
    stats += QString("Total Resources: %1\n").arg(m_libraryManager->getTotalResourceCount());
    stats += QString("Available Resources: %1\n").arg(m_libraryManager->getAvailableResourceCount());
    stats += QString("Total Users: %1\n").arg(m_libraryManager->getTotalUserCount());
    stats += QString("Active Users: %1\n").arg(m_libraryManager->getActiveUserCount());
    stats += QString("Active Loans: %1\n").arg(m_libraryManager->getTotalActiveLoans());
    stats += QString("Overdue Loans: %1\n").arg(m_libraryManager->getTotalOverdueLoans());
    m_statisticsText->setText(stats);
}
void MainWindow::onUserSearchChanged() { 
    updateUserTable();
}
void MainWindow::onUserFilterChanged() { 
    updateUserTable();
}
void MainWindow::onUserSelectionChanged() { 
    QModelIndexList selectedIndexes = m_userTable->selectionModel()->selectedRows();
    bool hasSelection = !selectedIndexes.isEmpty();
    m_editUserBtn->setEnabled(hasSelection);
    m_removeUserBtn->setEnabled(hasSelection);
    m_viewUserLoansBtn->setEnabled(hasSelection);
    if (hasSelection) {
        int row = selectedIndexes.first().row();
        m_selectedUserId = m_userTable->item(row, 0)->text();
    } else {
        m_selectedUserId.clear();
    }
}
void MainWindow::onAddUserClicked() {
    UserDialog dialog(UserDialog::Mode::Add, this);
    if (dialog.exec() == QDialog::Accepted) {
        auto user = dialog.getUser();        if (user) {
            m_libraryManager->addUser(std::move(user));
            updateUserTable();
            saveData(); 
            showMessage("User added successfully!");
        }
    }
}
void MainWindow::onEditUserClicked() {
    int selectedRow = m_userTable->currentRow();
    if (selectedRow < 0) {
        showError("Please select a user to edit.");
        return;
    }
    QString userId = m_userTable->item(selectedRow, 0)->text();
    User* user = m_libraryManager->findUserById(userId);
    if (!user) {
        showError("Selected user not found.");
        return;
    }
    UserDialog dialog(user, this);
    if (dialog.exec() == QDialog::Accepted) {
        updateUserTable();
        showMessage("User updated successfully!");
    }
}
void MainWindow::onRemoveUserClicked() {
    int selectedRow = m_userTable->currentRow();
    if (selectedRow < 0) {
        showError("Please select a user to remove.");
        return;
    }
    QString userId = m_userTable->item(selectedRow, 0)->text();
    QString userName = m_userTable->item(selectedRow, 1)->text();
    int ret = QMessageBox::question(this, "Remove User", 
                                  QString("Are you sure you want to remove user '%1'?").arg(userName),
                                  QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        if (m_libraryManager->removeUser(userId)) {
            updateUserTable();
            showMessage("User removed successfully!");
        } else {
            showError("Failed to remove user. User may have active loans.");
        }
    }
}
void MainWindow::onViewUserLoansClicked() {
    int selectedRow = m_userTable->currentRow();
    if (selectedRow < 0) {
        showError("Please select a user to view loans.");
        return;
    }
    QString userId = m_userTable->item(selectedRow, 0)->text();
    User* user = m_libraryManager->findUserById(userId);
    if (!user) {
        showError("Selected user not found.");
        return;
    }
    UserLoansDialog dialog(user, m_libraryManager.get(), this);
    dialog.exec();
}
void MainWindow::onManageReservationsClicked() {
    ReservationManagementDialog dialog(m_libraryManager.get(), this);
    dialog.exec();
    refreshAllData();
}
void MainWindow::onRefreshUsersClicked() { loadUserData(); }
void MainWindow::onLoanSelectionChanged() { 
    bool hasActiveLoanSelection = m_activeLoanTable->selectionModel()->hasSelection();
    bool hasOverdueLoanSelection = m_overdueLoanTable->selectionModel()->hasSelection();
    bool hasAnySelection = hasActiveLoanSelection || hasOverdueLoanSelection;
    m_returnBookBtn->setEnabled(hasAnySelection);
    m_renewLoanBtn->setEnabled(hasActiveLoanSelection); 
    if (hasActiveLoanSelection) {
        QModelIndexList selectedIndexes = m_activeLoanTable->selectionModel()->selectedRows();
        if (!selectedIndexes.isEmpty()) {
            int row = selectedIndexes.first().row();
            m_selectedLoanId = m_activeLoanTable->item(row, 0)->text();
        }
    } else if (hasOverdueLoanSelection) {
        QModelIndexList selectedIndexes = m_overdueLoanTable->selectionModel()->selectedRows();
        if (!selectedIndexes.isEmpty()) {
            int row = selectedIndexes.first().row();
            m_selectedLoanId = m_overdueLoanTable->item(row, 0)->text();
        }
    } else {
        m_selectedLoanId.clear();
    }
}
void MainWindow::onReturnBookClicked() {
    int selectedRow = m_activeLoanTable->currentRow();
    if (selectedRow < 0) {
        showError("Please select a loan to return.");
        return;
    }
    QString loanId = m_activeLoanTable->item(selectedRow, 0)->text();
    QString resourceTitle = m_activeLoanTable->item(selectedRow, 2)->text();
    int ret = QMessageBox::question(this, "Return Resource", 
                                  QString("Return '%1'?").arg(resourceTitle),
                                  QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        if (m_libraryManager->returnResource(loanId)) {
            updateLoanTables();
            updateResourceTable();
            updateUserTable();
            showMessage("Resource returned successfully!");
        } else {
            showError("Failed to return resource.");
        }
    }
}
void MainWindow::onRenewLoanClicked() {
    int selectedRow = m_activeLoanTable->currentRow();
    if (selectedRow < 0) {
        showError("Please select a loan to renew.");
        return;
    }
    QString loanId = m_activeLoanTable->item(selectedRow, 0)->text();
    QString resourceTitle = m_activeLoanTable->item(selectedRow, 2)->text();
    bool ok;
    int days = QInputDialog::getInt(this, "Renew Loan", 
                                  QString("Renew '%1' for how many days?").arg(resourceTitle),
                                  14, 1, 30, 1, &ok);
    if (ok) {
        QDateTime newDueDate = m_libraryManager->renewLoan(loanId, days);
        if (!newDueDate.isNull()) {
            updateLoanTables();
            showMessage(QString("Loan renewed until %1").arg(newDueDate.toString("yyyy-MM-dd")));
        } else {
            showError("Failed to renew loan. Maximum renewals may have been reached.");
        }
    }
}
void MainWindow::onRefreshLoansClicked() { loadLoanData(); }
void MainWindow::onAddEventClicked() {
    bool ok;
    QString event = QInputDialog::getText(this, "Add Event", 
                                        "Enter event description:", 
                                        QLineEdit::Normal, "", &ok);
    if (ok && !event.trimmed().isEmpty()) {
        m_libraryManager->addUpcomingEvent(event.trimmed());
        loadLibraryInfo();
        showMessage("Event added successfully!");
    }
}
void MainWindow::onRemoveEventClicked() {
    auto events = m_libraryManager->getUpcomingEvents();
    if (events.empty()) {
        showMessage("No events to remove.");
        return;
    }
    bool ok;
    QStringList eventList;
    for (const auto& event : events) {
        eventList << event;
    }
    QString selectedEvent = QInputDialog::getItem(this, "Remove Event",
                                                "Select event to remove:",
                                                eventList, 0, false, &ok);
    if (ok && !selectedEvent.isEmpty()) {
        m_libraryManager->removeUpcomingEvent(selectedEvent);
        loadLibraryInfo();
        showMessage("Event removed successfully!");
    }
}
void MainWindow::onRefreshStatsClicked() { updateStatistics(); }
void MainWindow::onAutoRefresh() { 
    m_libraryManager->performDailyMaintenance();
    updateLoanTables();
}
void MainWindow::onResourceAdded(const QString& resourceId) { 
    showMessage("Resource added: " + resourceId);
    updateResourceTable();
}
void MainWindow::onResourceRemoved(const QString& resourceId) { 
    showMessage("Resource removed: " + resourceId);
    updateResourceTable();
}
void MainWindow::onUserAdded(const QString& userId) { 
    showMessage("User added: " + userId);
    loadUserData();
}
void MainWindow::onUserRemoved(const QString& userId) { 
    showMessage("User removed: " + userId);
    loadUserData();
}
void MainWindow::onResourceBorrowed(const QString& loanId, const QString& userId, const QString& resourceId) { 
    showMessage(QString("Resource borrowed - Loan: %1").arg(loanId));
    updateResourceTable();
    updateLoanTables();
}
void MainWindow::onResourceReturned(const QString& loanId, const QString& userId, const QString& resourceId) { 
    showMessage(QString("Resource returned - Loan: %1").arg(loanId));
    updateResourceTable();
    updateLoanTables();
}
void MainWindow::onLoanRenewed(const QString& loanId, const QDateTime& newDueDate) { 
    showMessage(QString("Loan renewed: %1").arg(loanId));
    updateLoanTables();
}
void MainWindow::onItemOverdue(const QString& loanId, const QString& userId, const QString& resourceId) { 
    showMessage(QString("⚠️ Item overdue - Loan: %1").arg(loanId));
}
void MainWindow::onResourceReserved(const QString& reservationId, const QString& userId, const QString& resourceId) {
    showMessage(QString("📝 Resource reserved successfully! User: %1, Reservation ID: %2").arg(userId, reservationId));
    updateResourceTable(); 
}
void MainWindow::onReservationCancelled(const QString& reservationId, const QString& userId, const QString& resourceId) {
    showMessage(QString("❌ Reservation cancelled - User: %1, Reservation ID: %2").arg(userId, reservationId));
    updateResourceTable();
}
void MainWindow::onReservationExpired(const QString& reservationId, const QString& userId, const QString& resourceId) {
    showMessage(QString("⏰ Reservation expired - User: %1, Reservation ID: %2").arg(userId, reservationId));
    updateResourceTable();
}
void MainWindow::onReservedResourceAvailable(const QString& reservationId, const QString& userId, const QString& resourceId) {
    showMessage(QString("🔔 NOTIFICATION: Reserved resource is now available! User: %1, Reservation ID: %2").arg(userId, reservationId));
    QMessageBox::information(this, "Resource Available", 
                           QString("The resource you reserved is now available for pickup!\n\n"
                                  "User ID: %1\n"
                                  "Reservation ID: %2\n"
                                  "Resource ID: %3").arg(userId, reservationId, resourceId));
}
void MainWindow::updateUserTable() {
    loadUserData();
}
void MainWindow::onDataSaved() {
    showMessage("Data saved successfully!");
}
void MainWindow::onDataLoaded() {
    showMessage("Data loaded successfully!");
    refreshAllData();
}
void MainWindow::showAddResourceDialog() {
    ResourceDialog dialog(ResourceDialog::Mode::Add, this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            auto resource = dialog.getResource();            if (resource) {
                m_libraryManager->addResource(std::move(resource));
                showSuccess("Resource added successfully!");
                updateResourceTable();
                saveData(); 
            }
        } catch (const std::exception& e) {
            showError(QString("Error adding resource: %1").arg(e.what()));
        }
    }
}
void MainWindow::showEditResourceDialog(Resource* resource) {
    if (!resource) return;
    ResourceDialog dialog(resource, this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            auto updatedResource = dialog.getResource();
            if (updatedResource) {
                QString resourceId = resource->getId();
                m_libraryManager->removeResource(resourceId);                m_libraryManager->addResource(std::move(updatedResource));
                showSuccess("Resource updated successfully!");
                updateResourceTable();
                saveData(); 
            }
        } catch (const std::exception& e) {
            showError(QString("Error updating resource: %1").arg(e.what()));
        }
    }
}
void MainWindow::showAddUserDialog() {
    UserDialog dialog(UserDialog::Mode::Add, this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            auto user = dialog.getUser();            if (user) {
                m_libraryManager->addUser(std::move(user));
                showSuccess("User added successfully!");
                updateUserTable();
                saveData(); 
            }
        } catch (const std::exception& e) {
            showError(QString("Error adding user: %1").arg(e.what()));
        }
    }
}
void MainWindow::showEditUserDialog(User* user) {
    if (!user) return;
    UserDialog dialog(user, this);
    if (dialog.exec() == QDialog::Accepted) {
        try {
            auto updatedUser = dialog.getUser();
            if (updatedUser) {
                QString userId = user->getId();
                m_libraryManager->removeUser(userId);                m_libraryManager->addUser(std::move(updatedUser));
                showSuccess("User updated successfully!");
                updateUserTable();
                saveData(); 
            }
        } catch (const std::exception& e) {
            showError(QString("Error updating user: %1").arg(e.what()));
        }
    }
}
void MainWindow::showUserLoansDialog(User* user) {
    if (!user) return;
    UserLoansDialog dialog(user, m_libraryManager.get(), this);
    dialog.exec();
    updateUserTable();
    updateLoanTables();
}
void MainWindow::showAddEventDialog() {
    bool ok;
    QString event = QInputDialog::getText(this, "Add Event", "Enter event details:", 
                                         QLineEdit::Normal, "", &ok);
    if (ok && !event.isEmpty()) {
        m_libraryManager->addUpcomingEvent(event);
        loadLibraryInfo();
        showSuccess("Event added successfully!");
    }
}
void MainWindow::showMessage(const QString& message, int timeout) {
    m_statusBar->showMessage(message, timeout);
}
void MainWindow::showError(const QString& error) {
    QMessageBox::critical(this, "Error", error);
    m_statusBar->showMessage("Error: " + error, 5000);
}
void MainWindow::showSuccess(const QString& message) {
    QMessageBox::information(this, "Success", message);
    m_statusBar->showMessage(message, 3000);
}
bool MainWindow::confirmAction(const QString& message) {
    return QMessageBox::question(this, "Confirm", message, 
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}
bool MainWindow::validateResourceSelection() {
    if (m_selectedResourceId.isEmpty()) {
        showError("Please select a resource first.");
        return false;
    }
    return true;
}
bool MainWindow::validateUserSelection() {
    if (m_selectedUserId.isEmpty()) {
        showError("Please select a user first.");
        return false;
    }
    return true;
}
bool MainWindow::validateLoanSelection() {
    if (m_selectedLoanId.isEmpty()) {
        showError("Please select a loan first.");
        return false;
    }
    return true;
}
void MainWindow::updateLoanTables() {
    loadLoanData();
}