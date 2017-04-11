#ifndef LOGINGITHUB_H
#define LOGINGITHUB_H

#include <QDialog>
#include <QLabel>
#include <QStackedWidget>
#include <QLineEdit>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QJsonArray>
#include <QSettings>
#include <iostream>

namespace Ui {
class loginGithub;
}

class loginGithub : public QDialog
{
    Q_OBJECT

public:
    explicit loginGithub(QWidget *parent = 0);
    ~loginGithub();

private slots:
    void on_nextButton_clicked();

    void on_TwoFALineEdit_textChanged(const QString &arg1);

    void doLogin(QByteArray details);

    void on_finishButton_clicked();

private:
    Ui::loginGithub *ui;

    QNetworkAccessManager netMgr;
    QSettings settings;
};

#endif // LOGINGITHUB_H
