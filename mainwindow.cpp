#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    today =  QDate::currentDate();
    //相对路径转绝对路径
    QString exeDirPath = QCoreApplication::applicationDirPath().replace('/', '\\');
    if (settings.ExeIconFolderPath.startsWith(".\\"))
    {
        absluteExeIconFolderPath = exeDirPath + settings.ExeIconFolderPath.mid(1);
    }
    if (settings.JsonDataFolderPath.startsWith(".\\"))
    {
        absluteJsonDataFolderPath = exeDirPath + settings.JsonDataFolderPath.mid(1);
    }
    absluteJsonDataFilePath = QString("%1\\%2.json").arg(absluteJsonDataFolderPath, today.toString("yyyy-MM-dd"));
    //创建存数据的文件夹
    QDir dir(exeDirPath+"\\user_data");
    if (!dir.exists())
        dir.mkdir(exeDirPath+"\\user_data");
    QDir dir1(absluteExeIconFolderPath);
    if (!dir1.exists())
        dir1.mkdir(absluteExeIconFolderPath);
    QDir dir2(absluteJsonDataFolderPath);
    if (!dir2.exists())
        dir2.mkdir(absluteJsonDataFolderPath);
    //是否有今天的数据,如果就就加载
    QFile file(absluteJsonDataFilePath);
    if (file.exists())
    {
        QFile file(absluteJsonDataFilePath);
        file.open((QIODevice::ReadOnly));
        QByteArray jsonData = file.readAll();
        file.close();
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData));
        QJsonArray jsonArray = jsonDoc.array();
        for (const QJsonValue& value : jsonArray)
        {
            QJsonObject jsonObject = value.toObject();
            ExeItem* exeItem = new ExeItem();
            exeItem->Name = jsonObject["Name"].toString();
            exeItem->ExePath = jsonObject["ExePath"].toString();
            exeItem->IconPath = jsonObject["IconPath"].toString();
            exeItem->Seconds = jsonObject["Seconds"].toInt();
            totalSeconds += exeItem->Seconds;
            exeItemList.append(exeItem);
        }
    }
    //托盘
    InitializeTray();
    //Timer绑定
    connect(&getTopWindowTimer, &QTimer::timeout, this, &MainWindow::GetTopWindow_slot);
    getTopWindowTimer.setInterval(settings.GetTopWindowInterval_s * 1000);
    getTopWindowTimer.setTimerType(Qt::PreciseTimer);
    getTopWindowTimer.start();
    connect(&refreshListWidgetTimer, &QTimer::timeout, this, &MainWindow::RefreshListWidget_slot);
    refreshListWidgetTimer.setInterval(settings.RefreshListWidgetIntervalList_s * 1000);
    //检测是否开机自启动
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (settings.contains("ScreenTime"))
        ui->actionSelfStarting->setChecked(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionOpenAppDir_triggered()
{
    const wchar_t* wPath = reinterpret_cast<const wchar_t *>(QString("/select,%1").arg(QCoreApplication::applicationFilePath().replace("/", "\\")).utf16());
    ShellExecuteW(NULL, L"open", L"explorer.exe", wPath, NULL, SW_SHOW);
}


void MainWindow::on_actionExitApp_triggered(){ExitApp();}

void MainWindow::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        if (this->isVisible())
        {
            this->HideWindow();
        }
        else
        {
            this->ShowWindow();
        }
    default:
        break;
    }
}

void MainWindow::on_actionSelfStarting_triggered()
{
    if (!ui->actionSelfStarting->isChecked()) //点击后的状态所以取反
    {
        //删除注册表
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        settings.remove("ScreenTime");
    }
    else
    {
        //添加注册表
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        settings.setValue("ScreenTime", '"' + QCoreApplication::applicationFilePath().replace('/', '\\') + '"');
    }
}

void MainWindow::on_actionViewHistory_triggered()
{
    QString jsonFilePath = QFileDialog::getOpenFileName(nullptr, "选择 JSON 文件",absluteJsonDataFolderPath,"JSON Files (*.json);;All Files (*)");
    if (jsonFilePath.isEmpty())
        return;
    else
    {
        ViewHistoryWindow* w = new ViewHistoryWindow(jsonFilePath);
        w->show();
    }
}
