#include "persistence_service.h"
#include "library_manager.h"
#include "../models/resource.h"
#include "../models/book.h"
#include "../models/article.h"
#include "../models/thesis.h"
#include "../models/digitalcontent.h"
#include "../models/user.h"
#include "../models/loan.h"
#include "../models/reservation.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>
PersistenceService::PersistenceService(const QString& dataDirectory)
    : m_dataDirectory(dataDirectory) {
    m_resourcesFile = m_dataDirectory + "/resources.json";
    m_usersFile = m_dataDirectory + "/users.json";
    m_loansFile = m_dataDirectory + "/loans.json";
    m_reservationsFile = m_dataDirectory + "/reservations.json";
    m_configFile = m_dataDirectory + "/config.json";
    initializeDataDirectory();
}
bool PersistenceService::saveLibraryData(const LibraryManager& libraryManager) {
    clearError();
    try {
        bool success = true;
        QJsonObject config;
        config["libraryName"] = libraryManager.getLibraryName();
        config["operatingHours"] = libraryManager.getOperatingHours();
        config["defaultLoanPeriod"] = libraryManager.getDefaultLoanPeriod();
        QJsonArray eventsArray;
        for (const QString& event : libraryManager.getUpcomingEvents()) {
            eventsArray.append(event);
        }
        config["upcomingEvents"] = eventsArray;
        config["lastSaved"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        success &= saveConfiguration(config);
        auto resources = libraryManager.getAllResources();
        std::vector<std::unique_ptr<Resource>> resourcesCopy;
        for (const Resource* resource : resources) {
            if (const Book* book = dynamic_cast<const Book*>(resource)) {
                resourcesCopy.push_back(std::make_unique<Book>(*book));
            } else if (const Article* article = dynamic_cast<const Article*>(resource)) {
                resourcesCopy.push_back(std::make_unique<Article>(*article));
            }
        }
        success &= saveResources(resourcesCopy);
        auto users = libraryManager.getAllUsers();
        std::vector<std::unique_ptr<User>> usersCopy;
        for (const User* user : users) {
            usersCopy.push_back(std::make_unique<User>(*user));
        }        success &= saveUsers(usersCopy);
        auto activeLoans = libraryManager.getActiveLoans();
        auto loanHistory = libraryManager.getLoanHistory();
        std::vector<std::unique_ptr<Loan>> activeLoansCopy;
        std::vector<std::unique_ptr<Loan>> loanHistoryCopy;
        for (const Loan* loan : activeLoans) {
            activeLoansCopy.push_back(std::make_unique<Loan>(*loan));
        }
        for (const Loan* loan : loanHistory) {
            loanHistoryCopy.push_back(std::make_unique<Loan>(*loan));
        }
        success &= saveLoans(activeLoansCopy, loanHistoryCopy);
        auto activeReservations = libraryManager.getActiveReservations();
        auto reservationHistory = libraryManager.getReservationHistory();
        std::vector<std::unique_ptr<Reservation>> activeReservationsCopy;
        std::vector<std::unique_ptr<Reservation>> reservationHistoryCopy;
        for (const Reservation* reservation : activeReservations) {
            activeReservationsCopy.push_back(std::make_unique<Reservation>(*reservation));
        }
        for (const Reservation* reservation : reservationHistory) {
            reservationHistoryCopy.push_back(std::make_unique<Reservation>(*reservation));
        }
        success &= saveReservations(activeReservationsCopy, reservationHistoryCopy);
        return success;
    } catch (const std::exception& e) {
        setError(QString("Failed to save library data: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::loadLibraryData(LibraryManager& libraryManager) {
    clearError();
    try {
        QJsonObject config;        if (loadConfiguration(config)) {
            libraryManager.setLibraryName(config["libraryName"].toString());
            libraryManager.setOperatingHours(config["operatingHours"].toString());
            libraryManager.setDefaultLoanPeriod(config["defaultLoanPeriod"].toInt());
            QJsonArray eventsArray = config["upcomingEvents"].toArray();
            for (const QJsonValue& value : eventsArray) {
                libraryManager.addUpcomingEvent(value.toString());
            }
        }
        bool success = true;
        std::vector<std::unique_ptr<Resource>> resources;
        if (loadResources(resources)) {
            for (auto& resource : resources) {
                libraryManager.addResource(std::move(resource));
            }
        } else {
            qDebug() << "No resources file found, starting with empty resources";
        }
        std::vector<std::unique_ptr<User>> users;
        if (loadUsers(users)) {
            for (auto& user : users) {
                libraryManager.addUser(std::move(user));
            }
        } else {
            qDebug() << "No users file found, starting with empty users";
        }
        std::vector<std::unique_ptr<Loan>> activeLoans;
        std::vector<std::unique_ptr<Loan>> loanHistory;
        if (loadLoans(activeLoans, loanHistory)) {
            for (auto& loan : activeLoans) {
                libraryManager.addActiveLoan(std::move(loan));
            }
            for (auto& loan : loanHistory) {
                libraryManager.addLoanHistory(std::move(loan));
            }
        } else {
            qDebug() << "No loans file found, starting with empty loans";
        }
        std::vector<std::unique_ptr<Reservation>> activeReservations;
        std::vector<std::unique_ptr<Reservation>> reservationHistory;
        if (loadReservations(activeReservations, reservationHistory)) {
            for (auto& reservation : activeReservations) {
                libraryManager.addActiveReservation(std::move(reservation));
            }
            for (auto& reservation : reservationHistory) {
                libraryManager.addReservationHistory(std::move(reservation));
            }
        } else {
            qDebug() << "No reservations file found, starting with empty reservations";
        }        
        qDebug() << "Library data loaded successfully. Starting with clean slate.";
        return success;
    } catch (const std::exception& e) {
        setError(QString("Failed to load library data: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::saveResources(const std::vector<std::unique_ptr<Resource>>& resources) {
    clearError();
    try {
        QJsonArray resourcesArray = resourcesToJsonArray(resources);
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "resources";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["count"] = resourcesArray.size();
        root["data"] = resourcesArray;
        QJsonDocument document(root);
        return writeJsonToFile(m_resourcesFile, document);
    } catch (const std::exception& e) {
        setError(QString("Failed to save resources: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::loadResources(std::vector<std::unique_ptr<Resource>>& resources) {
    clearError();
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_resourcesFile, document)) {
            return false;
        }
        if (!validateJsonStructure(document, "resources")) {
            return false;
        }
        QJsonObject root = document.object();
        QJsonArray resourcesArray = root["data"].toArray();
        return jsonArrayToResources(resourcesArray, resources);
    } catch (const std::exception& e) {
        setError(QString("Failed to load resources: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::saveUsers(const std::vector<std::unique_ptr<User>>& users) {
    clearError();
    try {
        QJsonArray usersArray = usersToJsonArray(users);
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "users";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["count"] = usersArray.size();
        root["data"] = usersArray;
        QJsonDocument document(root);
        return writeJsonToFile(m_usersFile, document);
    } catch (const std::exception& e) {
        setError(QString("Failed to save users: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::loadUsers(std::vector<std::unique_ptr<User>>& users) {
    clearError();
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_usersFile, document)) {
            return false;
        }
        if (!validateJsonStructure(document, "users")) {
            return false;
        }
        QJsonObject root = document.object();
        QJsonArray usersArray = root["data"].toArray();
        return jsonArrayToUsers(usersArray, users);
    } catch (const std::exception& e) {
        setError(QString("Failed to load users: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::saveLoans(const std::vector<std::unique_ptr<Loan>>& activeLoans,
                                  const std::vector<std::unique_ptr<Loan>>& loanHistory) {
    clearError();
    try {
        QJsonArray activeLoansArray = loansToJsonArray(activeLoans);
        QJsonArray loanHistoryArray = loansToJsonArray(loanHistory);
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "loans";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["activeLoansCount"] = activeLoansArray.size();
        root["loanHistoryCount"] = loanHistoryArray.size();
        root["activeLoans"] = activeLoansArray;
        root["loanHistory"] = loanHistoryArray;
        QJsonDocument document(root);
        return writeJsonToFile(m_loansFile, document);
    } catch (const std::exception& e) {
        setError(QString("Failed to save loans: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::loadLoans(std::vector<std::unique_ptr<Loan>>& activeLoans,
                                  std::vector<std::unique_ptr<Loan>>& loanHistory) {
    clearError();
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_loansFile, document)) {
            return false;
        }
        if (!validateJsonStructure(document, "loans")) {
            return false;
        }
        QJsonObject root = document.object();
        QJsonArray activeLoansArray = root["activeLoans"].toArray();
        QJsonArray loanHistoryArray = root["loanHistory"].toArray();
        bool success = true;
        success &= jsonArrayToLoans(activeLoansArray, activeLoans);
        success &= jsonArrayToLoans(loanHistoryArray, loanHistory);
        return success;
    } catch (const std::exception& e) {
        setError(QString("Failed to load loans: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::saveReservations(const std::vector<std::unique_ptr<Reservation>>& activeReservations,
                                          const std::vector<std::unique_ptr<Reservation>>& reservationHistory) {
    clearError();
    try {
        QJsonArray activeReservationsArray = reservationsToJsonArray(activeReservations);
        QJsonArray reservationHistoryArray = reservationsToJsonArray(reservationHistory);
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "reservations";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["activeReservationsCount"] = activeReservationsArray.size();
        root["reservationHistoryCount"] = reservationHistoryArray.size();
        root["activeReservations"] = activeReservationsArray;
        root["reservationHistory"] = reservationHistoryArray;
        QJsonDocument document(root);
        return writeJsonToFile(m_reservationsFile, document);
    } catch (const std::exception& e) {
        setError(QString("Failed to save reservations: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::loadReservations(std::vector<std::unique_ptr<Reservation>>& activeReservations,
                                          std::vector<std::unique_ptr<Reservation>>& reservationHistory) {
    clearError();
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_reservationsFile, document)) {
            return false;
        }
        if (!validateJsonStructure(document, "reservations")) {
            return false;
        }
        QJsonObject root = document.object();
        QJsonArray activeReservationsArray = root["activeReservations"].toArray();
        QJsonArray reservationHistoryArray = root["reservationHistory"].toArray();
        bool success = true;
        success &= jsonArrayToReservations(activeReservationsArray, activeReservations);
        success &= jsonArrayToReservations(reservationHistoryArray, reservationHistory);
        return success;
    } catch (const std::exception& e) {
        setError(QString("Failed to load reservations: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::saveConfiguration(const QJsonObject& config) {
    clearError();
    try {
        QJsonObject root;
        root["version"] = "1.0";
        root["type"] = "configuration";
        root["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["data"] = config;
        QJsonDocument document(root);
        return writeJsonToFile(m_configFile, document);
    } catch (const std::exception& e) {
        setError(QString("Failed to save configuration: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::loadConfiguration(QJsonObject& config) {
    clearError();
    try {
        QJsonDocument document;
        if (!readJsonFromFile(m_configFile, document)) {
            return false;
        }
        if (!validateJsonStructure(document, "configuration")) {
            return false;
        }
        QJsonObject root = document.object();
        config = root["data"].toObject();
        return true;
    } catch (const std::exception& e) {
        setError(QString("Failed to load configuration: %1").arg(e.what()));
        return false;
    }
}
bool PersistenceService::initializeDataDirectory() {
    QDir dir;
    if (!dir.exists(m_dataDirectory)) {
        if (!dir.mkpath(m_dataDirectory)) {
            setError("Failed to create data directory: " + m_dataDirectory);
            return false;
        }
    }
    return true;
}
bool PersistenceService::backupData(const QString& backupSuffix) {
    QString suffix = backupSuffix.isEmpty() ? 
                    QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") : 
                    backupSuffix;
    QStringList filesToBackup = {m_resourcesFile, m_usersFile, m_loansFile, m_configFile};
    for (const QString& file : filesToBackup) {
        if (QFile::exists(file)) {
            QString backupFile = file + ".backup_" + suffix;
            if (!QFile::copy(file, backupFile)) {
                setError("Failed to backup file: " + file);
                return false;
            }
        }
    }
    return true;
}
bool PersistenceService::restoreFromBackup(const QString& backupSuffix) {
    QStringList filesToRestore = {m_resourcesFile, m_usersFile, m_loansFile, m_configFile};
    for (const QString& file : filesToRestore) {
        QString backupFile = file + ".backup_" + backupSuffix;
        if (QFile::exists(backupFile)) {
            if (QFile::exists(file)) {
                QFile::remove(file);
            }
            if (!QFile::copy(backupFile, file)) {
                setError("Failed to restore file: " + file);
                return false;
            }
        }
    }
    return true;
}
bool PersistenceService::validateJsonStructure(const QJsonDocument& doc, const QString& expectedType) {
    if (!doc.isObject()) {
        setError("Invalid JSON document: not an object");
        return false;
    }
    QJsonObject root = doc.object();
    if (!root.contains("type") || root["type"].toString() != expectedType) {
        setError("Invalid JSON type: expected " + expectedType);
        return false;
    }
    if (!root.contains("version")) {
        setError("Missing version information");
        return false;
    }
    return true;
}
bool PersistenceService::attemptDataRecovery() {
    QDir dataDir(m_dataDirectory);
    QStringList backupFiles = dataDir.entryList(QStringList() << "*.backup_*", QDir::Files);
    if (backupFiles.isEmpty()) {
        setError("No backup files found for recovery");
        return false;
    }
    QString mostRecentBackup;
    for (const QString& backup : backupFiles) {
        if (backup.contains("config.json.backup_")) {
            mostRecentBackup = backup.mid(backup.indexOf("backup_") + 7);
            break;
        }
    }
    if (mostRecentBackup.isEmpty()) {
        setError("Could not determine backup suffix");
        return false;
    }
    return restoreFromBackup(mostRecentBackup);
}
bool PersistenceService::validateResourceJson(const QJsonObject& json) {
    QStringList requiredFields = {"id", "title", "type", "author", "year", "isAvailable"};
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in resource JSON: %1").arg(field));
            return false;
        }
    }
    return true;
}
bool PersistenceService::validateUserJson(const QJsonObject& json) {
    QStringList requiredFields = {"id", "name", "email", "type", "status", "registrationDate"};
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in user JSON: %1").arg(field));
            return false;
        }
    }
    return true;
}
bool PersistenceService::validateLoanJson(const QJsonObject& json) {
    QStringList requiredFields = {"id", "userId", "resourceId", "loanDate", "dueDate", "status"};
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in loan JSON: %1").arg(field));
            return false;
        }
    }
    return true;
}
bool PersistenceService::validateReservationJson(const QJsonObject& json) {
    QStringList requiredFields = {"id", "userId", "resourceId", "reservationDate", "expiryDate", "status"};
    for (const QString& field : requiredFields) {
        if (!json.contains(field)) {
            setError(QString("Missing required field in reservation JSON: %1").arg(field));
            return false;
        }
    }
    return true;
}
bool PersistenceService::writeJsonToFile(const QString& filePath, const QJsonDocument& document) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        setError("Cannot open file for writing: " + filePath);
        return false;
    }
    qint64 bytesWritten = file.write(document.toJson());
    if (bytesWritten == -1) {
        setError("Failed to write to file: " + filePath);
        return false;
    }
    return true;
}
bool PersistenceService::readJsonFromFile(const QString& filePath, QJsonDocument& document) {
    QFile file(filePath);
    if (!file.exists()) {
        setError("File does not exist: " + filePath);
        return false;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        setError("Cannot open file for reading: " + filePath);
        return false;
    }
    QByteArray data = file.readAll();
    QJsonParseError error;
    document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        setError("JSON parse error: " + error.errorString());
        return false;
    }
    return true;
}
QJsonArray PersistenceService::resourcesToJsonArray(const std::vector<std::unique_ptr<Resource>>& resources) {
    QJsonArray array;
    for (const auto& resource : resources) {
        array.append(resource->toJson());
    }
    return array;
}
bool PersistenceService::jsonArrayToResources(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Resource>>& resources) {
    resources.clear();
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid resource JSON: not an object");
            return false;
        }
        QJsonObject resourceJson = value.toObject();
        auto resource = createResourceFromJson(resourceJson);
        if (resource) {
            resources.push_back(std::move(resource));
        } else {
            setError("Failed to create resource from JSON");
            return false;
        }
    }
    return true;
}
QJsonArray PersistenceService::usersToJsonArray(const std::vector<std::unique_ptr<User>>& users) {
    QJsonArray array;
    for (const auto& user : users) {
        array.append(user->toJson());
    }
    return array;
}
bool PersistenceService::jsonArrayToUsers(const QJsonArray& jsonArray, std::vector<std::unique_ptr<User>>& users) {
    users.clear();
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid user JSON: not an object");
            return false;
        }
        QJsonObject userJson = value.toObject();
        auto user = createUserFromJson(userJson);
        if (user) {
            users.push_back(std::move(user));
        } else {
            setError("Failed to create user from JSON");
            return false;
        }
    }
    return true;
}
QJsonArray PersistenceService::loansToJsonArray(const std::vector<std::unique_ptr<Loan>>& loans) {
    QJsonArray array;
    for (const auto& loan : loans) {
        array.append(loan->toJson());
    }
    return array;
}
bool PersistenceService::jsonArrayToLoans(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Loan>>& loans) {
    loans.clear();
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid loan JSON: not an object");
            return false;
        }
        QJsonObject loanJson = value.toObject();
        auto loan = createLoanFromJson(loanJson);
        if (loan) {
            loans.push_back(std::move(loan));
        } else {
            setError("Failed to create loan from JSON");
            return false;
        }
    }
    return true;
}
QJsonArray PersistenceService::reservationsToJsonArray(const std::vector<std::unique_ptr<Reservation>>& reservations) {
    QJsonArray array;
    for (const auto& reservation : reservations) {
        array.append(reservation->toJson());
    }
    return array;
}
bool PersistenceService::jsonArrayToReservations(const QJsonArray& jsonArray, std::vector<std::unique_ptr<Reservation>>& reservations) {
    reservations.clear();
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) {
            setError("Invalid reservation JSON: not an object");
            return false;
        }
        QJsonObject reservationJson = value.toObject();
        auto reservation = createReservationFromJson(reservationJson);
        if (reservation) {
            reservations.push_back(std::move(reservation));
        } else {
            setError("Failed to create reservation from JSON");
            return false;
        }
    }
    return true;
}
QJsonObject PersistenceService::createResourceJson(const Resource& resource) {
    return resource.toJson();
}
std::unique_ptr<Resource> PersistenceService::createResourceFromJson(const QJsonObject& json) {
    QString type = json["type"].toString();
      if (type == "Book") {
        auto book = std::make_unique<Book>("", "", "", 2000, "", "");
        book->fromJson(json);
        return std::move(book);
    } else if (type == "Article") {
        auto article = std::make_unique<Article>("", "", "", 2000, "");
        article->fromJson(json);
        return std::move(article);    } else if (type == "Thesis") {
        auto thesis = std::make_unique<Thesis>("", "", "", 2000);
        thesis->fromJson(json);
        return std::move(thesis);
    } else if (type == "Digital Content") {
        auto digitalContent = std::make_unique<DigitalContent>("", "", "", 2000);
        digitalContent->fromJson(json);
        return std::move(digitalContent);
    }
    return nullptr;
}
QJsonObject PersistenceService::createUserJson(const User& user) {
    return user.toJson();
}
std::unique_ptr<User> PersistenceService::createUserFromJson(const QJsonObject& json) {
    QString userId = json["userId"].toString();
    QString firstName = json["firstName"].toString();
    QString lastName = json["lastName"].toString();
    QString email = json["email"].toString();
    auto user = std::make_unique<User>(userId, firstName, lastName, email);
    user->setPhoneNumber(json["phoneNumber"].toString());
    user->setAddress(json["address"].toString());
    user->setUserType(User::stringToUserType(json["userType"].toString()));
    user->setStatus(User::stringToStatus(json["status"].toString()));
    user->setMaxBorrowLimit(json["maxBorrowLimit"].toInt());
    user->setNotes(json["notes"].toString());
    user->setYear(json["year"].toInt(-1));
    QJsonArray currentLoansArray = json["currentLoans"].toArray();
    for (const QJsonValue& value : currentLoansArray) {
        auto loan = std::make_unique<Loan>("", "", "", "", QDateTime(), QDateTime());
        loan->fromJson(value.toObject());
        user->addCurrentLoan(std::move(loan));
    }
    return user;
}
QJsonObject PersistenceService::createLoanJson(const Loan& loan) {
    return loan.toJson();
}
std::unique_ptr<Loan> PersistenceService::createLoanFromJson(const QJsonObject& json) {
    auto loan = std::make_unique<Loan>("", "", "", "", QDateTime(), QDateTime());
    loan->fromJson(json);
    return loan;
}
QJsonObject PersistenceService::createReservationJson(const Reservation& reservation) {
    return reservation.toJson();
}
std::unique_ptr<Reservation> PersistenceService::createReservationFromJson(const QJsonObject& json) {
    auto reservation = std::make_unique<Reservation>("", "", "", 7);
    reservation->fromJson(json);
    return reservation;
}
void PersistenceService::setError(const QString& error) {
    m_lastError = error;
    qDebug() << "PersistenceService Error:" << error;
}
void PersistenceService::clearError() {
    m_lastError.clear();
}