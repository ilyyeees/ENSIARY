#ifndef RESOURCE_H
#define RESOURCE_H
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <memory>
class Resource {
public:
    enum class Category {
        Book,
        Article,
        Thesis,
        DigitalContent,
        Other
    };
    enum class Status {
        Available,
        Borrowed,
        Reserved,
        Maintenance,
        Lost
    };
protected:
    QString m_id;
    QString m_title;
    QString m_author;
    int m_publicationYear;
    Category m_category;
    Status m_status;
    QDateTime m_dateAdded;
    QString m_description;
public:
    Resource(const QString& id, const QString& title, const QString& author, 
             int publicationYear, Category category);
    virtual ~Resource() = default;
    virtual QString getDetails() const = 0;
    virtual QString getResourceType() const = 0;
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject& json) = 0;
    QString getId() const { return m_id; }
    QString getTitle() const { return m_title; }
    QString getAuthor() const { return m_author; }
    int getPublicationYear() const { return m_publicationYear; }
    Category getCategory() const { return m_category; }
    Status getStatus() const { return m_status; }
    QDateTime getDateAdded() const { return m_dateAdded; }
    QString getDescription() const { return m_description; }
    void setTitle(const QString& title);
    void setAuthor(const QString& author);
    void setPublicationYear(int year);
    void setCategory(Category category);
    void setStatus(Status status);
    void setDescription(const QString& description);
    bool isAvailable() const { return m_status == Status::Available; }
    bool isBorrowed() const { return m_status == Status::Borrowed; }
    bool isReserved() const { return m_status == Status::Reserved; }
    static QString categoryToString(Category category);
    static Category stringToCategory(const QString& categoryStr);
    static QString statusToString(Status status);
    static Status stringToStatus(const QString& statusStr);
    bool operator==(const Resource& other) const;
    bool operator<(const Resource& other) const;
};
class ResourceException : public std::exception {
private:
    QString m_message;
public:
    explicit ResourceException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};
#endif