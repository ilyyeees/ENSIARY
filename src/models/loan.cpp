#include "loan.h"
#include <QUuid>
#include <QJsonObject>
Loan::Loan(const QString& loanId, const QString& userId, const QString& resourceId,
           const QString& resourceTitle, const QDateTime& borrowDate,
           const QDateTime& dueDate, int maxRenewals)
    : m_loanId(loanId), m_userId(userId), m_resourceId(resourceId),
      m_resourceTitle(resourceTitle), m_borrowDate(borrowDate), m_dueDate(dueDate),
      m_status(Status::Active), m_renewalCount(0), m_maxRenewals(maxRenewals),
      m_fineAmount(0.0) {
    validateLoanData();
    updateStatus();
}
void Loan::setDueDate(const QDateTime& dueDate) {
    if (dueDate <= m_borrowDate) {
        throw LoanException("Due date must be after borrow date");
    }
    m_dueDate = dueDate;
    updateStatus();
}
void Loan::setReturnDate(const QDateTime& returnDate) {
    if (returnDate < m_borrowDate) {
        throw LoanException("Return date cannot be before borrow date");
    }
    m_returnDate = returnDate;
    updateStatus();
}
void Loan::setStatus(Status status) {
    m_status = status;
}
void Loan::setFineAmount(double amount) {
    if (amount < 0.0) {
        throw LoanException("Fine amount cannot be negative");
    }
    m_fineAmount = amount;
}
void Loan::setNotes(const QString& notes) {
    m_notes = notes;
}
bool Loan::renewLoan(int daysToExtend) {
    if (!canBeRenewed()) {
        return false;
    }
    m_dueDate = m_dueDate.addDays(daysToExtend);
    m_renewalCount++;
    m_status = Status::Renewed;
    return true;
}
void Loan::returnItem() {
    m_returnDate = QDateTime::currentDateTime();
    m_status = Status::Returned;
    if (isOverdue()) {
        calculateFine();
    }
}
void Loan::markAsLost() {
    m_status = Status::Lost;
    m_fineAmount += 50.0; 
}
void Loan::calculateFine(double dailyFineRate) {
    int daysOverdue = getDaysOverdue();
    if (daysOverdue > 0) {
        m_fineAmount = daysOverdue * dailyFineRate;
    }
}
bool Loan::isOverdue() const {
    if (m_status == Status::Returned || m_status == Status::Lost) {
        return false; 
    }
    return QDateTime::currentDateTime() > m_dueDate;
}
bool Loan::canBeRenewed() const {
    return (m_status == Status::Active || m_status == Status::Renewed) &&
           m_renewalCount < m_maxRenewals &&
           !isOverdue();
}
int Loan::getDaysOverdue() const {
    if (!isOverdue()) {
        return 0;
    }
    return m_dueDate.daysTo(QDateTime::currentDateTime());
}
int Loan::getDaysUntilDue() const {
    if (m_status == Status::Returned || m_status == Status::Lost) {
        return 0;
    }
    int daysUntil = QDateTime::currentDateTime().daysTo(m_dueDate);
    return qMax(0, daysUntil);
}
QJsonObject Loan::toJson() const {
    QJsonObject json;
    json["loanId"] = m_loanId;
    json["userId"] = m_userId;
    json["resourceId"] = m_resourceId;
    json["resourceTitle"] = m_resourceTitle;
    json["borrowDate"] = m_borrowDate.toString(Qt::ISODate);
    json["dueDate"] = m_dueDate.toString(Qt::ISODate);
    json["returnDate"] = m_returnDate.toString(Qt::ISODate);
    json["status"] = statusToString(m_status);
    json["renewalCount"] = m_renewalCount;
    json["maxRenewals"] = m_maxRenewals;
    json["fineAmount"] = m_fineAmount;
    json["notes"] = m_notes;
    return json;
}
void Loan::fromJson(const QJsonObject& json) {
    m_loanId = json["loanId"].toString();
    m_userId = json["userId"].toString();
    m_resourceId = json["resourceId"].toString();
    m_resourceTitle = json["resourceTitle"].toString();
    m_borrowDate = QDateTime::fromString(json["borrowDate"].toString(), Qt::ISODate);
    m_dueDate = QDateTime::fromString(json["dueDate"].toString(), Qt::ISODate);
    m_returnDate = QDateTime::fromString(json["returnDate"].toString(), Qt::ISODate);
    m_status = stringToStatus(json["status"].toString());
    m_renewalCount = json["renewalCount"].toInt();
    m_maxRenewals = json["maxRenewals"].toInt();
    m_fineAmount = json["fineAmount"].toDouble();
    m_notes = json["notes"].toString();
}
QString Loan::getFormattedInfo() const {
    QString info = QString("%1 - %2")
                   .arg(m_resourceTitle)
                   .arg(getStatusString());
    if (m_status == Status::Active || m_status == Status::Renewed) {
        info += QString(" (Due: %1)").arg(m_dueDate.toString("yyyy-MM-dd"));
        if (isOverdue()) {
            info += QString(" - OVERDUE by %1 days").arg(getDaysOverdue());
        }
    } else if (m_status == Status::Returned) {
        info += QString(" (Returned: %1)").arg(m_returnDate.toString("yyyy-MM-dd"));
    }
    if (m_fineAmount > 0.0) {
        info += QString(" - Fine: $%.2f").arg(m_fineAmount);
    }
    return info;
}
QString Loan::getStatusString() const {
    return statusToString(m_status);
}
QString Loan::getDurationString() const {
    if (m_status == Status::Returned) {
        int days = m_borrowDate.daysTo(m_returnDate);
        return QString("%1 days").arg(days);
    } else {
        int days = m_borrowDate.daysTo(QDateTime::currentDateTime());
        return QString("%1 days (ongoing)").arg(days);
    }
}
QString Loan::statusToString(Status status) {
    switch (status) {
        case Status::Active: return "Active";
        case Status::Returned: return "Returned";
        case Status::Overdue: return "Overdue";
        case Status::Renewed: return "Renewed";
        case Status::Lost: return "Lost";
        default: return "Unknown";
    }
}
Loan::Status Loan::stringToStatus(const QString& statusStr) {
    if (statusStr == "Active") return Status::Active;
    if (statusStr == "Returned") return Status::Returned;
    if (statusStr == "Overdue") return Status::Overdue;
    if (statusStr == "Renewed") return Status::Renewed;
    if (statusStr == "Lost") return Status::Lost;
    return Status::Active; 
}
QString Loan::generateLoanId() {
    return "LOAN_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}
bool Loan::operator==(const Loan& other) const {
    return m_loanId == other.m_loanId;
}
bool Loan::operator<(const Loan& other) const {
    return m_dueDate < other.m_dueDate;
}
void Loan::validateLoanData() const {
    if (m_loanId.isEmpty()) {
        throw LoanException("Loan ID cannot be empty");
    }
    if (m_userId.isEmpty()) {
        throw LoanException("User ID cannot be empty");
    }
    if (m_resourceId.isEmpty()) {
        throw LoanException("Resource ID cannot be empty");
    }
    if (m_dueDate <= m_borrowDate) {
        throw LoanException("Due date must be after borrow date");
    }
    if (m_maxRenewals < 0) {
        throw LoanException("Maximum renewals cannot be negative");
    }
}
void Loan::updateStatus() {
    if (m_status == Status::Returned || m_status == Status::Lost) {
        return; 
    }
    if (isOverdue()) {
        m_status = Status::Overdue;
    }
}