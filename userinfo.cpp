#include "userinfo.h"
#include "ui_userinfo.h"

UserInfo::UserInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserInfo)
{
    ui->setupUi(this);

    reloadInfo();
}

UserInfo::~UserInfo()
{
    delete ui;
}

void UserInfo::reloadInfo() {
    ui->userFrame->setVisible(false);

    QString authToken = settings.value("login/token", "").toString();
    if (authToken == "") {
        ui->logoutButton->setProperty("type", "normal");
        ui->userPicture->setPixmap(QIcon::fromTheme("user").pixmap(64, 64));
        ui->userName->setText("Not Logged In");
        ui->userFrame->setVisible(true);
        ui->logoutButton->setText("Log In");
        ui->logoutButton->setIcon(QIcon::fromTheme("user"));
    } else {
        ui->logoutButton->setProperty("type", "destructive");
        ui->logoutButton->setIcon(QIcon::fromTheme("system-log-out"));
        ui->logoutButton->setText("Log Out");

        //Ask for user details
        QNetworkRequest request(QUrl("https://api.github.com/user"));
        request.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");
        request.setRawHeader("Authorization", QString("token " + authToken).toUtf8());

        QNetworkReply* reply = netMgr.get(request);
        connect(reply, &QNetworkReply::finished, [=] {
            QByteArray replyData = reply->readAll();

            QJsonDocument doc = QJsonDocument::fromJson(replyData);
            QJsonObject obj = doc.object();

            QNetworkRequest imageRequest(QUrl(obj.value("avatar_url").toString()));
            QNetworkReply* imageReply = netMgr.get(imageRequest);
            connect(imageReply, &QNetworkReply::finished, [=] {
                QImage avatar;
                avatar.loadFromData(imageReply->readAll());

                ui->userPicture->setPixmap(QPixmap::fromImage(avatar.scaled(64, 64)));
                ui->userFrame->setVisible(true);
            });

            ui->userPicture->setPixmap(QIcon::fromTheme("user").pixmap(64, 64));
            ui->userName->setText(obj.value("name").toString());
        });
    }
}

void UserInfo::on_okButton_clicked()
{
    this->close();
}

void UserInfo::on_logoutButton_clicked()
{
    if (ui->logoutButton->text() == "Log Out") {
        ui->logoutButton->setText("Click again to log out.");
    } else if (ui->logoutButton->text() == "Log In") {
        loginGithub gh;
        gh.exec();
        reloadInfo();
    } else {
        //Revoke the token
        this->setEnabled(false);

        QString authToken = settings.value("login/token", "").toString();
        QNetworkRequest request(QUrl("https://api.github.com/applications/991b38e16a7f0ed6e63e/tokens/" + authToken));
        request.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");

        //Add BASIC authentication data
        QString head = "991b38e16a7f0ed6e63e:68b75756843415505381b5a34a5266823cdbe98d";
        QByteArray data = head.toLocal8Bit().toBase64();
        QString headerData = "BASIC " + data;
        request.setRawHeader("Authorization", headerData.toLocal8Bit());

        QNetworkReply* reply =netMgr.deleteResource(request);
        connect(reply, &QNetworkReply::finished, [=] {
            this->setEnabled(true);
            switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) {
                case 204: {
                    settings.setValue("login/token", "");
                    settings.setValue("login/username", "");

                    tToast* toast = new tToast;
                    toast->setTitle("Logged Out");
                    toast->setText("You have been logged out.");
                    toast->setTimeout(5000);
                    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
                    toast->show(this);

                    reloadInfo();
                    break;
                }
            }
        });
    }
}
