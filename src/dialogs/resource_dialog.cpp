#include "resource_dialog.h"
#include "../models/thesis.h"
#include "../models/digitalcontent.h"
#include <QMessageBox>
#include <QDateTime>
#include <QUuid>
ResourceDialog::ResourceDialog(Mode mode, QWidget* parent)
    : QDialog(parent), m_mode(mode), m_originalResource(nullptr) {
    setWindowTitle(mode == Mode::Add ? "Add New Resource" : "Edit Resource");
    setModal(true);
    resize(500, 600);
    setupUI();
    setupConnections();
    setupValidation();
    if (mode == Mode::Add) {
        m_idEdit->setText("RES_" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper());
        m_yearSpinBox->setValue(QDateTime::currentDateTime().date().year());
    }
}
ResourceDialog::ResourceDialog(Resource* resource, QWidget* parent)
    : QDialog(parent), m_mode(Mode::Edit), m_originalResource(resource) {
    setWindowTitle("Edit Resource");
    setModal(true);
    resize(500, 600);
    setupUI();
    setupConnections();
    setupValidation();
    if (resource) {
        populateFields(resource);
    }
}
void ResourceDialog::setupUI() {
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
    m_typeComboBox = new QComboBox();
    m_typeComboBox->addItem("Book", "Book");
    m_typeComboBox->addItem("Article", "Article");
    m_typeComboBox->addItem("Thesis", "Thesis");
    m_typeComboBox->addItem("Digital Content", "Digital Content");
    m_formLayout->addRow("Resource Type:", m_typeComboBox);
    m_idEdit = new QLineEdit();
    m_idEdit->setReadOnly(m_mode == Mode::Edit);
    m_formLayout->addRow("ID:", m_idEdit);
    m_titleEdit = new QLineEdit();
    m_titleEdit->setPlaceholderText("Enter resource title...");
    m_formLayout->addRow("Title:", m_titleEdit);
    m_authorEdit = new QLineEdit();
    m_authorEdit->setPlaceholderText("Enter author name...");
    m_formLayout->addRow("Author:", m_authorEdit);
    m_yearSpinBox = new QSpinBox();
    m_yearSpinBox->setRange(1000, QDateTime::currentDateTime().date().year());
    m_yearSpinBox->setValue(QDateTime::currentDateTime().date().year());
    m_formLayout->addRow("Publication Year:", m_yearSpinBox);
    m_categoryComboBox = new QComboBox();
    m_categoryComboBox->addItem("Book", static_cast<int>(Resource::Category::Book));
    m_categoryComboBox->addItem("Article", static_cast<int>(Resource::Category::Article));
    m_categoryComboBox->addItem("Thesis", static_cast<int>(Resource::Category::Thesis));
    m_categoryComboBox->addItem("Digital Content", static_cast<int>(Resource::Category::DigitalContent));
    m_categoryComboBox->addItem("Other", static_cast<int>(Resource::Category::Other));
    m_formLayout->addRow("Category:", m_categoryComboBox);
    m_statusComboBox = new QComboBox();
    m_statusComboBox->addItem("Available", static_cast<int>(Resource::Status::Available));
    m_statusComboBox->addItem("Borrowed", static_cast<int>(Resource::Status::Borrowed));
    m_statusComboBox->addItem("Reserved", static_cast<int>(Resource::Status::Reserved));
    m_statusComboBox->addItem("Maintenance", static_cast<int>(Resource::Status::Maintenance));
    m_statusComboBox->addItem("Lost", static_cast<int>(Resource::Status::Lost));
    m_formLayout->addRow("Status:", m_statusComboBox);
    m_descriptionEdit = new QTextEdit();
    m_descriptionEdit->setMaximumHeight(80);
    m_descriptionEdit->setPlaceholderText("Enter description (optional)...");
    m_formLayout->addRow("Description:", m_descriptionEdit);
    m_bookGroup = new QGroupBox("Book Details");
    QFormLayout* bookLayout = new QFormLayout(m_bookGroup);
    m_isbnEdit = new QLineEdit();
    m_isbnEdit->setPlaceholderText("Enter ISBN...");
    bookLayout->addRow("ISBN:", m_isbnEdit);
    m_pagesSpinBox = new QSpinBox();
    m_pagesSpinBox->setRange(1, 9999);
    m_pagesSpinBox->setValue(100);
    bookLayout->addRow("Pages:", m_pagesSpinBox);
    m_publisherEdit = new QLineEdit();
    m_publisherEdit->setPlaceholderText("Enter publisher...");
    bookLayout->addRow("Publisher:", m_publisherEdit);
    m_editionEdit = new QLineEdit();
    m_editionEdit->setPlaceholderText("Enter edition...");
    bookLayout->addRow("Edition:", m_editionEdit);
    m_languageEdit = new QLineEdit();
    m_languageEdit->setPlaceholderText("Enter language...");
    m_languageEdit->setText("English");
    bookLayout->addRow("Language:", m_languageEdit);
    m_articleGroup = new QGroupBox("Article Details");
    QFormLayout* articleLayout = new QFormLayout(m_articleGroup);
    m_journalEdit = new QLineEdit();
    m_journalEdit->setPlaceholderText("Enter journal name...");
    articleLayout->addRow("Journal:", m_journalEdit);
    m_volumeEdit = new QLineEdit();
    m_volumeEdit->setPlaceholderText("Enter volume...");
    articleLayout->addRow("Volume:", m_volumeEdit);
    m_issueEdit = new QLineEdit();
    m_issueEdit->setPlaceholderText("Enter issue...");
    articleLayout->addRow("Issue:", m_issueEdit);
    m_pageRangeEdit = new QLineEdit();
    m_pageRangeEdit->setPlaceholderText("e.g., 123-145");
    articleLayout->addRow("Page Range:", m_pageRangeEdit);
      m_doiEdit = new QLineEdit();
    m_doiEdit->setPlaceholderText("Enter DOI...");
    articleLayout->addRow("DOI:", m_doiEdit);
    m_thesisGroup = new QGroupBox("Thesis Details");
    QFormLayout* thesisLayout = new QFormLayout(m_thesisGroup);
    m_supervisorEdit = new QLineEdit();
    m_supervisorEdit->setPlaceholderText("Enter supervisor name...");
    thesisLayout->addRow("Supervisor:", m_supervisorEdit);
    m_universityEdit = new QLineEdit();
    m_universityEdit->setPlaceholderText("Enter university name...");
    thesisLayout->addRow("University:", m_universityEdit);
    m_departmentEdit = new QLineEdit();
    m_departmentEdit->setPlaceholderText("Enter department...");
    thesisLayout->addRow("Department:", m_departmentEdit);
    m_degreeLevelComboBox = new QComboBox();
    m_degreeLevelComboBox->addItem("Bachelor's", static_cast<int>(Thesis::DegreeLevel::Bachelors));
    m_degreeLevelComboBox->addItem("Master's", static_cast<int>(Thesis::DegreeLevel::Masters));
    m_degreeLevelComboBox->addItem("PhD", static_cast<int>(Thesis::DegreeLevel::PhD));
    m_degreeLevelComboBox->addItem("Postdoc", static_cast<int>(Thesis::DegreeLevel::Postdoc));
    thesisLayout->addRow("Degree Level:", m_degreeLevelComboBox);
    m_keywordsEdit = new QLineEdit();
    m_keywordsEdit->setPlaceholderText("Enter keywords (comma-separated)...");
    thesisLayout->addRow("Keywords:", m_keywordsEdit);
    m_digitalGroup = new QGroupBox("Digital Content Details");
    QFormLayout* digitalLayout = new QFormLayout(m_digitalGroup);
    m_contentTypeComboBox = new QComboBox();
    m_contentTypeComboBox->addItem("E-Book", static_cast<int>(DigitalContent::ContentType::EBook));
    m_contentTypeComboBox->addItem("Audio Book", static_cast<int>(DigitalContent::ContentType::AudioBook));
    m_contentTypeComboBox->addItem("Video", static_cast<int>(DigitalContent::ContentType::Video));
    m_contentTypeComboBox->addItem("Document", static_cast<int>(DigitalContent::ContentType::Document));
    m_contentTypeComboBox->addItem("Software", static_cast<int>(DigitalContent::ContentType::Software));
    m_contentTypeComboBox->addItem("Database", static_cast<int>(DigitalContent::ContentType::Database));
    m_contentTypeComboBox->addItem("Web Resource", static_cast<int>(DigitalContent::ContentType::WebResource));
    digitalLayout->addRow("Content Type:", m_contentTypeComboBox);
    m_accessTypeComboBox = new QComboBox();
    m_accessTypeComboBox->addItem("Online", static_cast<int>(DigitalContent::AccessType::Online));
    m_accessTypeComboBox->addItem("Download", static_cast<int>(DigitalContent::AccessType::Download));
    m_accessTypeComboBox->addItem("Streaming", static_cast<int>(DigitalContent::AccessType::Streaming));
    digitalLayout->addRow("Access Type:", m_accessTypeComboBox);
    m_fileFormatEdit = new QLineEdit();
    m_fileFormatEdit->setPlaceholderText("e.g., PDF, MP4, ZIP...");
    digitalLayout->addRow("File Format:", m_fileFormatEdit);
    m_fileSizeEdit = new QLineEdit();
    m_fileSizeEdit->setPlaceholderText("e.g., 15 MB, 2.5 GB...");
    digitalLayout->addRow("File Size:", m_fileSizeEdit);
    m_urlEdit = new QLineEdit();
    m_urlEdit->setPlaceholderText("Enter URL for online content...");
    digitalLayout->addRow("URL:", m_urlEdit);
    m_platformEdit = new QLineEdit();
    m_platformEdit->setPlaceholderText("e.g., Windows, Web Browser...");
    digitalLayout->addRow("Platform:", m_platformEdit);
    m_authenticationCheckBox = new QCheckBox("Requires Authentication");
    digitalLayout->addRow("Access:", m_authenticationCheckBox);
    m_simultaneousUsersSpinBox = new QSpinBox();
    m_simultaneousUsersSpinBox->setRange(1, 9999);
    m_simultaneousUsersSpinBox->setValue(1);
    m_simultaneousUsersSpinBox->setToolTip("Number of users who can access this content simultaneously");
    digitalLayout->addRow("Simultaneous Users:", m_simultaneousUsersSpinBox);
    m_systemRequirementsEdit = new QLineEdit();
    m_systemRequirementsEdit->setPlaceholderText("Enter system requirements...");
    digitalLayout->addRow("System Requirements:", m_systemRequirementsEdit);
    m_mainLayout->addLayout(m_formLayout);
    m_mainLayout->addWidget(m_bookGroup);
    m_mainLayout->addWidget(m_articleGroup);
    m_mainLayout->addWidget(m_thesisGroup);
    m_mainLayout->addWidget(m_digitalGroup);
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_mainLayout->addWidget(m_buttonBox);
    updateFieldVisibility();
}
void ResourceDialog::setupConnections() {
    connect(m_typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ResourceDialog::onResourceTypeChanged);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &ResourceDialog::onAccepted);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
