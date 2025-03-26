#include <QApplication>
#include <QIcon>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QGroupBox>
#include <QDateTime>
#include <QStyle>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QToolButton>
#include <QIcon>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QScreen>
#include <QDialog>
#include <QSplitter>
#include <QFrame>
#include <QPropertyAnimation>
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>

#include "mainwindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set application icon
    a.setWindowIcon(QIcon(":/Icon.png"));
    
    // Create the main window but don't show it yet (handled in MainWindow constructor)
    MainWindow w;
    
    return a.exec();
} 