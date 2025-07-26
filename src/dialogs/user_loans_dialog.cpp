#include "user_loans_dialog.h"
#include <QMessageBox>
#include <QDateTime>
#include <QHeaderView>
UserLoansDialog::UserLoansDialog(User* user, LibraryManager* libraryManager, QWidget* parent)
    : QDialog(parent), m_user(user), m_libraryManager(libraryManager) {
    setWindowTitle(QString("Loans for %1").arg(user ? user->getName() : "Unknown User"));
    setModal(true);
    resize(800, 600);
    setupUI();
    setupConnections();
    loadLoanData();
}
void UserLoansDialog::setupUI() {
    QString styleSheet = R"(
        QDialog {
            background-color: #ffffff;
            color: #1e3a8a;
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
            font-weight: bold;
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
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #ea580c;
        }
        QPushButton:pressed {
            background-color: #c2410c;
        }
        QPushButton:disabled {
            background-color: #9ca3af;
            color: #6b7280;
        }
    )";
    this->setStyleSheet(styleSheet);
    m_mainLayout = new QVBoxLayout(this);
    m_userInfoGroup = new QGroupBox("User Information");
    QVBoxLayout* userInfoLayout = new QVBoxLayout(m_userInfoGroup);
    m_userNameLabel = new QLabel();
    m_userEmailLabel = new QLabel();
    m_userTypeLabel = new QLabel();
    m_userStatusLabel = new QLabel();
    m_currentLoansCountLabel = new QLabel();
    userInfoLayout->addWidget(m_userNameLabel);
    userInfoLayout->addWidget(m_userEmailLabel);
    userInfoLayout->addWidget(m_userTypeLabel);
    userInfoLayout->addWidget(m_userStatusLabel);
    userInfoLayout->addWidget(m_currentLoansCountLabel);
    m_mainLayout->addWidget(m_userInfoGroup);
    m_splitter = new QSplitter(Qt::Vertical);
    m_currentLoansGroup = new QGroupBox("Current Loans");
    QVBoxLayout* currentLoansLayout = new QVBoxLayout(m_currentLoansGroup);
    m_currentLoansTable = new QTableWidget();
    m_currentLoansTable->setColumnCount(5);
    QStringList currentHeaders = {"Loan ID", "Resource", "Borrow Date", "Due Date", "Status"};
    m_currentLoansTable->setHorizontalHeaderLabels(currentHeaders);
    m_currentLoansTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_currentLoansTable->setAlternatingRowColors(true);
    m_currentLoansTable->horizontalHeader()->setStretchLastSection(true);
    currentLoansLayout->addWidget(m_currentLoansTable);
    m_splitter->addWidget(m_currentLoansGroup);
    m_historyGroup = new QGroupBox("Loan History");
    QVBoxLayout* historyLayout = new QVBoxLayout(m_historyGroup);
    m_historyTable = new QTableWidget();
    m_historyTable->setColumnCount(4);
    QStringList historyHeaders = {"Loan ID", "Resource", "Borrow Date", "Return Date"};
    m_historyTable->setHorizontalHeaderLabels(historyHeaders);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    historyLayout->addWidget(m_historyTable);
    m_splitter->addWidget(m_historyGroup);
    m_detailsGroup = new QGroupBox("Loan Details");
    QVBoxLayout* detailsLayout = new QVBoxLayout(m_detailsGroup);
    m_detailsText = new QTextEdit();
    m_detailsText->setReadOnly(true);
    m_detailsText->setMaximumHeight(120);
    m_detailsText->setPlainText("Select a loan to view details...");
    detailsLayout->addWidget(m_detailsText);
    m_splitter->addWidget(m_detailsGroup);
    m_splitter->setStretchFactor(0, 2); 
    m_splitter->setStretchFactor(1, 2); 
    m_splitter->setStretchFactor(2, 1); 
    m_mainLayout->addWidget(m_splitter);
    m_buttonLayout = new QHBoxLayout();
    m_returnBtn = new QPushButton("Return Selected Loan");
    m_renewBtn = new QPushButton("Renew Selected Loan");
    m_refreshBtn = new QPushButton("Refresh");
    m_closeBtn = new QPushButton("Close");
    m_returnBtn->setEnabled(false);
    m_renewBtn->setEnabled(false);
    m_buttonLayout->addWidget(m_returnBtn);
    m_buttonLayout->addWidget(m_renewBtn);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_refreshBtn);
    m_buttonLayout->addWidget(m_closeBtn);
    m_mainLayout->addLayout(m_buttonLayout);
    if (m_user) {        m_userNameLabel->setText(QString("<b>Name:</b> %1").arg(m_user->getName()));
        m_userEmailLabel->setText(QString("<b>Email:</b> %1").arg(m_user->getEmail()));
        m_userTypeLabel->setText(QString("<b>Type:</b> %1").arg(User::typeToString(m_user->getType())));
        m_userStatusLabel->setText(QString("<b>Status:</b> %1").arg(User::statusToString(m_user->getStatus())));
        m_currentLoansCountLabel->setText(QString("<b>Current Loans:</b> %1 / %2").arg(m_user->getCurrentLoans().size()).arg(m_user->getMaxLoans()));
    }
}
void UserLoansDialog::setupConnections() {
    connect(m_currentLoansTable, &QTableWidget::itemSelectionChanged, this, &UserLoansDialog::onLoanSelectionChanged);
    connect(m_historyTable, &QTableWidget::itemSelectionChanged, this, &UserLoansDialog::onLoanSelectionChanged);
    connect(m_returnBtn, &QPushButton::clicked, this, &UserLoansDialog::onReturnLoan);
    connect(m_renewBtn, &QPushButton::clicked, this, &UserLoansDialog::onRenewLoan);
    connect(m_refreshBtn, &QPushButton::clicked, this, &UserLoansDialog::onRefresh);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}
