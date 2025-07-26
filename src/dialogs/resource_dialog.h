#ifndef RESOURCE_DIALOG_H
#define RESOURCE_DIALOG_H
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QString>
#include <QMessageBox>
#include "../models/resource.h"
#include "../models/book.h"
#include "../models/article.h"
#include "../models/thesis.h"
#include "../models/digitalcontent.h"
class ResourceDialog : public QDialog {
    Q_OBJECT
public:
    enum class Mode {
        Add,
        Edit
    };
    explicit ResourceDialog(Mode mode, QWidget* parent = nullptr);
    explicit ResourceDialog(Resource* resource, QWidget* parent = nullptr);
    std::unique_ptr<Resource> getResource() const;
private slots:
    void onResourceTypeChanged();
    void onAccepted();
    void validateAndAccept();
private:
    void setupUI();
    void setupConnections();
    void setupValidation();
    void populateFields(Resource* resource);
    void updateFieldVisibility();
    bool validateInput();
    void showValidationError(const QString& message);
    std::unique_ptr<Resource> createBook() const;
    std::unique_ptr<Resource> createArticle() const;
    std::unique_ptr<Resource> createThesis() const;
    std::unique_ptr<Resource> createDigitalContent() const;
    QVBoxLayout* m_mainLayout;
    QFormLayout* m_formLayout;
    QDialogButtonBox* m_buttonBox;
    QLineEdit* m_idEdit;
    QLineEdit* m_titleEdit;
    QLineEdit* m_authorEdit;
    QSpinBox* m_yearSpinBox;
    QComboBox* m_categoryComboBox;
    QComboBox* m_statusComboBox;
    QTextEdit* m_descriptionEdit;
    QComboBox* m_typeComboBox;
    QGroupBox* m_bookGroup;
    QLineEdit* m_isbnEdit;
    QSpinBox* m_pagesSpinBox;
    QLineEdit* m_publisherEdit;
    QLineEdit* m_editionEdit;
    QLineEdit* m_languageEdit;
    QGroupBox* m_articleGroup;
    QLineEdit* m_journalEdit;
    QLineEdit* m_volumeEdit;
    QLineEdit* m_issueEdit;
    QLineEdit* m_pageRangeEdit;
    QLineEdit* m_doiEdit;
    QGroupBox* m_thesisGroup;
    QLineEdit* m_supervisorEdit;
    QLineEdit* m_universityEdit;
    QLineEdit* m_departmentEdit;
    QComboBox* m_degreeLevelComboBox;
    QLineEdit* m_keywordsEdit;
    QGroupBox* m_digitalGroup;
    QComboBox* m_contentTypeComboBox;
    QComboBox* m_accessTypeComboBox;
    QLineEdit* m_fileFormatEdit;
    QLineEdit* m_fileSizeEdit;
    QLineEdit* m_urlEdit;
    QLineEdit* m_platformEdit;
    QCheckBox* m_authenticationCheckBox;
    QSpinBox* m_simultaneousUsersSpinBox;
    QLineEdit* m_systemRequirementsEdit;
    Mode m_mode;
    Resource* m_originalResource;
    mutable std::unique_ptr<Resource> m_resultResource;
};
#endif