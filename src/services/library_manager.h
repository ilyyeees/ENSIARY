#ifndef LIBRARY_MANAGER_H
#define LIBRARY_MANAGER_H
#include <vector>
#include <memory>
#include <QString>
#include <QObject>
#include <QDateTime>
#include "../models/resource.h"
#include "../models/user.h"
#include "../models/loan.h"
#include "../models/reservation.h"
class LibraryManager : public QObject {
    Q_OBJECT
private:
    std::vector<std::unique_ptr<Resource>> m_resources;
    std::vector<std::unique_ptr<User>> m_users;
    std::vector<std::unique_ptr<Loan>> m_activeLoans;
    std::vector<std::unique_ptr<Loan>> m_loanHistory;
    std::vector<std::unique_ptr<Reservation>> m_activeReservations;
    std::vector<std::unique_ptr<Reservation>> m_reservationHistory;
    QString m_libraryName;
    QString m_operatingHours;
    std::vector<QString> m_upcomingEvents;
    int m_defaultLoanPeriodDays;
public:
    explicit LibraryManager(QObject* parent = nullptr);
    ~LibraryManager() = default;    
    void addResource(std::unique_ptr<Resource> resource);
    bool removeResource(const QString& resourceId);
    Resource* findResourceById(const QString& resourceId);
    const Resource* findResourceById(const QString& resourceId) const;
    std::vector<Resource*> getAllResources();
    std::vector<const Resource*> getAllResources() const;
    std::vector<Resource*> searchResources(const QString& query);
    std::vector<Resource*> filterResourcesByCategory(Resource::Category category);
    std::vector<Resource*> filterResourcesByStatus(Resource::Status status);
    std::vector<Resource*> getAvailableResources();
    void addUser(std::unique_ptr<User> user);
    bool removeUser(const QString& userId);
    User* findUserById(const QString& userId);    const User* findUserById(const QString& userId) const;
    User* findUserByEmail(const QString& email);
    std::vector<User*> getAllUsers();
    std::vector<const User*> getAllUsers() const;
    std::vector<User*> searchUsers(const QString& query);
    std::vector<User*> getUsersWithOverdueItems();
    QString borrowResource(const QString& userId, const QString& resourceId);
    bool returnResource(const QString& loanId);
    QDateTime renewLoan(const QString& loanId, int additionalDays = 14);
    std::vector<Loan*> getActiveLoans();
    std::vector<const Loan*> getActiveLoans() const;
    std::vector<Loan*> getOverdueLoans();
    std::vector<Loan*> getLoanHistory();
    std::vector<const Loan*> getLoanHistory() const;
    std::vector<Loan*> getCompletedLoans();
    std::vector<Loan*> getUserLoans(const QString& userId);
    std::vector<Loan*> getResourceLoans(const QString& resourceId);    
    QString reserveResource(const QString& userId, const QString& resourceId);
    bool cancelReservation(const QString& reservationId);
    bool cancelUserReservation(const QString& userId, const QString& resourceId);
    std::vector<Reservation*> getActiveReservations();
    std::vector<const Reservation*> getActiveReservations() const;
    std::vector<Reservation*> getUserReservations(const QString& userId);
    std::vector<Reservation*> getResourceReservations(const QString& resourceId);
    std::vector<Reservation*> getExpiredReservations();
    std::vector<Reservation*> getReservationHistory();
    std::vector<const Reservation*> getReservationHistory() const;
    Reservation* findReservationById(const QString& reservationId);
    bool processExpiredReservations(); 
    void notifyWhenResourceAvailable(const QString& resourceId); 
    int getTotalResourceCount() const;
    int getAvailableResourceCount() const;
    int getTotalUserCount() const;
    int getActiveUserCount() const;
    int getTotalActiveLoans() const;
    int getTotalOverdueLoans() const;
    std::vector<Resource*> getMostBorrowedResources(int count = 10);
    std::vector<User*> getMostActiveUsers(int count = 10);
    void addActiveLoan(std::unique_ptr<Loan> loan);
    void addLoanHistory(std::unique_ptr<Loan> loan);
    void addActiveReservation(std::unique_ptr<Reservation> reservation);
    void addReservationHistory(std::unique_ptr<Reservation> reservation);
    void setLibraryName(const QString& name) { m_libraryName = name; }
    QString getLibraryName() const { return m_libraryName; }
    void setOperatingHours(const QString& hours) { m_operatingHours = hours; }
    QString getOperatingHours() const { return m_operatingHours; }    void addUpcomingEvent(const QString& event);
    void removeUpcomingEvent(const QString& event);
    std::vector<QString> getUpcomingEvents() const { return m_upcomingEvents; }
    void setDefaultLoanPeriod(int days) { m_defaultLoanPeriodDays = days; }
    int getDefaultLoanPeriod() const { return m_defaultLoanPeriodDays; }
    bool isValidResourceId(const QString& resourceId) const;
    bool isValidUserId(const QString& userId) const;
    bool isValidLoanId(const QString& loanId) const;
    bool canUserBorrow(const QString& userId) const;
    bool isResourceAvailable(const QString& resourceId) const;
    QString generateResourceId(const QString& prefix = "RES");
    QString generateUserId(const QString& prefix = "USER");
    QString generateLoanId();
    void performDailyMaintenance();
    void updateResourceAvailability();
signals:
    void resourceAdded(const QString& resourceId);
    void resourceRemoved(const QString& resourceId);
    void userAdded(const QString& userId);
    void userRemoved(const QString& userId);
    void resourceBorrowed(const QString& loanId, const QString& userId, const QString& resourceId);
    void resourceReturned(const QString& loanId, const QString& userId, const QString& resourceId);
    void loanRenewed(const QString& loanId, const QDateTime& newDueDate);
    void itemOverdue(const QString& loanId, const QString& userId, const QString& resourceId);
    void resourceReserved(const QString& reservationId, const QString& userId, const QString& resourceId);
    void reservationCancelled(const QString& reservationId, const QString& userId, const QString& resourceId);
    void reservationExpired(const QString& reservationId, const QString& userId, const QString& resourceId);
    void reservedResourceAvailable(const QString& reservationId, const QString& userId, const QString& resourceId);
private:
    std::vector<std::unique_ptr<Resource>>::iterator findResourceIterator(const QString& resourceId);
    std::vector<std::unique_ptr<User>>::iterator findUserIterator(const QString& userId);
    std::vector<std::unique_ptr<Loan>>::iterator findLoanIterator(const QString& loanId);
    bool matchesSearchQuery(const Resource& resource, const QString& query) const;
    bool matchesSearchQuery(const User& user, const QString& query) const;
    QDateTime calculateDueDate(int loanPeriodDays = 0) const;
    void processLoanReturn(Loan& loan);
    void moveLoanToHistory(const QString& loanId);
    void validateResourceData(const Resource& resource) const;
    void validateUserData(const User& user) const;
    void validateLoanData(const Loan& loan) const;
};
class LibraryManagerException : public std::exception {
private:
    QString m_message;
public:
    explicit LibraryManagerException(const QString& message) : m_message(message) {}
    const char* what() const noexcept override {
        return m_message.toStdString().c_str();
    }
    QString getMessage() const { return m_message; }
};
#endif