void UserLoansDialog::loadLoanData() {
    if (!m_user || !m_libraryManager) return;
    m_currentLoans.clear();
    const auto& userLoans = m_user->getCurrentLoans();
    for (const auto& loan : userLoans) {
        m_currentLoans.push_back(std::make_unique<Loan>(*loan));
    }
    m_loanHistory.clear();
    auto allLoans = m_libraryManager->getCompletedLoans();
    for (const auto& loan : allLoans) {
        if (loan->getUserId() == m_user->getId()) {
            m_loanHistory.push_back(std::make_unique<Loan>(*loan));
        }
    }
    populateCurrentLoans();
    populateLoanHistory();
    updateButtonStates();
}
void UserLoansDialog::populateCurrentLoans() {
    m_currentLoansTable->setRowCount(m_currentLoans.size());
    for (size_t i = 0; i < m_currentLoans.size(); ++i) {
        const Loan* loan = m_currentLoans[i].get();
        m_currentLoansTable->setItem(i, 0, new QTableWidgetItem(loan->getId()));
        Resource* resource = m_libraryManager->findResourceById(loan->getResourceId());
        QString resourceTitle = resource ? resource->getTitle() : "Unknown Resource";
        m_currentLoansTable->setItem(i, 1, new QTableWidgetItem(resourceTitle));
        m_currentLoansTable->setItem(i, 2, new QTableWidgetItem(loan->getBorrowDate().toString("yyyy-MM-dd")));
        m_currentLoansTable->setItem(i, 3, new QTableWidgetItem(formatDueDate(loan->getDueDate())));
        m_currentLoansTable->setItem(i, 4, new QTableWidgetItem(formatLoanStatus(*loan)));
        if (loan->isOverdue()) {
            for (int col = 0; col < m_currentLoansTable->columnCount(); ++col) {
                QTableWidgetItem* item = m_currentLoansTable->item(i, col);
                if (item) {
                    item->setBackground(QColor(255, 200, 200)); 
                }
            }
        }
    }
}
void UserLoansDialog::populateLoanHistory() {
    m_historyTable->setRowCount(m_loanHistory.size());
    for (size_t i = 0; i < m_loanHistory.size(); ++i) {
        const Loan* loan = m_loanHistory[i].get();
        m_historyTable->setItem(i, 0, new QTableWidgetItem(loan->getId()));
        Resource* resource = m_libraryManager->findResourceById(loan->getResourceId());
        QString resourceTitle = resource ? resource->getTitle() : "Unknown Resource";
        m_historyTable->setItem(i, 1, new QTableWidgetItem(resourceTitle));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(loan->getBorrowDate().toString("yyyy-MM-dd")));
        m_historyTable->setItem(i, 3, new QTableWidgetItem(loan->getReturnDate().toString("yyyy-MM-dd")));
    }
}
void UserLoansDialog::onLoanSelectionChanged() {
    QTableWidget* senderTable = qobject_cast<QTableWidget*>(sender());
    if (!senderTable) return;
    int selectedRow = senderTable->currentRow();
    if (selectedRow < 0) {
        m_selectedLoanId.clear();
        updateLoanDetails();
        updateButtonStates();
        return;
    }
    if (senderTable == m_currentLoansTable) {
        m_historyTable->clearSelection();
        if (selectedRow < static_cast<int>(m_currentLoans.size())) {
            m_selectedLoanId = m_currentLoans[selectedRow]->getId();
        }
    } else if (senderTable == m_historyTable) {
        m_currentLoansTable->clearSelection();
        if (selectedRow < static_cast<int>(m_loanHistory.size())) {
            m_selectedLoanId = m_loanHistory[selectedRow]->getId();
        }
    }
    updateLoanDetails();
    updateButtonStates();
}
void UserLoansDialog::updateLoanDetails() {
    if (m_selectedLoanId.isEmpty()) {
        m_detailsText->setPlainText("Select a loan to view details...");
        return;
    }
    const Loan* selectedLoan = nullptr;
    for (const auto& loan : m_currentLoans) {
        if (loan->getId() == m_selectedLoanId) {
            selectedLoan = loan.get();
            break;
        }
    }
    if (!selectedLoan) {
        for (const auto& loan : m_loanHistory) {
            if (loan->getId() == m_selectedLoanId) {
                selectedLoan = loan.get();
                break;
            }
        }
    }
    if (!selectedLoan) {
        m_detailsText->setPlainText("Loan details not available.");
        return;
    }
    Resource* resource = m_libraryManager->findResourceById(selectedLoan->getResourceId());
    QString resourceInfo = resource ? 
        QString("%1 by %2 (%3)").arg(resource->getTitle(), resource->getAuthor()).arg(resource->getPublicationYear()) :
        "Unknown Resource";
    QString details;
    details += QString("Loan ID: %1\n").arg(selectedLoan->getId());
    details += QString("Resource: %1\n").arg(resourceInfo);
    details += QString("Borrow Date: %1\n").arg(selectedLoan->getBorrowDate().toString("yyyy-MM-dd hh:mm"));
    details += QString("Due Date: %1\n").arg(selectedLoan->getDueDate().toString("yyyy-MM-dd hh:mm"));
    if (selectedLoan->isReturned()) {
        details += QString("Return Date: %1\n").arg(selectedLoan->getReturnDate().toString("yyyy-MM-dd hh:mm"));
        details += QString("Status: Returned\n");
    } else {
        details += QString("Status: %1\n").arg(formatLoanStatus(*selectedLoan));
        if (selectedLoan->isOverdue()) {
            int daysOverdue = calculateDaysOverdue(selectedLoan->getDueDate());
            details += QString("Days Overdue: %1\n").arg(daysOverdue);
        }
    }
    if (selectedLoan->getRenewalCount() > 0) {
        details += QString("Renewals: %1\n").arg(selectedLoan->getRenewalCount());
    }
    m_detailsText->setPlainText(details);
}
void UserLoansDialog::updateButtonStates() {
    bool hasCurrentLoanSelected = false;
    if (!m_selectedLoanId.isEmpty()) {
        for (const auto& loan : m_currentLoans) {
            if (loan->getId() == m_selectedLoanId) {
                hasCurrentLoanSelected = true;
                break;
            }
        }
    }
    m_returnBtn->setEnabled(hasCurrentLoanSelected);
    m_renewBtn->setEnabled(hasCurrentLoanSelected);
}
void UserLoansDialog::onReturnLoan() {
    if (m_selectedLoanId.isEmpty()) return;
    try {
        if (m_libraryManager->returnResource(m_selectedLoanId)) {
            QMessageBox::information(this, "Success", "Loan returned successfully!");
            onRefresh();
        } else {
            QMessageBox::warning(this, "Error", "Failed to return loan.");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Error returning loan: %1").arg(e.what()));
    }
}
void UserLoansDialog::onRenewLoan() {
    if (m_selectedLoanId.isEmpty()) return;
    try {
        QDateTime newDueDate = m_libraryManager->renewLoan(m_selectedLoanId);
        if (!newDueDate.isNull()) {
            QMessageBox::information(this, "Success", 
                QString("Loan renewed successfully! New due date: %1").arg(newDueDate.toString("yyyy-MM-dd")));
            onRefresh();
        } else {
            QMessageBox::warning(this, "Error", "Failed to renew loan.");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Error renewing loan: %1").arg(e.what()));
    }
}
void UserLoansDialog::onRefresh() {
    loadLoanData();
    if (m_user) {
        m_currentLoansCountLabel->setText(QString("<b>Current Loans:</b> %1 / %2").arg(m_user->getCurrentLoans().size()).arg(m_user->getMaxLoans()));
    }
}
QString UserLoansDialog::formatLoanStatus(const Loan& loan) const {
    if (loan.isReturned()) {
        return "Returned";
    } else if (loan.isOverdue()) {
        return "Overdue";
    } else {
        return "Active";
    }
}
QString UserLoansDialog::formatDueDate(const QDateTime& dueDate) const {
    QString formatted = dueDate.toString("yyyy-MM-dd");
    if (dueDate < QDateTime::currentDateTime()) {
        formatted += " (OVERDUE)";
    }
    return formatted;
}
int UserLoansDialog::calculateDaysOverdue(const QDateTime& dueDate) const {
    if (dueDate >= QDateTime::currentDateTime()) {
        return 0;
    }
    return dueDate.daysTo(QDateTime::currentDateTime());
}