#include "reservation.h"
#include <QUuid>
Reservation::Reservation(const QString& userId, const QString& resourceId, 
                        const QString& resourceTitle, int expirationDays)
    : m_userId(userId), m_resourceId(resourceId), m_resourceTitle(resourceTitle),
      m_status(Status::Active), m_reservationDate(QDateTime::currentDateTime()) {
    m_reservationId = generateReservationId();
    m_expirationDate = m_reservationDate.addDays(expirationDays);
}
void Reservation::setStatus(Status status) {
    m_status = status;
}
void Reservation::setNotes(const QString& notes) {
    m_notes = notes;
}
void Reservation::setExpirationDate(const QDateTime& expirationDate) {
    m_expirationDate = expirationDate;
}
bool Reservation::isExpired() const {
    return QDateTime::currentDateTime() > m_expirationDate && m_status == Status::Active;
}
bool Reservation::canBeFulfilled() const {
    return m_status == Status::Active && !isExpired();
}
void Reservation::fulfillReservation() {
    if (!canBeFulfilled()) {
        throw ReservationException("Cannot fulfill reservation: either expired or not active");
    }
    m_status = Status::Fulfilled;
}
void Reservation::cancelReservation() {
    if (m_status == Status::Fulfilled) {
        throw ReservationException("Cannot cancel fulfilled reservation");
    }
    m_status = Status::Cancelled;
}
void Reservation::extendReservation(int additionalDays) {
    if (m_status != Status::Active) {
        throw ReservationException("Can only extend active reservations");
    }
    m_expirationDate = m_expirationDate.addDays(additionalDays);
}
int Reservation::getDaysUntilExpiration() const {
    QDateTime now = QDateTime::currentDateTime();
    return now.daysTo(m_expirationDate);
}
QString Reservation::getFormattedInfo() const {
    QString info;
    info += QString("Reservation ID: %1\n").arg(m_reservationId);
    info += QString("Resource: %1\n").arg(m_resourceTitle);
    info += QString("Reserved Date: %1\n").arg(m_reservationDate.toString("yyyy-MM-dd hh:mm"));
    info += QString("Expires: %1\n").arg(m_expirationDate.toString("yyyy-MM-dd hh:mm"));
    info += QString("Status: %1\n").arg(statusToString(m_status));
    if (m_status == Status::Active) {
        int daysLeft = getDaysUntilExpiration();
        if (daysLeft >= 0) {
            info += QString("Days until expiration: %1\n").arg(daysLeft);
        } else {
            info += "⚠️ EXPIRED\n";
        }
    }
    if (!m_notes.isEmpty()) {
        info += QString("Notes: %1\n").arg(m_notes);
    }
    return info;
}
QJsonObject Reservation::toJson() const {
    QJsonObject json;
    json["reservationId"] = m_reservationId;
    json["userId"] = m_userId;
    json["resourceId"] = m_resourceId;
    json["resourceTitle"] = m_resourceTitle;
    json["reservationDate"] = m_reservationDate.toString(Qt::ISODate);
    json["expirationDate"] = m_expirationDate.toString(Qt::ISODate);
    json["status"] = statusToString(m_status);
    json["notes"] = m_notes;
    return json;
}
void Reservation::fromJson(const QJsonObject& json) {
    m_reservationId = json["reservationId"].toString();
    m_userId = json["userId"].toString();
    m_resourceId = json["resourceId"].toString();
    m_resourceTitle = json["resourceTitle"].toString();
    m_reservationDate = QDateTime::fromString(json["reservationDate"].toString(), Qt::ISODate);
    m_expirationDate = QDateTime::fromString(json["expirationDate"].toString(), Qt::ISODate);
    m_status = stringToStatus(json["status"].toString());
    m_notes = json["notes"].toString();
}
QString Reservation::generateReservationId() {
    return "RES_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}
QString Reservation::statusToString(Status status) {
    switch (status) {
        case Status::Active: return "Active";
        case Status::Fulfilled: return "Fulfilled";
        case Status::Expired: return "Expired";
        case Status::Cancelled: return "Cancelled";
        default: return "Unknown";
    }
}
Reservation::Status Reservation::stringToStatus(const QString& statusStr) {
    if (statusStr == "Active") return Status::Active;
    if (statusStr == "Fulfilled") return Status::Fulfilled;
    if (statusStr == "Expired") return Status::Expired;
    if (statusStr == "Cancelled") return Status::Cancelled;
    return Status::Active; 
}
bool Reservation::operator==(const Reservation& other) const {
    return m_reservationId == other.m_reservationId;
}
bool Reservation::operator<(const Reservation& other) const {
    return m_reservationDate < other.m_reservationDate;
}