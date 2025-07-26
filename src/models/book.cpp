#include "book.h"
#include <QJsonObject>
#include <QRegularExpression>
Book::Book(const QString& id, const QString& title, const QString& author,
           int publicationYear, const QString& isbn, const QString& publisher,
           int pageCount, const QString& language, const QString& genre, bool isHardcover)
    : Resource(id, title, author, publicationYear, Category::Book),
      m_isbn(isbn), m_publisher(publisher), m_pageCount(pageCount),
      m_language(language), m_genre(genre), m_isHardcover(isHardcover) {
    validateBookData();
}
QString Book::getDetails() const {
    QString details;
    details += QString("Title: %1\n").arg(getTitle());
    details += QString("Author: %1\n").arg(getAuthor());
    details += QString("ISBN: %1\n").arg(m_isbn);
    details += QString("Publisher: %1\n").arg(m_publisher);
    details += QString("Publication Year: %1\n").arg(getPublicationYear());
    details += QString("Pages: %1\n").arg(m_pageCount);
    details += QString("Language: %1\n").arg(m_language);
    details += QString("Genre: %1\n").arg(m_genre);
    details += QString("Format: %1\n").arg(m_isHardcover ? "Hardcover" : "Paperback");
    details += QString("Status: %1\n").arg(statusToString(getStatus()));
    if (!getDescription().isEmpty()) {
        details += QString("Description: %1\n").arg(getDescription());
    }
    return details;
}
QString Book::getResourceType() const {
    return "Book";
}
QJsonObject Book::toJson() const {
    QJsonObject json;
    json["id"] = getId();
    json["type"] = getResourceType();
    json["title"] = getTitle();
    json["author"] = getAuthor();
    json["publicationYear"] = getPublicationYear();
    json["category"] = categoryToString(getCategory());
    json["status"] = statusToString(getStatus());
    json["dateAdded"] = getDateAdded().toString(Qt::ISODate);
    json["description"] = getDescription();
    json["isbn"] = m_isbn;
    json["publisher"] = m_publisher;
    json["pageCount"] = m_pageCount;
    json["language"] = m_language;
    json["genre"] = m_genre;
    json["isHardcover"] = m_isHardcover;
    return json;
}
void Book::fromJson(const QJsonObject& json) {
    setTitle(json["title"].toString());
    setAuthor(json["author"].toString());
    setPublicationYear(json["publicationYear"].toInt());
    setCategory(stringToCategory(json["category"].toString()));
    setStatus(stringToStatus(json["status"].toString()));
    setDescription(json["description"].toString());
    m_isbn = json["isbn"].toString();
    m_publisher = json["publisher"].toString();
    m_pageCount = json["pageCount"].toInt();
    m_language = json["language"].toString();
    m_genre = json["genre"].toString();
    m_isHardcover = json["isHardcover"].toBool();
}
void Book::setIsbn(const QString& isbn) {
    if (!isValidIsbn(isbn)) {
        throw ResourceException("Invalid ISBN format");
    }
    m_isbn = isbn;
}
void Book::setPublisher(const QString& publisher) {
    if (publisher.isEmpty()) {
        throw ResourceException("Publisher cannot be empty");
    }
    m_publisher = publisher;
}
void Book::setPageCount(int pageCount) {
    if (pageCount < 0) {
        throw ResourceException("Page count cannot be negative");
    }
    m_pageCount = pageCount;
}
void Book::setLanguage(const QString& language) {
    m_language = language;
}
void Book::setGenre(const QString& genre) {
    m_genre = genre;
}
void Book::setHardcover(bool isHardcover) {
    m_isHardcover = isHardcover;
}
QString Book::getFormattedDetails() const {
    return QString("%1 by %2 (%3) - %4")
           .arg(getTitle())
           .arg(getAuthor())
           .arg(getPublicationYear())
           .arg(statusToString(getStatus()));
}
bool Book::isValidIsbn(const QString& isbn) const {
    if (isbn.isEmpty()) return false;
    QString cleanIsbn = isbn;
    cleanIsbn.remove('-').remove(' ');
    if (cleanIsbn.length() == 10 || cleanIsbn.length() == 13) {
        QRegularExpression isbnRegex("^\\d{9}[\\dX]$|^\\d{13}$");
        return isbnRegex.match(cleanIsbn).hasMatch();
    }
    return false;
}
void Book::validateBookData() const {
    if (!m_isbn.isEmpty() && !isValidIsbn(m_isbn)) {
        throw ResourceException("Invalid ISBN format");
    }
    if (m_publisher.isEmpty()) {
        throw ResourceException("Publisher cannot be empty");
    }
    if (m_pageCount < 0) {
        throw ResourceException("Page count cannot be negative");
    }
}