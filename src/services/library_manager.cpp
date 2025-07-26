#include "library_manager.h"
#include "../models/resource.h"
#include "../models/book.h"
#include "../models/article.h"
#include "../models/user.h"
#include "../models/loan.h"
#include "../models/reservation.h"
#include <algorithm>
#include <QUuid>
#include <QDebug>
LibraryManager::LibraryManager(QObject* parent)
    : QObject(parent), m_libraryName("ENSIARY Library Management System"),
      m_operatingHours("Monday-Friday: 8:00 AM - 8:00 PM, Saturday-Sunday: 10:00 AM - 6:00 PM"),
      m_defaultLoanPeriodDays(14) {
}
void LibraryManager::addResource(std::unique_ptr<Resource> resource) {
    if (!resource) {
        throw LibraryManagerException("Cannot add null resource");
    }
    validateResourceData(*resource);
    if (findResourceById(resource->getId()) != nullptr) {
        throw LibraryManagerException("Resource with ID " + resource->getId() + " already exists");
    }
    QString resourceId = resource->getId();
    m_resources.push_back(std::move(resource));
    emit resourceAdded(resourceId);
}
bool LibraryManager::removeResource(const QString& resourceId) {
    auto it = findResourceIterator(resourceId);
    if (it == m_resources.end()) {
        return false;
    }
    Resource* resource = it->get();
    if (resource->isBorrowed()) {
        throw LibraryManagerException("Cannot remove resource that is currently borrowed");
    }
    m_resources.erase(it);
    emit resourceRemoved(resourceId);
    return true;
}
Resource* LibraryManager::findResourceById(const QString& resourceId) {
    auto it = std::find_if(m_resources.begin(), m_resources.end(),
                          [&resourceId](const std::unique_ptr<Resource>& resource) {
                              return resource->getId() == resourceId;
                          });
    return (it != m_resources.end()) ? it->get() : nullptr;
}
const Resource* LibraryManager::findResourceById(const QString& resourceId) const {
    auto it = std::find_if(m_resources.begin(), m_resources.end(),
                          [&resourceId](const std::unique_ptr<Resource>& resource) {
                              return resource->getId() == resourceId;
                          });
    return (it != m_resources.end()) ? it->get() : nullptr;
}
std::vector<Resource*> LibraryManager::getAllResources() {
    std::vector<Resource*> resources;
    for (const auto& resource : m_resources) {
        resources.push_back(resource.get());
    }
    return resources;
}
std::vector<const Resource*> LibraryManager::getAllResources() const {
    std::vector<const Resource*> resources;
    for (const auto& resource : m_resources) {
        resources.push_back(resource.get());
    }
    return resources;
}
std::vector<Resource*> LibraryManager::searchResources(const QString& query) {
    std::vector<Resource*> results;
    for (const auto& resource : m_resources) {
        if (matchesSearchQuery(*resource, query)) {
            results.push_back(resource.get());
        }
    }
    return results;
}
std::vector<Resource*> LibraryManager::filterResourcesByCategory(Resource::Category category) {
    std::vector<Resource*> results;
    for (const auto& resource : m_resources) {
        if (resource->getCategory() == category) {
            results.push_back(resource.get());
        }
    }
    return results;
}
std::vector<Resource*> LibraryManager::filterResourcesByStatus(Resource::Status status) {
    std::vector<Resource*> results;
    for (const auto& resource : m_resources) {
        if (resource->getStatus() == status) {
            results.push_back(resource.get());
        }
    }
    return results;
}
std::vector<Resource*> LibraryManager::getAvailableResources() {
    return filterResourcesByStatus(Resource::Status::Available);
}
void LibraryManager::addUser(std::unique_ptr<User> user) {
    if (!user) {
        throw LibraryManagerException("Cannot add null user");
    }
    validateUserData(*user);
    if (findUserById(user->getUserId()) != nullptr) {
        throw LibraryManagerException("User with ID " + user->getUserId() + " already exists");
    }
    if (findUserByEmail(user->getEmail()) != nullptr) {
        throw LibraryManagerException("User with email " + user->getEmail() + " already exists");
    }
    QString userId = user->getUserId();
    m_users.push_back(std::move(user));
    emit userAdded(userId);
}
bool LibraryManager::removeUser(const QString& userId) {
    auto it = findUserIterator(userId);
    if (it == m_users.end()) {
        return false;
    }
    User* user = it->get();
    if (user->getCurrentLoanCount() > 0) {
        throw LibraryManagerException("Cannot remove user with active loans");
    }
    m_users.erase(it);
    emit userRemoved(userId);
    return true;
}
User* LibraryManager::findUserById(const QString& userId) {
    auto it = std::find_if(m_users.begin(), m_users.end(),
                          [&userId](const std::unique_ptr<User>& user) {
                              return user->getUserId() == userId;
                          });
    return (it != m_users.end()) ? it->get() : nullptr;
}
const User* LibraryManager::findUserById(const QString& userId) const {
    auto it = std::find_if(m_users.begin(), m_users.end(),
                          [&userId](const std::unique_ptr<User>& user) {
                              return user->getUserId() == userId;
                          });
    return (it != m_users.end()) ? it->get() : nullptr;
}
User* LibraryManager::findUserByEmail(const QString& email) {
    auto it = std::find_if(m_users.begin(), m_users.end(),
                          [&email](const std::unique_ptr<User>& user) {
                              return user->getEmail() == email;
                          });
    return (it != m_users.end()) ? it->get() : nullptr;
}
std::vector<User*> LibraryManager::getAllUsers() {
    std::vector<User*> users;
    for (const auto& user : m_users) {
        users.push_back(user.get());
    }
    return users;
}
std::vector<const User*> LibraryManager::getAllUsers() const {
    std::vector<const User*> users;
    for (const auto& user : m_users) {
        users.push_back(user.get());
    }
    return users;
}
std::vector<User*> LibraryManager::searchUsers(const QString& query) {
    std::vector<User*> results;
    for (const auto& user : m_users) {
        if (matchesSearchQuery(*user, query)) {
            results.push_back(user.get());
        }
    }
    return results;
}
std::vector<User*> LibraryManager::getUsersWithOverdueItems() {
    std::vector<User*> results;
    for (const auto& user : m_users) {
        if (user->hasOverdueItems()) {        results.push_back(user.get());
        }
    }
    return results;
}
QString LibraryManager::borrowResource(const QString& userId, const QString& resourceId) {
    User* user = findUserById(userId);
    if (!user) {
        throw LibraryManagerException("User not found: " + userId);
    }
    Resource* resource = findResourceById(resourceId);
    if (!resource) {
        throw LibraryManagerException("Resource not found: " + resourceId);
    }
    if (!user->canBorrow()) {
        throw LibraryManagerException("User cannot borrow items at this time");
    }
    if (!resource->isAvailable()) {
        throw LibraryManagerException("Resource is not available for borrowing");
    }
    QString loanId = generateLoanId();
    QDateTime borrowDate = QDateTime::currentDateTime();
    QDateTime dueDate = calculateDueDate();
    auto loan = std::make_unique<Loan>(loanId, userId, resourceId, resource->getTitle(),
                                      borrowDate, dueDate);
    resource->setStatus(Resource::Status::Borrowed);
    user->addCurrentLoan(std::unique_ptr<Loan>(new Loan(*loan)));
    m_activeLoans.push_back(std::move(loan));
    emit resourceBorrowed(loanId, userId, resourceId);
    return loanId;
}
bool LibraryManager::returnResource(const QString& loanId) {
    auto it = findLoanIterator(loanId);
    if (it == m_activeLoans.end()) {
        return false;
    }
    Loan* loan = it->get();
    QString resourceId = loan->getResourceId();
    Resource* resource = findResourceById(resourceId);
    if (resource) {
        resource->setStatus(Resource::Status::Available);
    }
    processLoanReturn(*loan);
    moveLoanToHistory(loanId);
    emit resourceReturned(loanId, loan->getUserId(), resourceId);
    notifyWhenResourceAvailable(resourceId);
    return true;
}
QDateTime LibraryManager::renewLoan(const QString& loanId, int additionalDays) {
    auto it = findLoanIterator(loanId);
    if (it == m_activeLoans.end()) {
        return QDateTime();
    }
    Loan* loan = it->get();
    if (!loan->canBeRenewed()) {
        throw LibraryManagerException("Loan cannot be renewed");
    }
    if (loan->renewLoan(additionalDays)) {
        emit loanRenewed(loanId, loan->getDueDate());
        return loan->getDueDate();
    }
    return QDateTime();
}
std::vector<Loan*> LibraryManager::getActiveLoans() {
    std::vector<Loan*> loans;
    for (const auto& loan : m_activeLoans) {
        loans.push_back(loan.get());
    }
    return loans;
}
std::vector<const Loan*> LibraryManager::getActiveLoans() const {
    std::vector<const Loan*> loans;
    for (const auto& loan : m_activeLoans) {
        loans.push_back(loan.get());
    }
    return loans;
}
std::vector<Loan*> LibraryManager::getOverdueLoans() {
    std::vector<Loan*> overdueLoans;
    for (const auto& loan : m_activeLoans) {
        if (loan->isOverdue()) {
            overdueLoans.push_back(loan.get());
        }
    }
    return overdueLoans;
}
std::vector<Loan*> LibraryManager::getLoanHistory() {
    std::vector<Loan*> loans;
    for (const auto& loan : m_loanHistory) {
        loans.push_back(loan.get());
    }
    return loans;
}
std::vector<const Loan*> LibraryManager::getLoanHistory() const {
    std::vector<const Loan*> loans;
    for (const auto& loan : m_loanHistory) {
        loans.push_back(loan.get());
    }
    return loans;
}
std::vector<Reservation*> LibraryManager::getReservationHistory() {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_reservationHistory) {
        reservations.push_back(reservation.get());
    }
    return reservations;
}
std::vector<const Reservation*> LibraryManager::getReservationHistory() const {
    std::vector<const Reservation*> reservations;
    for (const auto& reservation : m_reservationHistory) {
        reservations.push_back(reservation.get());
    }
    return reservations;
}
std::vector<Loan*> LibraryManager::getUserLoans(const QString& userId) {
    std::vector<Loan*> userLoans;
    for (const auto& loan : m_activeLoans) {
        if (loan->getUserId() == userId) {
            userLoans.push_back(loan.get());
        }
    }
    return userLoans;
}
std::vector<Loan*> LibraryManager::getResourceLoans(const QString& resourceId) {
    std::vector<Loan*> resourceLoans;
    for (const auto& loan : m_activeLoans) {
        if (loan->getResourceId() == resourceId) {
            resourceLoans.push_back(loan.get());
        }
    }
    for (const auto& loan : m_loanHistory) {
        if (loan->getResourceId() == resourceId) {
            resourceLoans.push_back(loan.get());
        }
    }
    return resourceLoans;
}
QString LibraryManager::reserveResource(const QString& userId, const QString& resourceId) {
    User* user = findUserById(userId);
    if (!user) {
        throw LibraryManagerException("User not found: " + userId);
    }
    Resource* resource = findResourceById(resourceId);
    if (!resource) {
        throw LibraryManagerException("Resource not found: " + resourceId);
    }
    if (!user->canBorrow()) {
        throw LibraryManagerException("User cannot make reservations");
    }
    for (const auto& reservation : m_activeReservations) {
        if (reservation->getUserId() == userId && reservation->getResourceId() == resourceId && 
            reservation->isActive()) {
            throw LibraryManagerException("User already has an active reservation for this resource");
        }
    }
    if (resource->isAvailable()) {
        throw LibraryManagerException("Resource is available for immediate borrowing - no reservation needed");
    }
    auto reservation = std::make_unique<Reservation>(userId, resourceId, resource->getTitle());
    QString reservationId = reservation->getReservationId();
    m_activeReservations.push_back(std::move(reservation));
    emit resourceReserved(reservationId, userId, resourceId);
    return reservationId;
}
bool LibraryManager::cancelReservation(const QString& reservationId) {
    auto it = std::find_if(m_activeReservations.begin(), m_activeReservations.end(),
                          [&reservationId](const std::unique_ptr<Reservation>& reservation) {
                              return reservation->getReservationId() == reservationId;
                          });
    if (it != m_activeReservations.end()) {
        QString userId = (*it)->getUserId();
        QString resourceId = (*it)->getResourceId();
        (*it)->cancelReservation();
        m_reservationHistory.push_back(std::move(*it));
        m_activeReservations.erase(it);
        emit reservationCancelled(reservationId, userId, resourceId);
        return true;
    }
    return false;
}
bool LibraryManager::cancelUserReservation(const QString& userId, const QString& resourceId) {
    auto it = std::find_if(m_activeReservations.begin(), m_activeReservations.end(),
                          [&userId, &resourceId](const std::unique_ptr<Reservation>& reservation) {
                              return reservation->getUserId() == userId && 
                                     reservation->getResourceId() == resourceId &&
                                     reservation->isActive();
                          });
    if (it != m_activeReservations.end()) {
        QString reservationId = (*it)->getReservationId();
        return cancelReservation(reservationId);
    }
    return false;
}
std::vector<Reservation*> LibraryManager::getActiveReservations() {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->isActive()) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}
std::vector<const Reservation*> LibraryManager::getActiveReservations() const {
    std::vector<const Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->isActive()) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}
