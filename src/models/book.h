#ifndef BOOK_H
#define BOOK_H
#include "resource.h"
#include <QString>
class Book : public Resource {
private:
    QString m_isbn;
    QString m_publisher;
    int m_pageCount;
    QString m_language;
    QString m_genre;
    bool m_isHardcover;
public:
    Book(const QString& id, const QString& title, const QString& author,
         int publicationYear, const QString& isbn, const QString& publisher,
         int pageCount = 0, const QString& language = "English",
         const QString& genre = "", bool isHardcover = false);
    QString getDetails() const override;
    QString getResourceType() const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& json) override;    
    QString getIsbn() const { return m_isbn; }
    QString getPublisher() const { return m_publisher; }
    int getPageCount() const { return m_pageCount; }
    int getPages() const { return m_pageCount; } 
    QString getLanguage() const { return m_language; }
    QString getGenre() const { return m_genre; }
    QString getEdition() const { return m_genre; } 
    bool isHardcover() const { return m_isHardcover; }    
    void setIsbn(const QString& isbn);
    void setPublisher(const QString& publisher);
    void setPageCount(int pageCount);
    void setLanguage(const QString& language);
    void setGenre(const QString& genre);
    void setEdition(const QString& edition) { setGenre(edition); } 
    void setHardcover(bool isHardcover);
    QString getFormattedDetails() const;
    bool isValidIsbn(const QString& isbn) const;
private:
    void validateBookData() const;
};
#endif