#ifndef LOAN_H
#define LOAN_H
#include <QString>
#include <QDateTime>
#include <QJsonObject>
class Loan {
public:
    enum class Status {
        Active,
        Returned,
        Overdue,
        Renewed,
        Lost
    };
private:
    QString m_loanId;
    QString m_userId;
    QString m_resourceId;
    QString m_resourceTitle; 
    QDateTime m_borrowDate;
    QDateTime m_dueDate;
    QDateTime m_returnDate;
    Status m_status;
    int m_renewalCount;
    int m_maxRenewals;
    double m_fineAmount;
    QString m_notes;
public:
    Loan(const QString& loanId, const QString& userId, const QString& resourceId,
         const QString& resourceTitle, const QDateTime& borrowDate,
         const QDateTime& dueDate, int maxRenewals = 3);
    ~Loan() = default;
    Loan(const Loan& other) = default;
    Loan& operator=(const Loan& other) = default;
    Loan(Loan&& other) noexcept = default;
    Loan& operator=(Loan&& other) noexcept = default;    
    QString getLoanId() const { return m_loanId; }
    QString getId() const { return m_loanId; } 
    QString getUserId() const { return m_userId; }
    QString getResourceId() const { return m_resourceId; }
    QString getResourceTitle() const { return m_resourceTitle; }
    QDateTime getBorrowDate() const { return m_borrowDate; }
    QDateTime getDueDate() const { return m_dueDate; }
    QDateTime getReturnDate() const { return m_returnDate; }
    Status getStatus() const { return m_status; }
    int getRenewalCount() const { return m_renewalCount; }
    int getMaxRenewals() const { return m_maxRenewals; }
    double getFineAmount() const { return m_fineAmount; }
    QString getNotes() const { return m_notes; }
    void setResourceTitle(const QString& title) { m_resourceTitle = title; }
    void setDueDate(const QDateTime& dueDate);
    void setReturnDate(const QDateTime& returnDate);
    void setStatus(Status status);
    void setFineAmount(double amount);
    void setNotes(const QString& notes);
    bool renewLoan(int daysToExtend = 14);
    void returnItem();
    void markAsLost();
    void calculateFine(double dailyFineRate = 0.50);
    bool isOverdue() const;
    bool isActive() const { return m_status == Status::Active; }
    bool isReturned() const { return m_status == Status::Returned; }
    bool canBeRenewed() const;
    int getDaysOverdue() const;
    int getDaysUntilDue() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    QString getFormattedInfo() const;
    QString getStatusString() const;
    QString getDurationString() const;
    static QString statusToString(Status status);
    static Status stringToStatus(const QString& statusStr);
    static QString generateLoanId();
    bool operator==(const Loan& other) const;
    bool operator<(const Loan& other) const; 
private:
    void validateLoanData() const;
    void updateStatus();
};
class LoanException : public std::exception {
private:
    QString m_message;
public:
    explicit LoanException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};
#endif