std::vector<Reservation*> LibraryManager::getUserReservations(const QString& userId) {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->getUserId() == userId) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}
std::vector<Reservation*> LibraryManager::getResourceReservations(const QString& resourceId) {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->getResourceId() == resourceId && reservation->isActive()) {
            reservations.push_back(reservation.get());
        }
    }
    std::sort(reservations.begin(), reservations.end(),
              [](const Reservation* a, const Reservation* b) {
                  return a->getReservationDate() < b->getReservationDate();
              });
    return reservations;
}
std::vector<Reservation*> LibraryManager::getExpiredReservations() {
    std::vector<Reservation*> reservations;
    for (const auto& reservation : m_activeReservations) {
        if (reservation->isExpired()) {
            reservations.push_back(reservation.get());
        }
    }
    return reservations;
}
Reservation* LibraryManager::findReservationById(const QString& reservationId) {
    auto it = std::find_if(m_activeReservations.begin(), m_activeReservations.end(),
                          [&reservationId](const std::unique_ptr<Reservation>& reservation) {
                              return reservation->getReservationId() == reservationId;
                          });
    return (it != m_activeReservations.end()) ? it->get() : nullptr;
}
bool LibraryManager::processExpiredReservations() {
    bool hasExpired = false;
    auto it = m_activeReservations.begin();
    while (it != m_activeReservations.end()) {
        if ((*it)->isExpired()) {
            QString reservationId = (*it)->getReservationId();
            QString userId = (*it)->getUserId();
            QString resourceId = (*it)->getResourceId();
            (*it)->setStatus(Reservation::Status::Expired);
            m_reservationHistory.push_back(std::move(*it));
            it = m_activeReservations.erase(it);
            emit reservationExpired(reservationId, userId, resourceId);
            hasExpired = true;
        } else {
            ++it;
        }
    }
    return hasExpired;
}
void LibraryManager::notifyWhenResourceAvailable(const QString& resourceId) {
    auto reservations = getResourceReservations(resourceId);
    if (!reservations.empty()) {
        Reservation* firstReservation = reservations[0];
        if (firstReservation->canBeFulfilled()) {
            emit reservedResourceAvailable(firstReservation->getReservationId(),
                                         firstReservation->getUserId(),
                                         resourceId);
        }
    }
}
std::vector<Loan*> LibraryManager::getCompletedLoans() {
    std::vector<Loan*> loans;
    for (const auto& loan : m_loanHistory) {
        loans.push_back(loan.get());
    }
    return loans;
}
int LibraryManager::getTotalResourceCount() const {
    return static_cast<int>(m_resources.size());
}
int LibraryManager::getAvailableResourceCount() const {
    return static_cast<int>(std::count_if(m_resources.begin(), m_resources.end(),
                                         [](const std::unique_ptr<Resource>& resource) {
                                             return resource->isAvailable();
                                         }));
}
int LibraryManager::getTotalUserCount() const {
    return static_cast<int>(m_users.size());
}
int LibraryManager::getActiveUserCount() const {
    return static_cast<int>(std::count_if(m_users.begin(), m_users.end(),
                                         [](const std::unique_ptr<User>& user) {
                                             return user->getStatus() == User::Status::Active;
                                         }));
}
int LibraryManager::getTotalActiveLoans() const {
    return static_cast<int>(m_activeLoans.size());
}
int LibraryManager::getTotalOverdueLoans() const {
    return static_cast<int>(std::count_if(m_activeLoans.begin(), m_activeLoans.end(),
                                         [](const std::unique_ptr<Loan>& loan) {
                                             return loan->isOverdue();
                                         }));
}
std::vector<Resource*> LibraryManager::getMostBorrowedResources(int count) {
    std::vector<Resource*> resources = getAllResources();
    if (resources.size() > static_cast<size_t>(count)) {
        resources.resize(count);
    }
    return resources;
}
std::vector<User*> LibraryManager::getMostActiveUsers(int count) {
    std::vector<User*> users = getAllUsers();
    if (users.size() > static_cast<size_t>(count)) {
        users.resize(count);
    }
    return users;
}
void LibraryManager::addUpcomingEvent(const QString& event) {
    if (!event.isEmpty() && std::find(m_upcomingEvents.begin(), m_upcomingEvents.end(), event) == m_upcomingEvents.end()) {
        m_upcomingEvents.push_back(event);
    }
}
void LibraryManager::removeUpcomingEvent(const QString& event) {
    m_upcomingEvents.erase(std::remove(m_upcomingEvents.begin(), m_upcomingEvents.end(), event),
                          m_upcomingEvents.end());
}
bool LibraryManager::isValidResourceId(const QString& resourceId) const {
    return findResourceById(resourceId) != nullptr;
}
bool LibraryManager::isValidUserId(const QString& userId) const {
    return findUserById(userId) != nullptr;
}
bool LibraryManager::isValidLoanId(const QString& loanId) const {
    return std::find_if(m_activeLoans.begin(), m_activeLoans.end(),
                       [&loanId](const std::unique_ptr<Loan>& loan) {
                           return loan->getLoanId() == loanId;
                       }) != m_activeLoans.end();
}
bool LibraryManager::canUserBorrow(const QString& userId) const {
    const User* user = findUserById(userId);
    return user ? user->canBorrow() : false;
}
bool LibraryManager::isResourceAvailable(const QString& resourceId) const {
    const Resource* resource = findResourceById(resourceId);
    return resource ? resource->isAvailable() : false;
}
QString LibraryManager::generateResourceId(const QString& prefix) {
    return prefix + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}
