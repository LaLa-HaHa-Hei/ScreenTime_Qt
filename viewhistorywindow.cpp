#include "viewhistorywindow.h"
#include "ui_viewhistorywindow.h"

ViewHistoryWindow::ViewHistoryWindow(QString jsonFilePath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ViewHistoryWindow)
{
    ui->setupUi(this);
    //加载知道的json文件
    QFile file(jsonFilePath);
    file.open((QIODevice::ReadOnly));
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData));

    QJsonArray jsonArray = jsonDoc.array();
    int totalSeconds = 0;
    for (const QJsonValue& value : jsonArray)
    {
        QJsonObject jsonObject = value.toObject();
        totalSeconds += jsonObject["Seconds"].toInt();
        ExeItemWidget* exeItemWidget = new ExeItemWidget(jsonObject["Name"].toString(), jsonObject["ExePath"].toString(), jsonObject["IconPath"].toString(), jsonObject["TimeText"].toString(), jsonObject["Percentage"].toInt());
        ui->listWidget->addItem(exeItemWidget);
        ui->listWidget->setItemWidget(exeItemWidget, exeItemWidget->Widget);
    }
    ui->statusbar->showMessage("总时间：" + SecondsToQString(totalSeconds));
}

ViewHistoryWindow::~ViewHistoryWindow()
{
    delete ui;
}
