#ifndef ADMIN_H
#define ADMIN_H

#include "user.h"
#include <vector>
#include <string>

// Admin action types for logging
enum class AdminActionType {
    LOGIN,
    LOGOUT,
    ADD_BOOK,
    REMOVE_BOOK,
    ADD_STUDENT,
    UPDATE_STUDENT,
    REMOVE_STUDENT,
    BORROW_BOOK,
    RETURN_BOOK,
    ADD_ADMIN
};

// Class for managing admin authentication and actions
class Admin : public User
{
public:
    Admin();
    Admin(const std::string &name,
          const std::string &id,
          const std::string &password);

    bool verifyPassword(const std::string &inputPassword) const;
    bool isRoot() const;
    
    // Expose password for database operations (secure design would use better methods)
    std::string getPassword() const { return password; }

    // Action log entry
    struct ActionLog {
        std::string adminId;
        std::string adminName;
        AdminActionType actionType;
        std::string actionDetails;
        std::string timestamp;
    };

    // Add an action to the log
    void logAction(AdminActionType actionType, const std::string &details);
    
    // Get the action log
    static std::vector<ActionLog> getActionLog();
    
    // Load action log from database
    static void loadActionLogFromDatabase();

private:
    std::string password;
    static std::vector<ActionLog> actionLog; // Shared among all admins
};

#endif // ADMIN_H 