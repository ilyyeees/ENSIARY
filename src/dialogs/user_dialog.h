#ifndef USER_DIALOG_H
#define USER_DIALOG_H
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QDateEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QString>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include "../models/user.h"
class UserDialog : public QDialog {
    Q_OBJECT
public:
    enum class Mode {
        Add,
        Edit
    };
    explicit UserDialog(Mode mode, QWidget* parent = nullptr);
    explicit UserDialog(User* user, QWidget* parent = nullptr);
    std::unique_ptr<User> getUser() const;
private slots:
    void onAccepted();
    void validateAndAccept();
    void onEmailChanged();
    void onPhoneChanged();
    void onUserTypeChanged(); 
private:
    void setupUI();
    void setupConnections();
    void setupValidation();
    void populateFields(User* user);
    bool validateInput();
    void showValidationError(const QString& message);
    bool isValidEmail(const QString& email) const;
    bool isValidPhone(const QString& phone) const;
    QVBoxLayout* m_mainLayout;
    QFormLayout* m_formLayout;
    QDialogButtonBox* m_buttonBox;
    QLineEdit* m_idEdit;
    QLineEdit* m_nameEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_phoneEdit;
    QLineEdit* m_addressEdit;    QComboBox* m_typeComboBox;
    QComboBox* m_statusComboBox;
    QSpinBox* m_maxLoansSpinBox;
    QTextEdit* m_notesEdit;
    QComboBox* m_yearComboBox; 
    QLabel* m_yearLabel; 
    QLabel* m_emailValidationLabel;
    QLabel* m_phoneValidationLabel;
    Mode m_mode;
    User* m_originalUser;
    mutable std::unique_ptr<User> m_resultUser;
};
#endif