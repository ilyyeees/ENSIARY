#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QTimer>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <memory>
class LibraryManager;
class PersistenceService;
class Resource;
class User;
class Loan;
class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    std::unique_ptr<LibraryManager> m_libraryManager;
    std::unique_ptr<PersistenceService> m_persistenceService;
    QTabWidget* m_tabWidget;
    QStatusBar* m_statusBar;
    QMenuBar* m_menuBar;
    QToolBar* m_toolBar;
    QWidget* m_resourceTab;
    QLineEdit* m_resourceSearchEdit;
    QComboBox* m_resourceCategoryFilter;
    QComboBox* m_resourceStatusFilter;
    QTableWidget* m_resourceTable;
    QPushButton* m_addResourceBtn;
    QPushButton* m_editResourceBtn;
    QPushButton* m_removeResourceBtn;
    QPushButton* m_borrowResourceBtn;
    QPushButton* m_reserveResourceBtn;
    QPushButton* m_refreshResourcesBtn;
    QWidget* m_userTab;
    QLineEdit* m_userSearchEdit;
    QComboBox* m_userTypeFilter;
    QComboBox* m_userStatusFilter;
    QTableWidget* m_userTable;
    QPushButton* m_addUserBtn;
    QPushButton* m_editUserBtn;
    QPushButton* m_removeUserBtn;
    QPushButton* m_viewUserLoansBtn;
    QPushButton* m_manageReservationsBtn;
    QPushButton* m_refreshUsersBtn;
    QWidget* m_loanTab;
    QTableWidget* m_activeLoanTable;
    QTableWidget* m_overdueLoanTable;
    QPushButton* m_returnBookBtn;
    QPushButton* m_renewLoanBtn;
    QPushButton* m_refreshLoansBtn;
    QWidget* m_infoTab;
    QLabel* m_libraryNameLabel;
    QLabel* m_operatingHoursLabel;
    QListWidget* m_upcomingEventsList;
    QPushButton* m_addEventBtn;
    QPushButton* m_removeEventBtn;
    QTextEdit* m_statisticsText;
    QPushButton* m_refreshStatsBtn;
    QTimer* m_refreshTimer;
    QString m_selectedResourceId;
    QString m_selectedUserId;
    QString m_selectedLoanId;
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent* event) override;
private slots:
    void onResourceSearchChanged();
    void onResourceFilterChanged();
    void onResourceSelectionChanged();
    void onAddResourceClicked();
    void onEditResourceClicked();
    void onRemoveResourceClicked();
    void onBorrowResourceClicked();
    void onReserveResourceClicked();
    void onRefreshResourcesClicked();
    void onUserSearchChanged();
    void onUserFilterChanged();
    void onUserSelectionChanged();
    void onAddUserClicked();
    void onEditUserClicked();
    void onRemoveUserClicked();
    void onViewUserLoansClicked();
    void onManageReservationsClicked();
    void onRefreshUsersClicked();
    void onLoanSelectionChanged();
    void onReturnBookClicked();
    void onRenewLoanClicked();
    void onRefreshLoansClicked();
    void onAddEventClicked();
    void onRemoveEventClicked();
    void onRefreshStatsClicked();
    void onAutoRefresh();
    void onDataSaved();
    void onDataLoaded();
    void onResourceAdded(const QString& resourceId);
    void onResourceRemoved(const QString& resourceId);
    void onUserAdded(const QString& userId);
    void onUserRemoved(const QString& userId);
    void onResourceBorrowed(const QString& loanId, const QString& userId, const QString& resourceId);
    void onResourceReturned(const QString& loanId, const QString& userId, const QString& resourceId);
    void onLoanRenewed(const QString& loanId, const QDateTime& newDueDate);
    void onItemOverdue(const QString& loanId, const QString& userId, const QString& resourceId);
    void onResourceReserved(const QString& reservationId, const QString& userId, const QString& resourceId);
    void onReservationCancelled(const QString& reservationId, const QString& userId, const QString& resourceId);
    void onReservationExpired(const QString& reservationId, const QString& userId, const QString& resourceId);
    void onReservedResourceAvailable(const QString& reservationId, const QString& userId, const QString& resourceId);
private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupResourceTab();
    void setupUserTab();
    void setupLoanTab();
    void setupInfoTab();
    void setupConnections();
    void setupTimer();
    void loadResourceData();
    void loadUserData();
    void loadLoanData();
    void loadLibraryInfo();
    void refreshAllData();
    void populateResourceTable(const std::vector<Resource*>& resources);
    void updateResourceTable();
    void clearResourceSelection();
    void populateUserTable(const std::vector<User*>& users);
    void updateUserTable();
    void clearUserSelection();
    void populateActiveLoanTable(const std::vector<Loan*>& loans);
    void populateOverdueLoanTable(const std::vector<Loan*>& loans);
    void updateLoanTables();
    void clearLoanSelection();
    void showAddResourceDialog();
    void showEditResourceDialog(Resource* resource);
    void showAddUserDialog();
    void showEditUserDialog(User* user);
    void showUserLoansDialog(User* user);
    void showAddEventDialog();
    void showMessage(const QString& message, int timeout = 3000);
    void showError(const QString& error);
    void showSuccess(const QString& message);
    bool confirmAction(const QString& message);
    void updateStatistics();
    QString formatResourceInfo(const Resource& resource);
    QString formatUserInfo(const User& user);
    QString formatLoanInfo(const Loan& loan);
    void saveData();
    void loadData();
    void autoSaveData();
    bool validateResourceSelection();
    bool validateUserSelection();
    bool validateLoanSelection();
};
#endif