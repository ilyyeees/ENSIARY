#include "article.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
Article::Article(const QString& id, const QString& title, const QString& author,
                 int publicationYear, const QString& journal, int volume,
                 int issue, const QString& pageRange, const QString& doi,
                 const QString& researchField)
    : Resource(id, title, author, publicationYear, Category::Article),
      m_journal(journal), m_volume(volume), m_issue(issue),
      m_pageRange(pageRange), m_doi(doi), m_researchField(researchField) {
    validateArticleData();
}
QString Article::getDetails() const {
    QString details;
    details += QString("Title: %1\n").arg(getTitle());
    details += QString("Author: %1\n").arg(getAuthor());
    details += QString("Journal: %1\n").arg(m_journal);
    details += QString("Publication Year: %1\n").arg(getPublicationYear());
    if (m_volume > 0) {
        details += QString("Volume: %1\n").arg(m_volume);
    }
    if (m_issue > 0) {
        details += QString("Issue: %1\n").arg(m_issue);
    }
    if (!m_pageRange.isEmpty()) {
        details += QString("Pages: %1\n").arg(m_pageRange);
    }
    if (!m_doi.isEmpty()) {
        details += QString("DOI: %1\n").arg(m_doi);
    }
    if (!m_researchField.isEmpty()) {
        details += QString("Research Field: %1\n").arg(m_researchField);
    }
    if (!m_keywords.isEmpty()) {
        details += QString("Keywords: %1\n").arg(m_keywords.join(", "));
    }
    details += QString("Status: %1\n").arg(statusToString(getStatus()));
    if (!m_abstract.isEmpty()) {
        details += QString("Abstract: %1\n").arg(m_abstract);
    }
    if (!getDescription().isEmpty()) {
        details += QString("Description: %1\n").arg(getDescription());
    }
    return details;
}
QString Article::getResourceType() const {
    return "Article";
}
QJsonObject Article::toJson() const {
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
    json["journal"] = m_journal;
    json["volume"] = m_volume;
    json["issue"] = m_issue;
    json["pageRange"] = m_pageRange;
    json["doi"] = m_doi;
    json["abstract"] = m_abstract;
    json["researchField"] = m_researchField;
    QJsonArray keywordsArray;
    for (const QString& keyword : m_keywords) {
        keywordsArray.append(keyword);
    }
    json["keywords"] = keywordsArray;
    return json;
}
void Article::fromJson(const QJsonObject& json) {
    setTitle(json["title"].toString());
    setAuthor(json["author"].toString());
    setPublicationYear(json["publicationYear"].toInt());
    setCategory(stringToCategory(json["category"].toString()));
    setStatus(stringToStatus(json["status"].toString()));
    setDescription(json["description"].toString());
    m_journal = json["journal"].toString();
    m_volume = json["volume"].toInt();
    m_issue = json["issue"].toInt();
    m_pageRange = json["pageRange"].toString();
    m_doi = json["doi"].toString();
    m_abstract = json["abstract"].toString();
    m_researchField = json["researchField"].toString();
    m_keywords.clear();
    QJsonArray keywordsArray = json["keywords"].toArray();
    for (const QJsonValue& value : keywordsArray) {
        m_keywords.append(value.toString());
    }
}
void Article::setJournal(const QString& journal) {
    if (journal.isEmpty()) {
        throw ResourceException("Journal name cannot be empty");
    }
    m_journal = journal;
}
void Article::setVolume(int volume) {
    if (volume < 0) {
        throw ResourceException("Volume number cannot be negative");
    }
    m_volume = volume;
}
void Article::setIssue(int issue) {
    if (issue < 0) {
        throw ResourceException("Issue number cannot be negative");
    }
    m_issue = issue;
}
void Article::setPageRange(const QString& pageRange) {
    m_pageRange = pageRange;
}
void Article::setDoi(const QString& doi) {
    if (!doi.isEmpty() && !isValidDoi(doi)) {
        throw ResourceException("Invalid DOI format");
    }
    m_doi = doi;
}
void Article::setAbstract(const QString& abstract) {
    m_abstract = abstract;
}
void Article::setKeywords(const QStringList& keywords) {
    m_keywords = keywords;
}
void Article::addKeyword(const QString& keyword) {
    if (!keyword.isEmpty() && !m_keywords.contains(keyword)) {
        m_keywords.append(keyword);
    }
}
void Article::removeKeyword(const QString& keyword) {
    m_keywords.removeAll(keyword);
}
void Article::setResearchField(const QString& researchField) {
    m_researchField = researchField;
}
QString Article::getFormattedDetails() const {
    QString formatted = QString("%1 by %2").arg(getTitle()).arg(getAuthor());
    if (!m_journal.isEmpty()) {
        formatted += QString(" - %1").arg(m_journal);
    }
    if (m_volume > 0 || m_issue > 0) {
        formatted += " (";
        if (m_volume > 0) {
            formatted += QString("Vol. %1").arg(m_volume);
            if (m_issue > 0) {
                formatted += QString(", No. %1").arg(m_issue);
            }
        } else if (m_issue > 0) {
            formatted += QString("No. %1").arg(m_issue);
        }
        formatted += ")";
    }
    formatted += QString(" - %1").arg(statusToString(getStatus()));
    return formatted;
}
QString Article::getCitation() const {
    QString citation = QString("%1. \"%2.\" %3")
                      .arg(getAuthor())
                      .arg(getTitle())
                      .arg(m_journal);
    if (m_volume > 0) {
        citation += QString(" %1").arg(m_volume);
        if (m_issue > 0) {
            citation += QString(".%1").arg(m_issue);
        }
    }
    citation += QString(" (%1)").arg(getPublicationYear());
    if (!m_pageRange.isEmpty()) {
        citation += QString(": %1").arg(m_pageRange);
    }
    if (!m_doi.isEmpty()) {
        citation += QString(". DOI: %1").arg(m_doi);
    }
    return citation;
}
bool Article::isValidDoi(const QString& doi) const {
    if (doi.isEmpty()) return true; 
    QRegularExpression doiRegex("^10\\.\\d+\\/\\S+$");
    return doiRegex.match(doi).hasMatch();
}
void Article::validateArticleData() const {
    if (m_journal.isEmpty()) {
        throw ResourceException("Journal name cannot be empty");
    }
    if (m_volume < 0) {
        throw ResourceException("Volume number cannot be negative");
    }
    if (m_issue < 0) {
        throw ResourceException("Issue number cannot be negative");
    }
    if (!m_doi.isEmpty() && !isValidDoi(m_doi)) {
        throw ResourceException("Invalid DOI format");
    }
}