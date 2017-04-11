#include "mainwindow.h"
#include "ui_mainwindow.h"

extern bool isLoggedIn();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->reportBugFrame->setParent(this);
    ui->reportBugFrame->setGeometry(0, 0, 0, 0);

    ui->detailsPane->setVisible(false);
    reloadIssues();
    reloadLogins();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loginButton_clicked()
{
    loginGithub gh;
    gh.exec();
    reloadLogins();
}

QString MainWindow::getCurrentRepo() {
    if (ui->programTheShell->isChecked()) {
        return "theshell";
    } else if (ui->programTheMedia->isChecked()) {
        return "themedia";
    } else if (ui->programTheFile->isChecked()) {
        return "thefile";
    } else if (ui->programTheTerminal->isChecked()) {
        return "theterminal";
    } else if (ui->programThePhoto->isChecked()) {
        return "thephoto";
    } else if (ui->programTheCalculator->isChecked()) {
        return "thecalculator";
    } else if (ui->programThePackage->isChecked()) {
        return "thepackage";
    }

    return "";
}

void MainWindow::reloadIssues() {
    ui->issuesWidget->clear();

    //Create request
    QNetworkRequest request(QUrl("https://api.github.com/repos/vicr123/" + getCurrentRepo() + "/issues"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");

    QString authToken = settings.value("login/token", "").toString();
    if (authToken != "") {
        request.setRawHeader("Authorization", QString("token " + authToken).toUtf8());
    }

    QNetworkReply* reply = netMgr.get(request);
    connect(reply, &QNetworkReply::finished, [=] {
        QByteArray response = reply->readAll();

        QJsonDocument document = QJsonDocument::fromJson(response);
        for (QJsonValue issue : document.array()) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setData(Qt::UserRole, issue.toObject());
            item->setText(issue.toObject().value("title").toString());
            ui->issuesWidget->addItem(item);
        }
    });
}

void MainWindow::clearProgramSelection()
{
    ui->programTheShell->setChecked(false);
    ui->programTheMedia->setChecked(false);
    ui->programTheFile->setChecked(false);
    ui->programTheTerminal->setChecked(false);
    ui->programThePhoto->setChecked(false);
    ui->programTheCalculator->setChecked(false);
    ui->programThePackage->setChecked(false);
}

void MainWindow::on_programTheShell_clicked()
{
    clearProgramSelection();
    ui->programTheShell->setChecked(true);
    reloadIssues();
}

void MainWindow::on_programTheMedia_clicked()
{
    clearProgramSelection();
    ui->programTheMedia->setChecked(true);
    reloadIssues();
}

void MainWindow::on_programTheFile_clicked()
{
    clearProgramSelection();
    ui->programTheFile->setChecked(true);
    reloadIssues();
}

void MainWindow::on_programTheTerminal_clicked()
{
    clearProgramSelection();
    ui->programTheTerminal->setChecked(true);
    reloadIssues();
}

void MainWindow::on_programThePhoto_clicked()
{
    clearProgramSelection();
    ui->programThePhoto->setChecked(true);
    reloadIssues();
}

void MainWindow::on_programTheCalculator_clicked()
{
    clearProgramSelection();
    ui->programTheCalculator->setChecked(true);
    reloadIssues();
}

void MainWindow::on_programThePackage_clicked()
{
    clearProgramSelection();
    ui->programThePackage->setChecked(true);
    reloadIssues();
}

void MainWindow::on_issuesWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == NULL) {
        ui->detailsPane->setVisible(false);
    } else {
        ui->detailsPane->setVisible(true);
        QJsonObject object = current->data(Qt::UserRole).toJsonObject();
        ui->issueTitle->setText(object.value("title").toString());

        if (object.value("locked").toBool()) {
            ui->newCommentFrame->setEnabled(false);
            ui->commentTextEdit->setPlaceholderText(tr("This thread has been locked. No new comments can be added."));
        } else {
            ui->newCommentFrame->setEnabled(true);
            ui->commentTextEdit->setPlaceholderText(tr("Write a comment..."));
        }

        QString state = object.value("state").toString();
        if (state == "open") {
            ui->IssueState->setText(tr("This issue is currently %1").arg("open."));
        } else if (state == "closed") {
            ui->IssueState->setText(tr("This issue is currently %1").arg("closed."));
        }

        QJsonObject author = object.value("user").toObject();
        ui->issueAuthor->setText(tr("Opened by %1.").arg(author.value("login").toString()));

        //Get all comments
        QNetworkRequest commentsReq(QUrl(object.value("comments_url").toString()));
        commentsReq.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");

        QString authToken = settings.value("login/token", "").toString();
        if (authToken != "") {
            commentsReq.setRawHeader("Authorization", QString("token " + authToken).toUtf8());
        }

        QNetworkReply* commentsReply = netMgr.get(commentsReq);
        connect(commentsReply, &QNetworkReply::finished, [=] {
            QFrame* firstComment = new QFrame();
            firstComment->setFrameShape(QFrame::StyledPanel);
            QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);

            QLabel* bodyLabel = new QLabel();
            bodyLabel->setWordWrap(true);
            bodyLabel->setText(object.value("body").toString());
            layout->addWidget(bodyLabel);

            QLabel* authorLabel = new QLabel();
            authorLabel->setWordWrap(true);
            authorLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            authorLabel->setDisabled(true);
            authorLabel->setText(author.value("login").toString());
            layout->addWidget(authorLabel);

            firstComment->setLayout(layout);
            ui->issuesDetailsContents->layout()->addWidget(firstComment);

            QByteArray comments = commentsReply->readAll();

            QJsonDocument commentsDoc = QJsonDocument::fromJson(comments);
            for (QJsonValue comment : commentsDoc.array()) {
                QJsonObject commentObj = comment.toObject();

                QFrame* commentFrame = new QFrame();
                commentFrame->setFrameShape(QFrame::StyledPanel);
                QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);

                QLabel* bodyLabel = new QLabel();
                bodyLabel->setWordWrap(true);
                bodyLabel->setText(commentObj.value("body").toString());
                layout->addWidget(bodyLabel);
                commentFrame->setLayout(layout);

                QJsonObject author = commentObj.value("user").toObject();

                QLabel* authorLabel = new QLabel();
                authorLabel->setWordWrap(true);
                authorLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                authorLabel->setDisabled(true);
                authorLabel->setText(author.value("login").toString());
                layout->addWidget(authorLabel);

                ui->issuesDetailsContents->layout()->addWidget(commentFrame);
            }
        });

        QLayoutItem* item;
        while ((item = ui->issuesDetailsContents->layout()->takeAt(0)) != NULL) {
            ui->issuesDetailsContents->layout()->removeItem(item);
            delete item->widget();
            delete item;
        }
    }
}

