#ifndef USERINFO_H
#define USERINFO_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QLabel>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include "logingithub.h"
#include "ttoast.h"

namespace Ui {
class UserInfo;
}

class UserInfo : public QDialog
{
    Q_OBJECT

public:
    explicit UserInfo(QWidget *parent = 0);
    ~UserInfo();

private slots:
    void on_okButton_clicked();

    void on_logoutButton_clicked();

    void reloadInfo();

private:
    Ui::UserInfo *ui;

    QNetworkAccessManager netMgr;
    QSettings settings;
};

#endif // USERINFO_H
