#include "resource.h"
#include <stdexcept>
#include <QJsonDocument>
Resource::Resource(const QString& id, const QString& title, const QString& author, 
                   int publicationYear, Category category)
    : m_id(id), m_title(title), m_author(author), m_publicationYear(publicationYear),
      m_category(category), m_status(Status::Available), m_dateAdded(QDateTime::currentDateTime()) {
    if (id.isEmpty()) {
        throw ResourceException("Resource ID cannot be empty");
    }
    if (title.isEmpty()) {
        throw ResourceException("Resource title cannot be empty");
    }
    if (author.isEmpty()) {
        throw ResourceException("Resource author cannot be empty");
    }
    if (publicationYear < 1000 || publicationYear > QDateTime::currentDateTime().date().year()) {
        throw ResourceException("Invalid publication year");
    }
}
void Resource::setTitle(const QString& title) {
    if (title.isEmpty()) {
        throw ResourceException("Title cannot be empty");
    }
    m_title = title;
}
void Resource::setAuthor(const QString& author) {
    if (author.isEmpty()) {
        throw ResourceException("Author cannot be empty");
    }
    m_author = author;
}
void Resource::setPublicationYear(int year) {
    int currentYear = QDateTime::currentDateTime().date().year();
    if (year < 1000 || year > currentYear) {
        throw ResourceException("Invalid publication year: must be between 1000 and " + QString::number(currentYear));
    }
    m_publicationYear = year;
}
void Resource::setCategory(Category category) {
    m_category = category;
}
void Resource::setStatus(Status status) {
    m_status = status;
}
void Resource::setDescription(const QString& description) {
    m_description = description;
}
QString Resource::categoryToString(Category category) {
    switch (category) {
        case Category::Book: return "Book";
        case Category::Article: return "Article";
        case Category::Thesis: return "Thesis";
        case Category::DigitalContent: return "Digital Content";
        case Category::Other: return "Other";
        default: return "Unknown";
    }
}
Resource::Category Resource::stringToCategory(const QString& categoryStr) {
    if (categoryStr == "Book") return Category::Book;
    if (categoryStr == "Article") return Category::Article;
    if (categoryStr == "Thesis") return Category::Thesis;
    if (categoryStr == "Digital Content") return Category::DigitalContent;
    if (categoryStr == "Other") return Category::Other;
    return Category::Other; 
}
QString Resource::statusToString(Status status) {
    switch (status) {
        case Status::Available: return "Available";
        case Status::Borrowed: return "Borrowed";
        case Status::Reserved: return "Reserved";
        case Status::Maintenance: return "Maintenance";
        case Status::Lost: return "Lost";
        default: return "Unknown";
    }
}
Resource::Status Resource::stringToStatus(const QString& statusStr) {
    if (statusStr == "Available") return Status::Available;
    if (statusStr == "Borrowed") return Status::Borrowed;
    if (statusStr == "Reserved") return Status::Reserved;
    if (statusStr == "Maintenance") return Status::Maintenance;
    if (statusStr == "Lost") return Status::Lost;
    return Status::Available; 
}
bool Resource::operator==(const Resource& other) const {
    return m_id == other.m_id;
}
bool Resource::operator<(const Resource& other) const {
    return m_title < other.m_title;
}