#include "logingithub.h"
#include "ui_logingithub.h"

loginGithub::loginGithub(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loginGithub)
{
    ui->setupUi(this);

    ui->incorrectPass->setVisible(false);
    ui->TwoFAIncorrectLabel->setVisible(false);
}

loginGithub::~loginGithub()
{
    delete ui;
}

void loginGithub::on_nextButton_clicked()
{
    this->setEnabled(false);
    switch (ui->pages->currentIndex()) {
        case 0: {
            QNetworkRequest loginRequest(QUrl("https://api.github.com/user"));
            loginRequest.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");

            //Add BASIC authentication data
            QString head = ui->usernameBox->text() + ":" + ui->passwordBox->text();
            QByteArray data = head.toLocal8Bit().toBase64();
            QString headerData = "BASIC " + data;
            loginRequest.setRawHeader("Authorization", headerData.toLocal8Bit());

            QNetworkReply* reply = netMgr.get(loginRequest);
            connect(reply, &QNetworkReply::finished, [=] {
                QByteArray replyData = reply->readAll();
                qDebug() << reply->rawHeaderPairs();
                qDebug() << replyData;

                switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) {
                    case 200: {
                        doLogin(replyData);
                        break;
                    }
                    case 401: {
                        QJsonDocument doc = QJsonDocument::fromJson(replyData);
                        QString error = doc.object().value("message").toString();
                        if (error == "Bad credentials") {
                            ui->incorrectPass->setVisible(true);
                        } else if (error == "Must specify two-factor authentication OTP code.") {
                            if (reply->rawHeader("X-Github-OTP") == "required; app") {
                                ui->TwoFARequestLabel->setText("Enter your Two Factor Authentication code obtained from the app.");
                            } else {
                                ui->TwoFARequestLabel->setText("Enter your Two Factor Authentication code obtained via SMS.");
                            }
                            ui->pages->setCurrentIndex(1);
                            ui->TwoFALineEdit->setFocus();
                        }
                        break;
                    }
                }

                this->setEnabled(true);
                reply->deleteLater();
            });
            connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                [=](QNetworkReply::NetworkError code) {
                qDebug() << "Error";
            });
            break;
        }
        case 1: {
            QNetworkRequest loginRequest(QUrl("https://api.github.com/user"));
            loginRequest.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");
            loginRequest.setRawHeader("X-GitHub-OTP", ui->TwoFALineEdit->text().toUtf8());

            //Add BASIC authentication data
            QString head = ui->usernameBox->text() + ":" + ui->passwordBox->text();
            QByteArray data = head.toLocal8Bit().toBase64();
            QString headerData = "BASIC " + data;
            loginRequest.setRawHeader("Authorization", headerData.toLocal8Bit());

            QNetworkReply* reply = netMgr.get(loginRequest);
            connect(reply, &QNetworkReply::finished, [=] {
                QByteArray replyData = reply->readAll();
                qDebug() << reply->rawHeaderPairs();
                qDebug() << replyData;
                switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) {
                    case 200: {
                        doLogin(replyData);
                        break;
                    }
                    case 401: {
                        QJsonDocument doc = QJsonDocument::fromJson(replyData);
                        QString error = doc.object().value("message").toString();
                        if (error == "Must specify two-factor authentication OTP code.") {
                            ui->TwoFAIncorrectLabel->setVisible(true);
                        }
                        break;
                    }
                }

                this->setEnabled(true);
                reply->deleteLater();
            });
            connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                [=](QNetworkReply::NetworkError code) {
                qDebug() << "Error";
            });
            break;
        }
    }
}

void loginGithub::on_TwoFALineEdit_textChanged(const QString &arg1)
{
    if (arg1.length() == 6) {
        ui->nextButton->click();
    }
}

void loginGithub::doLogin(QByteArray details) {
    ui->nextButton->setVisible(false);

    QJsonDocument doc = QJsonDocument::fromJson(details);
    QString name = doc.object().value("name").toString();

    ui->welcomingMessage->setText(tr("Hello %1! Give us a moment to log you in and create an authorization...").arg(name));

    ui->pages->setCurrentIndex(2);

    //Create the authorization
    QNetworkRequest request(QUrl("https://api.github.com/authorizations"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");
    request.setRawHeader("X-GitHub-OTP", ui->TwoFALineEdit->text().toUtf8());

    //Add BASIC authentication data
    QString head = ui->usernameBox->text() + ":" + ui->passwordBox->text();
    QByteArray data = head.toLocal8Bit().toBase64();
    QString headerData = "BASIC " + data;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    QJsonObject object;
    object.insert("client_id", "991b38e16a7f0ed6e63e");
    object.insert("client_secret", "68b75756843415505381b5a34a5266823cdbe98d");
    object.insert("note", "the* app issue creation");

    QJsonArray array;
    array.append("public_repo");
    object.insert("scopes", array);

    QJsonDocument document(object);

    QNetworkReply* reply = netMgr.post(request, document.toJson());
    connect(reply, &QNetworkReply::finished, [=] {
        QByteArray replyData = reply->readAll();
        qDebug() << reply->rawHeaderPairs();
        qDebug() << replyData;
        switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) {
            case 200:
            case 201: {
                QJsonDocument reply = QJsonDocument::fromJson(replyData);
                settings.setValue("login/token", reply.object().value("token").toString());

                ui->pages->setCurrentIndex(3);
                break;
            }
        }
        reply->deleteLater();
    });
}

void loginGithub::on_finishButton_clicked()
{
    this->close();
}