void ResourceDialog::setupValidation() {
    QString requiredStyle = "QLineEdit { border: 2px solid #ff6b6b; }";
    QString normalStyle = "QLineEdit { border: 1px solid #ccc; }";
    connect(m_titleEdit, &QLineEdit::textChanged, [this, normalStyle](const QString& text) {
        if (!text.isEmpty()) {
            m_titleEdit->setStyleSheet(normalStyle);
        }
    });
    connect(m_authorEdit, &QLineEdit::textChanged, [this, normalStyle](const QString& text) {
        if (!text.isEmpty()) {
            m_authorEdit->setStyleSheet(normalStyle);
        }
    });
}
void ResourceDialog::onResourceTypeChanged() {
    updateFieldVisibility();
    QString type = m_typeComboBox->currentData().toString();
    if (type == "Book") {
        m_categoryComboBox->setCurrentIndex(0); 
    } else if (type == "Article") {
        m_categoryComboBox->setCurrentIndex(1); 
    }
}
void ResourceDialog::updateFieldVisibility() {
    QString type = m_typeComboBox->currentData().toString();
    bool isBook = (type == "Book");
    bool isArticle = (type == "Article");
    bool isThesis = (type == "Thesis");
    bool isDigital = (type == "Digital Content");
    m_bookGroup->setVisible(isBook);
    m_articleGroup->setVisible(isArticle);
    m_thesisGroup->setVisible(isThesis);
    m_digitalGroup->setVisible(isDigital);
    if (isBook) {
        resize(500, 700);
    } else if (isArticle) {
        resize(500, 650);
    } else if (isThesis) {
        resize(500, 650);
    } else if (isDigital) {
        resize(500, 750);
    } else {
        resize(500, 450);
    }
}
void ResourceDialog::onAccepted() {
    validateAndAccept();
}
void ResourceDialog::validateAndAccept() {
    if (validateInput()) {
        accept();
    }
}
bool ResourceDialog::validateInput() {
    QString requiredStyle = "QLineEdit { border: 2px solid #ff6b6b; }";
    bool isValid = true;
    if (m_titleEdit->text().trimmed().isEmpty()) {
        m_titleEdit->setStyleSheet(requiredStyle);
        showValidationError("Title is required.");
        isValid = false;
    }
    if (m_authorEdit->text().trimmed().isEmpty()) {
        m_authorEdit->setStyleSheet(requiredStyle);
        showValidationError("Author is required.");
        isValid = false;
    }
    if (m_idEdit->text().trimmed().isEmpty()) {
        m_idEdit->setStyleSheet(requiredStyle);
        showValidationError("ID is required.");
        isValid = false;
    }
    QString type = m_typeComboBox->currentData().toString();
    if (type == "Book") {
        if (m_isbnEdit->text().trimmed().isEmpty()) {
            m_isbnEdit->setStyleSheet(requiredStyle);
            showValidationError("ISBN is required for books.");
            isValid = false;
        }
        if (m_publisherEdit->text().trimmed().isEmpty()) {
            m_publisherEdit->setStyleSheet(requiredStyle);
            showValidationError("Publisher is required for books.");
            isValid = false;
        }
    } else if (type == "Article") {
        if (m_journalEdit->text().trimmed().isEmpty()) {
            m_journalEdit->setStyleSheet(requiredStyle);
            showValidationError("Journal name is required for articles.");
            isValid = false;
        }
    } else if (type == "Thesis") {
        if (m_supervisorEdit->text().trimmed().isEmpty()) {
            m_supervisorEdit->setStyleSheet(requiredStyle);
            showValidationError("Supervisor is required for thesis.");
            isValid = false;
        }
        if (m_universityEdit->text().trimmed().isEmpty()) {
            m_universityEdit->setStyleSheet(requiredStyle);
            showValidationError("University is required for thesis.");
            isValid = false;
        }
    } else if (type == "Digital Content") {
        if (m_fileFormatEdit->text().trimmed().isEmpty()) {
            m_fileFormatEdit->setStyleSheet(requiredStyle);
            showValidationError("File format is required for digital content.");
            isValid = false;
        }
    }
    return isValid;
}
void ResourceDialog::showValidationError(const QString& message) {
    QMessageBox::warning(this, "Validation Error", message);
}
void ResourceDialog::populateFields(Resource* resource) {
    if (!resource) return;
    m_idEdit->setText(resource->getId());
    m_titleEdit->setText(resource->getTitle());
    m_authorEdit->setText(resource->getAuthor());
    m_yearSpinBox->setValue(resource->getPublicationYear());
    int categoryIndex = m_categoryComboBox->findData(static_cast<int>(resource->getCategory()));
    if (categoryIndex >= 0) {
        m_categoryComboBox->setCurrentIndex(categoryIndex);
    }
    int statusIndex = m_statusComboBox->findData(static_cast<int>(resource->getStatus()));
    if (statusIndex >= 0) {
        m_statusComboBox->setCurrentIndex(statusIndex);
    }
    m_descriptionEdit->setPlainText(resource->getDescription());
    if (Book* book = dynamic_cast<Book*>(resource)) {
        m_typeComboBox->setCurrentText("Book");
        m_isbnEdit->setText(book->getIsbn());
        m_pagesSpinBox->setValue(book->getPages());
        m_publisherEdit->setText(book->getPublisher());
        m_editionEdit->setText(book->getEdition());
        m_languageEdit->setText(book->getLanguage());
    } else if (Article* article = dynamic_cast<Article*>(resource)) {
        m_typeComboBox->setCurrentText("Article");
        m_journalEdit->setText(article->getJournal());
        m_volumeEdit->setText(article->getVolume());
        m_issueEdit->setText(article->getIssue());
        m_pageRangeEdit->setText(article->getPageRange());
        m_doiEdit->setText(article->getDoi());
    } else if (Thesis* thesis = dynamic_cast<Thesis*>(resource)) {
        m_typeComboBox->setCurrentText("Thesis");
        m_supervisorEdit->setText(thesis->getSupervisor());
        m_universityEdit->setText(thesis->getUniversity());
        m_departmentEdit->setText(thesis->getDepartment());
        int degreeLevelIndex = m_degreeLevelComboBox->findData(static_cast<int>(thesis->getDegreeLevel()));        if (degreeLevelIndex >= 0) {
            m_degreeLevelComboBox->setCurrentIndex(degreeLevelIndex);
        }
        m_keywordsEdit->setText(thesis->getKeywords());
    } else if (DigitalContent* digital = dynamic_cast<DigitalContent*>(resource)) {
        m_typeComboBox->setCurrentText("Digital Content");
        int contentTypeIndex = m_contentTypeComboBox->findData(static_cast<int>(digital->getContentType()));
        if (contentTypeIndex >= 0) {
            m_contentTypeComboBox->setCurrentIndex(contentTypeIndex);
        }
        int accessTypeIndex = m_accessTypeComboBox->findData(static_cast<int>(digital->getAccessType()));
        if (accessTypeIndex >= 0) {
            m_accessTypeComboBox->setCurrentIndex(accessTypeIndex);
        }
          m_fileFormatEdit->setText(digital->getFileFormat());
        m_fileSizeEdit->setText(digital->getFileSize());
        m_urlEdit->setText(digital->getUrl().toString());
        m_platformEdit->setText(digital->getPlatform());
        m_authenticationCheckBox->setChecked(digital->requiresAuthentication());
        m_simultaneousUsersSpinBox->setValue(digital->getSimultaneousUsers());
        m_systemRequirementsEdit->setText(digital->getSystemRequirements());
    }else if (Thesis* thesis = dynamic_cast<Thesis*>(resource)) {
        m_typeComboBox->setCurrentText("Thesis");
        m_supervisorEdit->setText(thesis->getSupervisor());
        m_universityEdit->setText(thesis->getUniversity());        m_departmentEdit->setText(thesis->getDepartment());
        m_degreeLevelComboBox->setCurrentIndex(static_cast<int>(thesis->getDegreeLevel()));
        m_keywordsEdit->setText(thesis->getKeywords());
    } else if (DigitalContent* digital = dynamic_cast<DigitalContent*>(resource)) {
        m_typeComboBox->setCurrentText("Digital Content");
        m_contentTypeComboBox->setCurrentIndex(static_cast<int>(digital->getContentType()));
        m_accessTypeComboBox->setCurrentIndex(static_cast<int>(digital->getAccessType()));        m_fileFormatEdit->setText(digital->getFileFormat());
        m_fileSizeEdit->setText(digital->getFileSize());
        m_urlEdit->setText(digital->getUrl().toString());
        m_platformEdit->setText(digital->getPlatform());
        m_authenticationCheckBox->setChecked(digital->requiresAuthentication());
        m_simultaneousUsersSpinBox->setValue(digital->getSimultaneousUsers());
        m_systemRequirementsEdit->setText(digital->getSystemRequirements());
    }
    updateFieldVisibility();
}
std::unique_ptr<Resource> ResourceDialog::getResource() const {
    if (!m_resultResource) {
        QString type = m_typeComboBox->currentData().toString();
        if (type == "Book") {
            m_resultResource = createBook();
        } else if (type == "Article") {
            m_resultResource = createArticle();
        } else if (type == "Thesis") {
            m_resultResource = createThesis();
        } else if (type == "Digital Content") {
            m_resultResource = createDigitalContent();
        }
    }
    return std::move(m_resultResource);
}
std::unique_ptr<Resource> ResourceDialog::createBook() const {
    auto book = std::make_unique<Book>(
        m_idEdit->text().trimmed(),
        m_titleEdit->text().trimmed(),
        m_authorEdit->text().trimmed(),
        m_yearSpinBox->value(),
        m_isbnEdit->text().trimmed(),
        m_publisherEdit->text().trimmed(),
        m_pagesSpinBox->value()
    );
    book->setEdition(m_editionEdit->text().trimmed());
    book->setLanguage(m_languageEdit->text().trimmed());
    book->setCategory(static_cast<Resource::Category>(m_categoryComboBox->currentData().toInt()));
    book->setStatus(static_cast<Resource::Status>(m_statusComboBox->currentData().toInt()));
    book->setDescription(m_descriptionEdit->toPlainText().trimmed());
    return std::move(book);
}
std::unique_ptr<Resource> ResourceDialog::createArticle() const {
    auto article = std::make_unique<Article>(
        m_idEdit->text().trimmed(),
        m_titleEdit->text().trimmed(),
        m_authorEdit->text().trimmed(),
        m_yearSpinBox->value(),
        m_journalEdit->text().trimmed(),
        m_volumeEdit->text().trimmed().toInt(),
        m_issueEdit->text().trimmed().toInt()
    );
    article->setPageRange(m_pageRangeEdit->text().trimmed());
    article->setDoi(m_doiEdit->text().trimmed());
    article->setCategory(static_cast<Resource::Category>(m_categoryComboBox->currentData().toInt()));
    article->setStatus(static_cast<Resource::Status>(m_statusComboBox->currentData().toInt()));
    article->setDescription(m_descriptionEdit->toPlainText().trimmed());
    return std::move(article);
}
std::unique_ptr<Resource> ResourceDialog::createThesis() const {    auto thesis = std::make_unique<Thesis>(
        m_idEdit->text().trimmed(),
        m_titleEdit->text().trimmed(),
        m_authorEdit->text().trimmed(),
        m_yearSpinBox->value()
    );
    thesis->setSupervisor(m_supervisorEdit->text().trimmed());
    thesis->setUniversity(m_universityEdit->text().trimmed());
    thesis->setDepartment(m_departmentEdit->text().trimmed());
    thesis->setDegreeLevel(static_cast<Thesis::DegreeLevel>(m_degreeLevelComboBox->currentData().toInt()));
    QString keywordsText = m_keywordsEdit->text().trimmed();
    if (!keywordsText.isEmpty()) {
        thesis->setKeywords(keywordsText);
    }
    thesis->setCategory(static_cast<Resource::Category>(m_categoryComboBox->currentData().toInt()));
    thesis->setStatus(static_cast<Resource::Status>(m_statusComboBox->currentData().toInt()));
    thesis->setDescription(m_descriptionEdit->toPlainText().trimmed());
    return std::move(thesis);
}
std::unique_ptr<Resource> ResourceDialog::createDigitalContent() const {    auto digital = std::make_unique<DigitalContent>(
        m_idEdit->text().trimmed(),
        m_titleEdit->text().trimmed(),
        m_authorEdit->text().trimmed(),
        m_yearSpinBox->value()
    );
    digital->setContentType(static_cast<DigitalContent::ContentType>(m_contentTypeComboBox->currentData().toInt()));
    digital->setAccessType(static_cast<DigitalContent::AccessType>(m_accessTypeComboBox->currentData().toInt()));
    digital->setFileFormat(m_fileFormatEdit->text().trimmed());
    digital->setFileSize(m_fileSizeEdit->text().trimmed());
    digital->setUrl(QUrl(m_urlEdit->text().trimmed()));
    digital->setPlatform(m_platformEdit->text().trimmed());
    digital->setRequiresAuthentication(m_authenticationCheckBox->isChecked());
    digital->setSimultaneousUsers(m_simultaneousUsersSpinBox->value());
    digital->setSystemRequirements(m_systemRequirementsEdit->text().trimmed());
    digital->setCategory(static_cast<Resource::Category>(m_categoryComboBox->currentData().toInt()));
    digital->setStatus(static_cast<Resource::Status>(m_statusComboBox->currentData().toInt()));
    digital->setDescription(m_descriptionEdit->toPlainText().trimmed());
    return std::move(digital);
}