#ifndef DIGITALCONTENT_H
#define DIGITALCONTENT_H
#include "resource.h"
#include <QString>
#include <QDateTime>
#include <QUrl>
class DigitalContent : public Resource {
public:
    enum class ContentType {
        EBook,
        AudioBook,
        Video,
        Document,
        Software,
        Database,
        WebResource
    };
    enum class AccessType {
        Online,      
        Download,    
        Streaming    
    };
private:
    ContentType m_contentType;
    AccessType m_accessType;
    QString m_fileFormat;
    QString m_fileSize;
    QUrl m_url;
    QString m_platform;
    bool m_requiresAuthentication;
    int m_simultaneousUsers;
    QString m_systemRequirements;
public:    
    DigitalContent(const QString& id, const QString& title, const QString& author,
                  int publicationYear = 2024);
    ~DigitalContent() override = default;
    DigitalContent(const DigitalContent& other);
    DigitalContent& operator=(const DigitalContent& other);
    DigitalContent(DigitalContent&& other) noexcept = default;
    DigitalContent& operator=(DigitalContent&& other) noexcept = default;
    ContentType getContentType() const { return m_contentType; }
    AccessType getAccessType() const { return m_accessType; }
    QString getFileFormat() const { return m_fileFormat; }
    QString getFileSize() const { return m_fileSize; }
    QUrl getUrl() const { return m_url; }
    QString getPlatform() const { return m_platform; }
    bool requiresAuthentication() const { return m_requiresAuthentication; }
    int getSimultaneousUsers() const { return m_simultaneousUsers; }
    QString getSystemRequirements() const { return m_systemRequirements; }
    void setContentType(ContentType type);
    void setAccessType(AccessType type);
    void setFileFormat(const QString& format);
    void setFileSize(const QString& size);
    void setUrl(const QUrl& url);
    void setPlatform(const QString& platform);
    void setRequiresAuthentication(bool requiresAuth);
    void setSimultaneousUsers(int users);
    void setSystemRequirements(const QString& requirements);    
    QString getDetails() const override;
    QString getResourceType() const override { return "Digital Content"; }
    bool canAccommodateSimultaneousLoans() const;
    bool isCurrentlyAvailable() const;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;
    static QString contentTypeToString(ContentType type);
    static ContentType stringToContentType(const QString& typeStr);
    static QString accessTypeToString(AccessType type);
    static AccessType stringToAccessType(const QString& typeStr);
    bool isValidDigitalContent() const;
private:
    void initializeDigitalContent();
    void validateDigitalContentData() const;
};
class DigitalContentException : public std::exception {
private:
    QString m_message;
public:
    explicit DigitalContentException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};
#endif