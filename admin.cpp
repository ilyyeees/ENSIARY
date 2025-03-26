#include "admin.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// Initialize the static action log
std::vector<Admin::ActionLog> Admin::actionLog;

Admin::Admin()
    : User(), password("")
{
}

Admin::Admin(const std::string &name,
             const std::string &id,
             const std::string &password)
    : User(name, id, "", "", ""), password(password)
{
}

bool Admin::verifyPassword(const std::string &inputPassword) const
{
    return password == inputPassword;
}

bool Admin::isRoot() const
{
    return getId() == "root";
}

void Admin::logAction(AdminActionType actionType, const std::string &details)
{
    ActionLog log;
    log.adminId = getId();
    log.adminName = getName();
    log.actionType = actionType;
    log.actionDetails = details;
    
    // Get current time
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    log.timestamp = timestamp.toStdString();
    
    actionLog.push_back(log);
    
    // Also save to database for persistence
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO AdminActionLog (admin_id, admin_name, action_type, action_details, timestamp)
        VALUES (:admin_id, :admin_name, :action_type, :action_details, :timestamp)
    )");
    
    query.bindValue(":admin_id", QString::fromStdString(log.adminId));
    query.bindValue(":admin_name", QString::fromStdString(log.adminName));
    query.bindValue(":action_type", static_cast<int>(actionType));
    query.bindValue(":action_details", QString::fromStdString(log.actionDetails));
    query.bindValue(":timestamp", timestamp);
    
    if (!query.exec()) {
        qWarning() << "Failed to save admin action log:" << query.lastError().text();
    }
}

std::vector<Admin::ActionLog> Admin::getActionLog()
{
    // If the static log is empty, load it from the database
    if (actionLog.empty()) {
        loadActionLogFromDatabase();
    }
    
    return actionLog;
}

void Admin::loadActionLogFromDatabase()
{
    actionLog.clear();
    
    QSqlQuery query;
    query.prepare(R"(
        SELECT admin_id, admin_name, action_type, action_details, timestamp
        FROM AdminActionLog
        ORDER BY timestamp DESC
    )");
    
    if (!query.exec()) {
        qWarning() << "Failed to load admin action log:" << query.lastError().text();
        return;
    }
    
    while (query.next()) {
        ActionLog log;
        log.adminId = query.value("admin_id").toString().toStdString();
        log.adminName = query.value("admin_name").toString().toStdString();
        log.actionType = static_cast<AdminActionType>(query.value("action_type").toInt());
        log.actionDetails = query.value("action_details").toString().toStdString();
        log.timestamp = query.value("timestamp").toString().toStdString();
        
        actionLog.push_back(log);
    }
} 