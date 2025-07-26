#include "reservation_management_dialog.h"
#include "../services/library_manager.h"
#include "../models/reservation.h"
#include "../models/user.h"
#include "../models/resource.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QDebug>
ReservationManagementDialog::ReservationManagementDialog(LibraryManager* libraryManager, QWidget *parent)
    : QDialog(parent)
    , m_libraryManager(libraryManager)
    , m_tabWidget(nullptr)
    , m_createTab(nullptr)
    , m_activeTab(nullptr)
    , m_historyTab(nullptr)
    , m_userComboBox(nullptr)
    , m_resourceComboBox(nullptr)
    , m_expiryDateEdit(nullptr)
    , m_createButton(nullptr)
    , m_statusLabel(nullptr)
    , m_activeReservationsTable(nullptr)
    , m_cancelButton(nullptr)
    , m_viewDetailsButton(nullptr)
    , m_refreshButton(nullptr)
    , m_activeCountLabel(nullptr)
    , m_historyTable(nullptr)
    , m_refreshHistoryButton(nullptr)
    , m_historyCountLabel(nullptr)
{
    setWindowTitle("Reservation Management - Admin Panel");
    setModal(true);
    resize(800, 600);
    setupUI();
    populateUserComboBox();
    populateResourceComboBox();
    populateActiveReservations();
    populateReservationHistory();
    updateCreateButtonState();
}
void ReservationManagementDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget();
    setupCreateReservationTab();
    setupActiveReservationsTab();
    setupReservationHistoryTab();
    mainLayout->addWidget(m_tabWidget);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton* closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeButton);
    mainLayout->addLayout(buttonLayout);
}
void ReservationManagementDialog::setupCreateReservationTab()
{
    m_createTab = new QWidget();
    m_tabWidget->addTab(m_createTab, "Create Reservation");
    QVBoxLayout* layout = new QVBoxLayout(m_createTab);
    QGroupBox* formGroup = new QGroupBox("Create New Reservation");
    QVBoxLayout* formLayout = new QVBoxLayout(formGroup);
    QHBoxLayout* userLayout = new QHBoxLayout();
    userLayout->addWidget(new QLabel("Select User:"));
    m_userComboBox = new QComboBox();
    m_userComboBox->setMinimumWidth(300);
    connect(m_userComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReservationManagementDialog::onUserSelectionChanged);
    userLayout->addWidget(m_userComboBox);
    userLayout->addStretch();
    formLayout->addLayout(userLayout);
    QHBoxLayout* resourceLayout = new QHBoxLayout();
    resourceLayout->addWidget(new QLabel("Select Resource:"));
    m_resourceComboBox = new QComboBox();
    m_resourceComboBox->setMinimumWidth(300);
    connect(m_resourceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReservationManagementDialog::onResourceSelectionChanged);
    resourceLayout->addWidget(m_resourceComboBox);
    resourceLayout->addStretch();
    formLayout->addLayout(resourceLayout);
    QHBoxLayout* dateLayout = new QHBoxLayout();
    dateLayout->addWidget(new QLabel("Reservation Expiry:"));
    m_expiryDateEdit = new QDateEdit();
    m_expiryDateEdit->setDate(QDate::currentDate().addDays(7)); 
    m_expiryDateEdit->setMinimumDate(QDate::currentDate().addDays(1));
    m_expiryDateEdit->setCalendarPopup(true);
    dateLayout->addWidget(m_expiryDateEdit);
    dateLayout->addStretch();
    formLayout->addLayout(dateLayout);
    QHBoxLayout* createLayout = new QHBoxLayout();
    m_createButton = new QPushButton("Create Reservation");
    m_createButton->setEnabled(false);
    connect(m_createButton, &QPushButton::clicked, this, &ReservationManagementDialog::onCreateReservation);
    createLayout->addWidget(m_createButton);
    createLayout->addStretch();
    formLayout->addLayout(createLayout);
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    formLayout->addWidget(m_statusLabel);
    layout->addWidget(formGroup);
    layout->addStretch();
}
void ReservationManagementDialog::setupActiveReservationsTab()
{
    m_activeTab = new QWidget();
    m_tabWidget->addTab(m_activeTab, "Active Reservations");
    QVBoxLayout* layout = new QVBoxLayout(m_activeTab);
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel("Active Reservations:"));
    m_activeCountLabel = new QLabel("0 reservations");
    m_activeCountLabel->setStyleSheet("QLabel { font-weight: bold; color: blue; }");
    headerLayout->addWidget(m_activeCountLabel);
    headerLayout->addStretch();
    m_refreshButton = new QPushButton("Refresh");
    connect(m_refreshButton, &QPushButton::clicked, this, &ReservationManagementDialog::onRefreshData);
    headerLayout->addWidget(m_refreshButton);
    layout->addLayout(headerLayout);
    m_activeReservationsTable = new QTableWidget();
    m_activeReservationsTable->setColumnCount(6);
    QStringList headers = {"User", "Resource", "Reserved Date", "Expiry Date", "Status", "Days Left"};
    m_activeReservationsTable->setHorizontalHeaderLabels(headers);
    m_activeReservationsTable->horizontalHeader()->setStretchLastSection(true);
    m_activeReservationsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_activeReservationsTable->setAlternatingRowColors(true);
    connect(m_activeReservationsTable, &QTableWidget::itemSelectionChanged,
            this, &ReservationManagementDialog::onActiveReservationSelectionChanged);
    layout->addWidget(m_activeReservationsTable);
    QHBoxLayout* actionLayout = new QHBoxLayout();
    m_viewDetailsButton = new QPushButton("View Details");
    m_viewDetailsButton->setEnabled(false);
    connect(m_viewDetailsButton, &QPushButton::clicked, this, &ReservationManagementDialog::onViewReservationDetails);
    actionLayout->addWidget(m_viewDetailsButton);
    m_cancelButton = new QPushButton("Cancel Reservation");
    m_cancelButton->setEnabled(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &ReservationManagementDialog::onCancelReservation);
    actionLayout->addWidget(m_cancelButton);
    actionLayout->addStretch();
    layout->addLayout(actionLayout);
}
void ReservationManagementDialog::setupReservationHistoryTab()
{
    m_historyTab = new QWidget();
    m_tabWidget->addTab(m_historyTab, "Reservation History");
    QVBoxLayout* layout = new QVBoxLayout(m_historyTab);
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(new QLabel("Reservation History:"));
    m_historyCountLabel = new QLabel("0 reservations");
    m_historyCountLabel->setStyleSheet("QLabel { font-weight: bold; color: green; }");
    headerLayout->addWidget(m_historyCountLabel);
    headerLayout->addStretch();
    m_refreshHistoryButton = new QPushButton("Refresh");
    connect(m_refreshHistoryButton, &QPushButton::clicked, this, &ReservationManagementDialog::onRefreshData);
    headerLayout->addWidget(m_refreshHistoryButton);
    layout->addLayout(headerLayout);
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(6);
    QStringList headers = {"User", "Resource", "Reserved Date", "Completed Date", "Status", "Outcome"};
    m_historyTable->setHorizontalHeaderLabels(headers);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setAlternatingRowColors(true);
    connect(m_historyTable, &QTableWidget::itemSelectionChanged,
            this, &ReservationManagementDialog::onHistoryReservationSelectionChanged);
    layout->addWidget(m_historyTable);
}
void ReservationManagementDialog::populateUserComboBox()
{
    m_userComboBox->clear();
    m_userComboBox->addItem("Select a user...", "");
    auto users = m_libraryManager->getAllUsers();
    for (const User* user : users) {
        QString displayText = QString("%1 - %2 (%3)")
                                .arg(user->getName())
                                .arg(user->getEmail())
                                .arg(user->getId());
        m_userComboBox->addItem(displayText, user->getId());
    }
}
void ReservationManagementDialog::populateResourceComboBox()
{
    m_resourceComboBox->clear();
    m_resourceComboBox->addItem("Select a resource...", "");
    auto resources = m_libraryManager->getAllResources();
    for (const Resource* resource : resources) {
        QString displayText = QString("%1 - %2 (%3)")
                                .arg(resource->getTitle())
                                .arg(resource->getAuthor())
                                .arg(resource->getId());
        if (!resource->isAvailable()) {
            displayText += " [UNAVAILABLE]";
        }
        m_resourceComboBox->addItem(displayText, resource->getId());
    }
}
void ReservationManagementDialog::populateActiveReservations()
{
    auto reservations = m_libraryManager->getActiveReservations();
    m_activeReservationsTable->setRowCount(reservations.size());
    for (int i = 0; i < reservations.size(); ++i) {
        const Reservation* reservation = reservations[i];
        User* user = m_libraryManager->findUserById(reservation->getUserId());
        Resource* resource = m_libraryManager->findResourceById(reservation->getResourceId());
        QString userName = user ? user->getName() : "Unknown User";
        QString resourceTitle = resource ? resource->getTitle() : "Unknown Resource";
        m_activeReservationsTable->setItem(i, 0, new QTableWidgetItem(userName));
        m_activeReservationsTable->setItem(i, 1, new QTableWidgetItem(resourceTitle));
        m_activeReservationsTable->setItem(i, 2, new QTableWidgetItem(reservation->getReservationDate().toString("yyyy-MM-dd hh:mm")));        m_activeReservationsTable->setItem(i, 3, new QTableWidgetItem(reservation->getExpirationDate().toString("yyyy-MM-dd hh:mm")));
        m_activeReservationsTable->setItem(i, 4, new QTableWidgetItem(Reservation::statusToString(reservation->getStatus())));
        int daysLeft = QDate::currentDate().daysTo(reservation->getExpirationDate().date());
        QString daysLeftText = daysLeft > 0 ? QString("%1 days").arg(daysLeft) : "Expired";
        m_activeReservationsTable->setItem(i, 5, new QTableWidgetItem(daysLeftText));
        m_activeReservationsTable->item(i, 0)->setData(Qt::UserRole, reservation->getId());
        if (daysLeft <= 0) {
            for (int col = 0; col < 6; ++col) {
                m_activeReservationsTable->item(i, col)->setBackground(QColor(255, 200, 200));
            }
        }
    }
    m_activeCountLabel->setText(QString("%1 active reservations").arg(reservations.size()));
}
void ReservationManagementDialog::populateReservationHistory()
{
    auto history = m_libraryManager->getReservationHistory();
    m_historyTable->setRowCount(history.size());
    for (int i = 0; i < history.size(); ++i) {
        const Reservation* reservation = history[i];
        User* user = m_libraryManager->findUserById(reservation->getUserId());
        Resource* resource = m_libraryManager->findResourceById(reservation->getResourceId());
        QString userName = user ? user->getName() : "Unknown User";
        QString resourceTitle = resource ? resource->getTitle() : "Unknown Resource";
        m_historyTable->setItem(i, 0, new QTableWidgetItem(userName));
        m_historyTable->setItem(i, 1, new QTableWidgetItem(resourceTitle));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(reservation->getReservationDate().toString("yyyy-MM-dd hh:mm")));        m_historyTable->setItem(i, 3, new QTableWidgetItem(reservation->getExpirationDate().toString("yyyy-MM-dd hh:mm")));
        m_historyTable->setItem(i, 4, new QTableWidgetItem(Reservation::statusToString(reservation->getStatus())));
        QString outcome = "Unknown";
        if (reservation->getStatus() == Reservation::Status::Fulfilled) {
            outcome = "Fulfilled";
        } else if (reservation->getStatus() == Reservation::Status::Cancelled) {
            outcome = "Cancelled";
        } else if (reservation->getStatus() == Reservation::Status::Expired) {
            outcome = "Expired";
        }
        m_historyTable->setItem(i, 5, new QTableWidgetItem(outcome));
        m_historyTable->item(i, 0)->setData(Qt::UserRole, reservation->getId());
    }
    m_historyCountLabel->setText(QString("%1 completed reservations").arg(history.size()));
}
void ReservationManagementDialog::onCreateReservation()
{
    QString userId = m_userComboBox->currentData().toString();
    QString resourceId = m_resourceComboBox->currentData().toString();
    if (userId.isEmpty() || resourceId.isEmpty()) {
        QMessageBox::warning(this, "Invalid Selection", "Please select both a user and a resource.");
        return;
    }
    int expirationDays = QDate::currentDate().daysTo(m_expiryDateEdit->date());
    if (expirationDays <= 0) {
        QMessageBox::warning(this, "Invalid Date", "Expiry date must be in the future.");
        return;
    }
    QString reservationId = m_libraryManager->reserveResource(userId, resourceId);
    if (!reservationId.isEmpty()) {
        m_statusLabel->setText("Reservation created successfully!");
        m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        populateActiveReservations();
        m_tabWidget->setCurrentIndex(1);
        m_userComboBox->setCurrentIndex(0);
        m_resourceComboBox->setCurrentIndex(0);
        m_expiryDateEdit->setDate(QDate::currentDate().addDays(7));
    } else {
        m_statusLabel->setText("Failed to create reservation. Resource may not be available or user may have existing reservation.");
        m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    }
}
void ReservationManagementDialog::onCancelReservation()
{
    if (m_selectedActiveReservationId.isEmpty()) {
        return;
    }
    int ret = QMessageBox::question(this, "Cancel Reservation",
                                    "Are you sure you want to cancel this reservation?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        bool success = m_libraryManager->cancelReservation(m_selectedActiveReservationId);
        if (success) {
            QMessageBox::information(this, "Success", "Reservation cancelled successfully.");
            onRefreshData();
        } else {
            QMessageBox::warning(this, "Error", "Failed to cancel reservation.");
        }
    }
}
void ReservationManagementDialog::onViewReservationDetails()
{
    if (m_selectedActiveReservationId.isEmpty()) {
        return;
    }
    Reservation* reservation = m_libraryManager->findReservationById(m_selectedActiveReservationId);
    if (reservation) {
        showReservationDetails(reservation);
    }
}
void ReservationManagementDialog::onRefreshData()
{
    populateActiveReservations();
    populateReservationHistory();
    populateUserComboBox();
    populateResourceComboBox();
}
void ReservationManagementDialog::onActiveReservationSelectionChanged()
{
    QList<QTableWidgetItem*> selectedItems = m_activeReservationsTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        int row = selectedItems.first()->row();
        QTableWidgetItem* firstItem = m_activeReservationsTable->item(row, 0);
        if (firstItem) {
            m_selectedActiveReservationId = firstItem->data(Qt::UserRole).toString();
            m_cancelButton->setEnabled(true);
            m_viewDetailsButton->setEnabled(true);
        }
    } else {
        m_selectedActiveReservationId.clear();
        m_cancelButton->setEnabled(false);
        m_viewDetailsButton->setEnabled(false);
    }
}
void ReservationManagementDialog::onHistoryReservationSelectionChanged()
{
    QList<QTableWidgetItem*> selectedItems = m_historyTable->selectedItems();
    if (!selectedItems.isEmpty()) {
        int row = selectedItems.first()->row();
        QTableWidgetItem* firstItem = m_historyTable->item(row, 0);
        if (firstItem) {
            m_selectedHistoryReservationId = firstItem->data(Qt::UserRole).toString();
        }
    } else {
        m_selectedHistoryReservationId.clear();
    }
}
void ReservationManagementDialog::onUserSelectionChanged()
{
    updateCreateButtonState();
}
void ReservationManagementDialog::onResourceSelectionChanged()
{
    updateCreateButtonState();
}
void ReservationManagementDialog::updateCreateButtonState()
{
    bool userSelected = !m_userComboBox->currentData().toString().isEmpty();
    bool resourceSelected = !m_resourceComboBox->currentData().toString().isEmpty();
    m_createButton->setEnabled(userSelected && resourceSelected);
    if (m_statusLabel) {
        m_statusLabel->clear();
    }
}
void ReservationManagementDialog::showReservationDetails(const Reservation* reservation)
{
    if (!reservation) return;
    User* user = m_libraryManager->findUserById(reservation->getUserId());
    Resource* resource = m_libraryManager->findResourceById(reservation->getResourceId());
    QString details = QString(
        "Reservation Details:\n\n"
        "Reservation ID: %1\n"
        "User: %2 (%3)\n"
        "Resource: %4\n"
        "Author: %5\n"
        "Reserved: %6\n"
        "Expires: %7\n"
        "Status: %8\n"
        "Days until expiry: %9"
    ).arg(reservation->getId())
     .arg(user ? user->getName() : "Unknown")
     .arg(user ? user->getEmail() : "Unknown")
     .arg(resource ? resource->getTitle() : "Unknown")
     .arg(resource ? resource->getAuthor() : "Unknown")
     .arg(reservation->getReservationDate().toString("yyyy-MM-dd hh:mm:ss"))     .arg(reservation->getExpirationDate().toString("yyyy-MM-dd hh:mm:ss"))
     .arg(Reservation::statusToString(reservation->getStatus()))
     .arg(QDate::currentDate().daysTo(reservation->getExpirationDate().date()));
    QMessageBox::information(this, "Reservation Details", details);
}