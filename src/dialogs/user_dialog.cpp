#include "user_dialog.h"
#include "../models/loan.h"
#include <QMessageBox>
#include <QDateTime>
#include <QUuid>
#include <QRegularExpression>
UserDialog::UserDialog(Mode mode, QWidget* parent)
    : QDialog(parent), m_mode(mode), m_originalUser(nullptr) {
    setWindowTitle(mode == Mode::Add ? "Add New User" : "Edit User");
    setModal(true);
    resize(450, 500);
      setupUI();
    setupConnections();
    setupValidation();
    onUserTypeChanged();
    if (mode == Mode::Add) {
        m_idEdit->setText("USER_" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper());
    }
}
UserDialog::UserDialog(User* user, QWidget* parent)
    : QDialog(parent), m_mode(Mode::Edit), m_originalUser(user) {
    setWindowTitle("Edit User");
    setModal(true);
    resize(450, 500);
      setupUI();
    setupConnections();
    setupValidation();
    onUserTypeChanged();
    if (user) {
        populateFields(user);
    }
}
void UserDialog::setupUI() {
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
        QLineEdit, QSpinBox, QDateEdit, QComboBox, QTextEdit {
            background-color: #ffffff;
            border: 2px solid #d1d5db;
            border-radius: 4px;
            padding: 8px;
            font-size: 12px;
            color: #1e3a8a;
        }
        QLineEdit:focus, QSpinBox:focus, QDateEdit:focus, QComboBox:focus, QTextEdit:focus {
            border-color: #f97316;
            outline: none;
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
        QCheckBox {
            color: #1e3a8a;
            font-weight: bold;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #2563eb;
            border-radius: 3px;
        }
        QCheckBox::indicator:checked {
            background-color: #f97316;
            border-color: #f97316;
        }
    )";
    this->setStyleSheet(styleSheet);
    m_mainLayout = new QVBoxLayout(this);
    m_formLayout = new QFormLayout();
    m_idEdit = new QLineEdit();
    m_idEdit->setReadOnly(m_mode == Mode::Edit);
    m_formLayout->addRow("User ID:", m_idEdit);
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("Enter full name...");
    m_formLayout->addRow("Full Name:", m_nameEdit);
    QVBoxLayout* emailLayout = new QVBoxLayout();
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("Enter email address...");
    m_emailValidationLabel = new QLabel();
    m_emailValidationLabel->setStyleSheet("color: red; font-size: 10px;");
    m_emailValidationLabel->hide();
    emailLayout->addWidget(m_emailEdit);
    emailLayout->addWidget(m_emailValidationLabel);
    emailLayout->setContentsMargins(0, 0, 0, 0);
    emailLayout->setSpacing(2);
    m_formLayout->addRow("Email:", emailLayout);
    QVBoxLayout* phoneLayout = new QVBoxLayout();
    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText("Enter phone number...");
    m_phoneValidationLabel = new QLabel();
    m_phoneValidationLabel->setStyleSheet("color: red; font-size: 10px;");
    m_phoneValidationLabel->hide();
    phoneLayout->addWidget(m_phoneEdit);
    phoneLayout->addWidget(m_phoneValidationLabel);
    phoneLayout->setContentsMargins(0, 0, 0, 0);
    phoneLayout->setSpacing(2);
    m_formLayout->addRow("Phone:", phoneLayout);
    m_addressEdit = new QLineEdit();
    m_addressEdit->setPlaceholderText("Enter address (optional)...");
    m_formLayout->addRow("Address:", m_addressEdit);
    m_typeComboBox = new QComboBox();    m_typeComboBox->addItem("Student", static_cast<int>(User::UserType::Student));
    m_typeComboBox->addItem("Teacher", static_cast<int>(User::UserType::Teacher));
    m_typeComboBox->addItem("Staff", static_cast<int>(User::UserType::Staff));
    m_typeComboBox->addItem("Administrator", static_cast<int>(User::UserType::Administrator));    m_typeComboBox->addItem("Guest", static_cast<int>(User::UserType::Guest));
    m_formLayout->addRow("User Type:", m_typeComboBox);
    m_yearLabel = new QLabel("Year:");
    m_yearComboBox = new QComboBox();
    m_yearComboBox->addItem("1st Year", 1);
    m_yearComboBox->addItem("2nd Year", 2);
    m_yearComboBox->addItem("3rd Year", 3);
    m_yearComboBox->addItem("4th Year", 4);
    m_yearComboBox->addItem("5th Year", 5);
    m_formLayout->addRow(m_yearLabel, m_yearComboBox);
    m_yearLabel->setVisible(false);
    m_yearComboBox->setVisible(false);
    m_statusComboBox = new QComboBox();
    m_statusComboBox->addItem("Active", static_cast<int>(User::Status::Active));
    m_statusComboBox->addItem("Inactive", static_cast<int>(User::Status::Inactive));
    m_statusComboBox->addItem("Suspended", static_cast<int>(User::Status::Suspended));
    m_formLayout->addRow("Status:", m_statusComboBox);
    m_maxLoansSpinBox = new QSpinBox();
    m_maxLoansSpinBox->setRange(1, 50);
    m_maxLoansSpinBox->setValue(5);
    m_formLayout->addRow("Max Loans:", m_maxLoansSpinBox);
    m_notesEdit = new QTextEdit();
    m_notesEdit->setMaximumHeight(80);
    m_notesEdit->setPlaceholderText("Enter additional notes (optional)...");
    m_formLayout->addRow("Notes:", m_notesEdit);
    m_mainLayout->addLayout(m_formLayout);
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_mainLayout->addWidget(m_buttonBox);
}
void UserDialog::setupConnections() {
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &UserDialog::onAccepted);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_emailEdit, &QLineEdit::textChanged, this, &UserDialog::onEmailChanged);
    connect(m_phoneEdit, &QLineEdit::textChanged, this, &UserDialog::onPhoneChanged);
    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UserDialog::onUserTypeChanged);
}
void UserDialog::setupValidation() {
    QString requiredStyle = "QLineEdit { border: 2px solid #ff6b6b; }";
    QString normalStyle = "QLineEdit { border: 1px solid #ccc; }";
    QString validStyle = "QLineEdit { border: 2px solid #4CAF50; }";
    connect(m_nameEdit, &QLineEdit::textChanged, [this, normalStyle](const QString& text) {
        if (!text.isEmpty()) {
            m_nameEdit->setStyleSheet(normalStyle);
        }
    });
    connect(m_idEdit, &QLineEdit::textChanged, [this, normalStyle](const QString& text) {
        if (!text.isEmpty()) {
            m_idEdit->setStyleSheet(normalStyle);
        }
    });
}
void UserDialog::onEmailChanged() {
    QString email = m_emailEdit->text().trimmed();
    QString validStyle = "QLineEdit { border: 2px solid #4CAF50; }";
    QString invalidStyle = "QLineEdit { border: 2px solid #ff6b6b; }";
    QString normalStyle = "QLineEdit { border: 1px solid #ccc; }";
    if (email.isEmpty()) {
        m_emailEdit->setStyleSheet(normalStyle);
        m_emailValidationLabel->hide();
    } else if (isValidEmail(email)) {
        m_emailEdit->setStyleSheet(validStyle);
        m_emailValidationLabel->hide();
    } else {
        m_emailEdit->setStyleSheet(invalidStyle);
        m_emailValidationLabel->setText("Invalid email format");
        m_emailValidationLabel->show();
    }
}
void UserDialog::onPhoneChanged() {
    QString phone = m_phoneEdit->text().trimmed();
    QString validStyle = "QLineEdit { border: 2px solid #4CAF50; }";
    QString invalidStyle = "QLineEdit { border: 2px solid #ff6b6b; }";
    QString normalStyle = "QLineEdit { border: 1px solid #ccc; }";
    if (phone.isEmpty()) {
        m_phoneEdit->setStyleSheet(normalStyle);
        m_phoneValidationLabel->hide();
    } else if (isValidPhone(phone)) {
        m_phoneEdit->setStyleSheet(validStyle);
        m_phoneValidationLabel->hide();
    } else {
        m_phoneEdit->setStyleSheet(invalidStyle);
        m_phoneValidationLabel->setText("Invalid phone format");
        m_phoneValidationLabel->show();
    }
}
void UserDialog::onAccepted() {
    validateAndAccept();
}
void UserDialog::validateAndAccept() {
    if (validateInput()) {
        accept();
    }
}
bool UserDialog::validateInput() {
    QString requiredStyle = "QLineEdit { border: 2px solid #ff6b6b; }";
    bool isValid = true;
    if (m_idEdit->text().trimmed().isEmpty()) {
        m_idEdit->setStyleSheet(requiredStyle);
        showValidationError("User ID is required.");
        isValid = false;
    }
    if (m_nameEdit->text().trimmed().isEmpty()) {
        m_nameEdit->setStyleSheet(requiredStyle);
        showValidationError("Full name is required.");
        isValid = false;
    }
    if (m_emailEdit->text().trimmed().isEmpty()) {
        m_emailEdit->setStyleSheet(requiredStyle);
        showValidationError("Email address is required.");
        isValid = false;
    } else if (!isValidEmail(m_emailEdit->text().trimmed())) {
        m_emailEdit->setStyleSheet(requiredStyle);
        showValidationError("Please enter a valid email address.");
        isValid = false;
    }
    if (!m_phoneEdit->text().trimmed().isEmpty() && !isValidPhone(m_phoneEdit->text().trimmed())) {
        m_phoneEdit->setStyleSheet(requiredStyle);
        showValidationError("Please enter a valid phone number.");
        isValid = false;
    }
    return isValid;
}
bool UserDialog::isValidEmail(const QString& email) const {
    QRegularExpression emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return emailRegex.match(email).hasMatch();
}
bool UserDialog::isValidPhone(const QString& phone) const {
    QRegularExpression phoneRegex(R"(^[\+]?[1-9][\d]{0,15}$|^\(?\d{3}\)?[-.\s]?\d{3}[-.\s]?\d{4}$)");
    return phoneRegex.match(phone.simplified()).hasMatch();
}
void UserDialog::showValidationError(const QString& message) {
    QMessageBox::warning(this, "Validation Error", message);
}
void UserDialog::populateFields(User* user) {
    if (!user) return;
    m_idEdit->setText(user->getId());
    m_nameEdit->setText(user->getName());
    m_emailEdit->setText(user->getEmail());
    m_phoneEdit->setText(user->getPhone());
    m_addressEdit->setText(user->getAddress());
    int typeIndex = m_typeComboBox->findData(static_cast<int>(user->getType()));
    if (typeIndex >= 0) {
        m_typeComboBox->setCurrentIndex(typeIndex);
    }
    int statusIndex = m_statusComboBox->findData(static_cast<int>(user->getStatus()));
    if (statusIndex >= 0) {
        m_statusComboBox->setCurrentIndex(statusIndex);
    }
      m_maxLoansSpinBox->setValue(user->getMaxLoans());
    m_notesEdit->setPlainText(user->getNotes());
    if (user->getType() == User::UserType::Student && user->getYear() > 0) {
        int yearIndex = m_yearComboBox->findData(user->getYear());
        if (yearIndex >= 0) {
            m_yearComboBox->setCurrentIndex(yearIndex);
        }
    }
    onUserTypeChanged();
}
std::unique_ptr<User> UserDialog::getUser() const {
    if (!m_resultUser) {
        QString fullName = m_nameEdit->text().trimmed();
        QStringList nameParts = fullName.split(' ', Qt::SkipEmptyParts);
        QString firstName = nameParts.isEmpty() ? "" : nameParts.first();
        QString lastName = nameParts.size() > 1 ? nameParts.mid(1).join(' ') : "";
        m_resultUser = std::make_unique<User>(
            m_idEdit->text().trimmed(),
            firstName,
            lastName,
            m_emailEdit->text().trimmed(),
            static_cast<User::UserType>(m_typeComboBox->currentData().toInt())
        );        
        m_resultUser->setPhone(m_phoneEdit->text().trimmed());
        m_resultUser->setAddress(m_addressEdit->text().trimmed());
        m_resultUser->setStatus(static_cast<User::Status>(m_statusComboBox->currentData().toInt()));
        m_resultUser->setMaxLoans(m_maxLoansSpinBox->value());
        m_resultUser->setNotes(m_notesEdit->toPlainText().trimmed());
        if (m_resultUser->getType() == User::UserType::Student) {
            if (m_yearComboBox->isVisible()) {
                m_resultUser->setYear(m_yearComboBox->currentData().toInt());
            } else {
                m_resultUser->setYear(1); 
            }
        }
    }
    return std::move(m_resultUser);
}
void UserDialog::onUserTypeChanged() {
    User::UserType selectedType = static_cast<User::UserType>(m_typeComboBox->currentData().toInt());
    bool isStudent = (selectedType == User::UserType::Student);
    m_yearLabel->setVisible(isStudent);
    m_yearComboBox->setVisible(isStudent);
    if (!isStudent) {
        m_yearComboBox->setCurrentIndex(0);
    }
}