QString LibraryManager::generateUserId(const QString& prefix) {
    return prefix + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}
QString LibraryManager::generateLoanId() {
    return Loan::generateLoanId();
}
void LibraryManager::performDailyMaintenance() {
    updateResourceAvailability();
    processExpiredReservations();
    for (const auto& loan : m_activeLoans) {
        if (loan->isOverdue()) {
            emit itemOverdue(loan->getLoanId(), loan->getUserId(), loan->getResourceId());
        }
    }
}
void LibraryManager::updateResourceAvailability() {
}
std::vector<std::unique_ptr<Resource>>::iterator LibraryManager::findResourceIterator(const QString& resourceId) {
    return std::find_if(m_resources.begin(), m_resources.end(),
                       [&resourceId](const std::unique_ptr<Resource>& resource) {
                           return resource->getId() == resourceId;
                       });
}
std::vector<std::unique_ptr<User>>::iterator LibraryManager::findUserIterator(const QString& userId) {
    return std::find_if(m_users.begin(), m_users.end(),
                       [&userId](const std::unique_ptr<User>& user) {
                           return user->getUserId() == userId;
                       });
}
std::vector<std::unique_ptr<Loan>>::iterator LibraryManager::findLoanIterator(const QString& loanId) {
    return std::find_if(m_activeLoans.begin(), m_activeLoans.end(),
                       [&loanId](const std::unique_ptr<Loan>& loan) {
                           return loan->getLoanId() == loanId;
                       });
}
bool LibraryManager::matchesSearchQuery(const Resource& resource, const QString& query) const {
    QString lowerQuery = query.toLower();
    return resource.getTitle().toLower().contains(lowerQuery) ||
           resource.getAuthor().toLower().contains(lowerQuery) ||
           resource.getDescription().toLower().contains(lowerQuery);
}
bool LibraryManager::matchesSearchQuery(const User& user, const QString& query) const {
    QString lowerQuery = query.toLower();
    return user.getFirstName().toLower().contains(lowerQuery) ||
           user.getLastName().toLower().contains(lowerQuery) ||
           user.getEmail().toLower().contains(lowerQuery) ||
           user.getUserId().toLower().contains(lowerQuery);
}
QDateTime LibraryManager::calculateDueDate(int loanPeriodDays) const {
    int days = (loanPeriodDays > 0) ? loanPeriodDays : m_defaultLoanPeriodDays;
    return QDateTime::currentDateTime().addDays(days);
}
void LibraryManager::processLoanReturn(Loan& loan) {
    loan.returnItem();
    User* user = findUserById(loan.getUserId());
    if (user) {
        user->moveLoanToHistory(loan.getLoanId());
    }
}
void LibraryManager::moveLoanToHistory(const QString& loanId) {
    auto it = findLoanIterator(loanId);
    if (it != m_activeLoans.end()) {
        m_loanHistory.push_back(std::move(*it));
        m_activeLoans.erase(it);
    }
}
void LibraryManager::validateResourceData(const Resource& resource) const {
    if (resource.getId().isEmpty()) {
        throw LibraryManagerException("Resource ID cannot be empty");
    }
    if (resource.getTitle().isEmpty()) {
        throw LibraryManagerException("Resource title cannot be empty");
    }
    if (resource.getAuthor().isEmpty()) {
        throw LibraryManagerException("Resource author cannot be empty");
    }
}
void LibraryManager::validateUserData(const User& user) const {
    if (user.getUserId().isEmpty()) {
        throw LibraryManagerException("User ID cannot be empty");
    }
    if (user.getFirstName().isEmpty()) {
        throw LibraryManagerException("User first name cannot be empty");
    }
    if (user.getLastName().isEmpty()) {
        throw LibraryManagerException("User last name cannot be empty");
    }
    if (user.getEmail().isEmpty()) {
        throw LibraryManagerException("User email cannot be empty");
    }
}
void LibraryManager::validateLoanData(const Loan& loan) const {
    if (loan.getLoanId().isEmpty()) {
        throw LibraryManagerException("Loan ID cannot be empty");
    }
    if (loan.getUserId().isEmpty()) {
        throw LibraryManagerException("Loan user ID cannot be empty");
    }
    if (loan.getResourceId().isEmpty()) {
        throw LibraryManagerException("Loan resource ID cannot be empty");
    }
}
void LibraryManager::addActiveLoan(std::unique_ptr<Loan> loan) {
    if (loan) {
        m_activeLoans.push_back(std::move(loan));
    }
}
void LibraryManager::addLoanHistory(std::unique_ptr<Loan> loan) {
    if (loan) {
        m_loanHistory.push_back(std::move(loan));
    }
}
void LibraryManager::addActiveReservation(std::unique_ptr<Reservation> reservation) {
    if (reservation) {
        m_activeReservations.push_back(std::move(reservation));
    }
}
void LibraryManager::addReservationHistory(std::unique_ptr<Reservation> reservation) {
    if (reservation) {
        m_reservationHistory.push_back(std::move(reservation));
    }
}