void MainWindow::reloadLogins() {
    QSettings settings;
    if (settings.value("login/token", "") == "") {
        ui->writeCommentsMessage->setVisible(true);
        ui->commentButton->setEnabled(false);
        ui->newBugLoginMessage->setVisible(true);
        ui->submitReportButton->setEnabled(false);
    } else {
        ui->writeCommentsMessage->setVisible(false);
        ui->commentButton->setEnabled(true);
        ui->newBugLoginMessage->setVisible(false);
        ui->submitReportButton->setEnabled(true);
    }
}

void MainWindow::on_actionReport_new_bug_triggered()
{
    ui->reportBugFrame->setGeometry(0, this->height(), this->width(), this->height() - ui->issuesWidget->y());

    tPropertyAnimation* anim = new tPropertyAnimation(ui->reportBugFrame, "geometry");
    anim->setStartValue(ui->reportBugFrame->geometry());
    anim->setEndValue(QRect(0, ui->issuesWidget->mapTo(this, QPoint(0, 0)).y(), this->width(), this->height() - ui->issuesWidget->mapTo(this, QPoint(0, 0)).y()));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void MainWindow::on_cancelBugReportButton_clicked()
{
    tPropertyAnimation* anim = new tPropertyAnimation(ui->reportBugFrame, "geometry");
    anim->setStartValue(ui->reportBugFrame->geometry());
    anim->setEndValue(QRect(0, this->height(), this->width(), this->height() - ui->issuesWidget->mapTo(this, QPoint(0, 0)).y()));
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();

    tToast* toast = new tToast;
    toast->setTitle("Bug Report Cancelled");
    toast->setText("Your bug report has been cancelled.");
    toast->setTimeout(5000);
    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
    toast->show(this);
}

void MainWindow::on_submitReportButton_clicked()
{
    ui->reportBugFrame->setEnabled(false);

    QJsonObject object;
    object.insert("title", ui->newBugTitle->text());
    object.insert("body", ui->newBugBody->toPlainText());

    QJsonDocument document(object);

    //Post Issue
    QNetworkRequest request(QUrl("https://api.github.com/repos/vicr123/" + getCurrentRepo() + "/issues"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");

    QString authToken = settings.value("login/token", "").toString();
    if (authToken != "") {
        request.setRawHeader("Authorization", QString("token " + authToken).toUtf8());
    }

    QNetworkReply* reply = netMgr.post(request, document.toJson());
    connect(reply, &QNetworkReply::finished, [=] {
        switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) {
            case 201: {
                ui->reportBugFrame->setEnabled(true);

                ui->newBugTitle->setText("");
                ui->newBugBody->setText("");
                reloadIssues();

                tPropertyAnimation* anim = new tPropertyAnimation(ui->reportBugFrame, "geometry");
                anim->setStartValue(ui->reportBugFrame->geometry());
                anim->setEndValue(QRect(0, this->height(), this->width(), this->height() - ui->issuesWidget->mapTo(this, QPoint(0, 0)).y()));
                anim->setEasingCurve(QEasingCurve::OutCubic);
                anim->setDuration(500);
                connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
                anim->start();

                tToast* toast = new tToast;
                toast->setTitle("Bug Report Created");
                toast->setText("Your bug report has been created successfully!");
                toast->setTimeout(5000);
                connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
                toast->show(this);
                break;
            }

            default: {
                ui->reportBugFrame->setEnabled(true);

                tToast* toast = new tToast;
                toast->setTitle("Error");
                toast->setText("Your bug report couldn't be sent.");
                toast->setTimeout(10000);

                QMap<QString, QString> actions;
                actions.insert("retry", "Retry");
                toast->setActions(actions);

                connect(toast, &tToast::actionClicked, [=](QString key) {
                    if (key == "retry") {
                        ui->submitReportButton->click();
                    }
                });
                connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
                toast->show(this);
            }
        }
    });
}

void MainWindow::on_commentButton_clicked()
{
    ui->newCommentFrame->setEnabled(false);

    QJsonObject object;
    object.insert("body", ui->commentTextEdit->toPlainText());

    QJsonDocument document(object);

    //Post Issue
    QNetworkRequest request(QUrl(ui->issuesWidget->currentItem()->data(Qt::UserRole).toJsonObject().value("comments_url").toString()));
    request.setHeader(QNetworkRequest::UserAgentHeader, "ts-bugreport/1.0");

    QString authToken = settings.value("login/token", "").toString();
    if (authToken != "") {
        request.setRawHeader("Authorization", QString("token " + authToken).toUtf8());
    }

    QNetworkReply* reply = netMgr.post(request, document.toJson());
    connect(reply, &QNetworkReply::finished, [=] {
        switch (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) {
            case 201: {
                ui->newCommentFrame->setEnabled(true);

                ui->commentTextEdit->setPlainText("");
                on_issuesWidget_currentItemChanged(ui->issuesWidget->currentItem(), ui->issuesWidget->currentItem());

                tToast* toast = new tToast;
                toast->setTitle("Comment Created");
                toast->setText("Your comment has been created successfully!");
                toast->setTimeout(5000);
                connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
                toast->show(this);
                break;
            }

            default: {
                ui->newCommentFrame->setEnabled(true);

                tToast* toast = new tToast;
                toast->setTitle("Error");
                toast->setText("Your comment couldn't be sent.");
                toast->setTimeout(10000);

                QMap<QString, QString> actions;
                actions.insert("retry", "Retry");
                toast->setActions(actions);

                connect(toast, &tToast::actionClicked, [=](QString key) {
                    if (key == "retry") {
                        ui->commentButton->click();
                    }
                });
                connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
                toast->show(this);
            }
        }
    });
}
