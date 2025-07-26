#ifndef RESERVATION_H
#define RESERVATION_H
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QUuid>
class Reservation {
public:
    enum class Status {
        Active,     
        Fulfilled,  
        Expired,    
        Cancelled   
    };
private:
    QString m_reservationId;
    QString m_userId;
    QString m_resourceId;
    QString m_resourceTitle;
    QDateTime m_reservationDate;
    QDateTime m_expirationDate;
    Status m_status;
    QString m_notes;
public:
    Reservation(const QString& userId, const QString& resourceId, 
                const QString& resourceTitle, int expirationDays = 7);
    QString getReservationId() const { return m_reservationId; }
    QString getId() const { return m_reservationId; } 
    QString getUserId() const { return m_userId; }
    QString getResourceId() const { return m_resourceId; }
    QString getResourceTitle() const { return m_resourceTitle; }
    QDateTime getReservationDate() const { return m_reservationDate; }
    QDateTime getExpirationDate() const { return m_expirationDate; }
    Status getStatus() const { return m_status; }
    QString getNotes() const { return m_notes; }
    void setStatus(Status status);
    void setNotes(const QString& notes);
    void setExpirationDate(const QDateTime& expirationDate);
    bool isActive() const { return m_status == Status::Active; }
    bool isExpired() const;
    bool canBeFulfilled() const;
    void fulfillReservation();
    void cancelReservation();
    void extendReservation(int additionalDays);
    int getDaysUntilExpiration() const;
    QString getFormattedInfo() const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    static QString generateReservationId();
    static QString statusToString(Status status);
    static Status stringToStatus(const QString& statusStr);
    bool operator==(const Reservation& other) const;
    bool operator<(const Reservation& other) const; 
};
class ReservationException : public std::exception {
private:
    QString m_message;
public:
    explicit ReservationException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};
#endif