#ifndef RESERVATION_MANAGEMENT_DIALOG_H
#define RESERVATION_MANAGEMENT_DIALOG_H
#include <QDialog>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateEdit>
#include <QTabWidget>
class LibraryManager;
class Reservation;
class User;
class Resource;
class ReservationManagementDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReservationManagementDialog(LibraryManager* libraryManager, QWidget *parent = nullptr);
private slots:
    void onCreateReservation();
    void onCancelReservation();
    void onViewReservationDetails();
    void onRefreshData();
    void onActiveReservationSelectionChanged();
    void onHistoryReservationSelectionChanged();
    void onUserSelectionChanged();
    void onResourceSelectionChanged();
private:
    void setupUI();
    void setupCreateReservationTab();
    void setupActiveReservationsTab();
    void setupReservationHistoryTab();
    void populateUserComboBox();
    void populateResourceComboBox();
    void populateActiveReservations();
    void populateReservationHistory();
    void updateCreateButtonState();
    void showReservationDetails(const Reservation* reservation);
    LibraryManager* m_libraryManager;
    QTabWidget* m_tabWidget;
    QWidget* m_createTab;
    QComboBox* m_userComboBox;
    QComboBox* m_resourceComboBox;
    QDateEdit* m_expiryDateEdit;
    QPushButton* m_createButton;
    QLabel* m_statusLabel;
    QWidget* m_activeTab;
    QTableWidget* m_activeReservationsTable;
    QPushButton* m_cancelButton;
    QPushButton* m_viewDetailsButton;
    QPushButton* m_refreshButton;
    QLabel* m_activeCountLabel;
    QWidget* m_historyTab;
    QTableWidget* m_historyTable;
    QPushButton* m_refreshHistoryButton;
    QLabel* m_historyCountLabel;
    QString m_selectedActiveReservationId;
    QString m_selectedHistoryReservationId;
};
#endif