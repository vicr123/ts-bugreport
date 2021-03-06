#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <tpropertyanimation.h>
#include <ttoast.h>
#include <QSpacerItem>
#include <QScrollBar>
#include <QProgressBar>
#include <QToolButton>
#include <QMenu>
#include <QResizeEvent>
#include "logingithub.h"
#include "userinfo.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_loginButton_clicked();

    void reloadIssues();

    void on_programTheShell_clicked();

    void clearProgramSelection();

    void on_programTheMedia_clicked();

    void on_programTheFile_clicked();

    void on_programTheTerminal_clicked();

    void on_programThePhoto_clicked();

    void on_programTheCalculator_clicked();

    void on_programThePackage_clicked();

    void on_issuesWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void reloadLogins();

    void on_actionReport_new_bug_triggered();

    void on_cancelBugReportButton_clicked();

    void on_submitReportButton_clicked();

    void on_commentButton_clicked();

    void on_actionAccount_triggered();

    QString mdToHtml(QString markdown);

    void checkRateLimiting();

private:
    Ui::MainWindow *ui;

    QString getCurrentRepo();
    QString username;

    QNetworkAccessManager netMgr;
    QSettings settings;

    void resizeEvent(QResizeEvent* event);
    bool creatingIssue = false;
    bool rateLimitReached = false;
};

#endif // MAINWINDOW_H
