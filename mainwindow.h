#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include "ui_mainwindow.h" //才能使用ui->....
#include <windows.h>
#include <psapi.h>
#include <shellapi.h>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QLabel>
#include <QProgressBar>
#include <QFileInfo>
#include <QDebug>
#include <QPixmap>
#include <QFileIconProvider>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>
#include "viewhistorywindow.h"
#include <QDate>
#include <QJsonObject>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Settings
{
public:
    QString ExeIconFolderPath = ".\\user_data\\exe_icon";
    QString JsonDataFolderPath = ".\\user_data\\data";
    int GetTopWindowInterval_s = 1;
    int RefreshListWidgetIntervalList_s = 3;
//    void DefaultSetting();
};
static QString SecondsToQString(int seconds)//将秒数转化为小时分钟秒格式
{
    QString time = "";
    int sec = seconds % 60;
    seconds /= 60;
    int min = seconds % 60;
    seconds /= 60;
    int hour = seconds;
    if (hour != 0)
        time = QString::number(hour) + "小时";
    if (min != 0)
        time += QString::number(min) + "分钟";
    if (sec != 0)
        time += QString::number(sec) + "秒";
    return time;
}
class ExeItemWidget : public QListWidgetItem
{
public:
    QWidget* Widget;
    ExeItemWidget(const QString& name, const QString& exePath, const QString& iconPath, QString timeText, int percentage) : QListWidgetItem()
    {
        Widget = new QWidget();
        nameLabel = new QLabel(name, Widget);
        nameLabel->setToolTip(exePath);
        iconLabel = new QLabel(Widget);
        QPixmap pixmap(iconPath);
//            if (pixmap.isNull())
//            {
            QPixmap pixmap2("./img/img/unknowfile.png");
            iconLabel->setPixmap(pixmap2);
//            }
//            else
            iconLabel->setPixmap(pixmap);
        iconLabel->resize(iconLabel->pixmap()->size());
        timeLabel = new QLabel(timeText, Widget);
        progressbar = new QProgressBar(Widget);
        progressbar->setRange(0, 100);
        progressbar->setValue(percentage);
        progressbar->setMaximumHeight(15);
        mainLayout = new QHBoxLayout(Widget);
        textLayout = new QHBoxLayout();
        exceptIconLayout = new QVBoxLayout();

        mainLayout->setSpacing(5); // 设置主布局的间距为0
        mainLayout->setContentsMargins(0, 7, 0, 7); // 设置主布局的边距为0
        exceptIconLayout->setSpacing(0); // 设置子布局的间距为0
        exceptIconLayout->setContentsMargins(0, 0, 0, 0); // 设置子布局的边距为0

        textLayout->addWidget(nameLabel);
        textLayout->addWidget(timeLabel);
        mainLayout->addWidget(iconLabel);
        exceptIconLayout->addLayout(textLayout);
        exceptIconLayout->addWidget(progressbar);
        mainLayout->addLayout(exceptIconLayout);

        setSizeHint(Widget->sizeHint());
    }
private:
    QLabel* nameLabel;
    QLabel* iconLabel;
    QLabel* timeLabel;
    QProgressBar* progressbar;
    QHBoxLayout* mainLayout;
    QHBoxLayout* textLayout;
    QVBoxLayout* exceptIconLayout;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ExitApp()
    {
        SaveData();
        trayIcon->hide();
        trayIcon = nullptr;
        QApplication::quit();
    }
    void HideWindow()
    {
        hide();
        refreshListWidgetTimer.stop();
    }
    void ShowWindow()
    {
        show();
        raise();
        activateWindow();
        RefreshListWidget();
        refreshListWidgetTimer.start();
    }

private slots:
    void onAboutToQuit()//关机前保存数据
    {
        SaveData();
    }
    void on_actionOpenAppDir_triggered();//打开程序目录
    void on_actionExitApp_triggered();
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);//托盘点击
    void RefreshListWidget_slot() {RefreshListWidget();}
    void GetTopWindow_slot(){GetTopWindow();}
    void on_actionSelfStarting_triggered();//开机自启动
    void on_actionViewHistory_triggered();//打开"查看历史"窗口

private:
    class ExeItem
    {
    public:
        QString Name;
        QString ExePath;
        QString IconPath;
        int Seconds = 0;
    };
    Ui::MainWindow *ui;
    QTimer getTopWindowTimer;
    QTimer refreshListWidgetTimer;
    QSystemTrayIcon* trayIcon;
    QMenu* menu;

    int totalSeconds = 0;
    QString absluteJsonDataFolderPath;
    QString absluteExeIconFolderPath;
    QString absluteJsonDataFilePath;
    Settings settings;
    const QDate today = QDate::currentDate();
    QList<ExeItem*> exeItemList;
    void CheckDate()
    {
        if (today < QDateTime::currentDateTime().date())//已经第二天
        {
            ShellExecuteW(NULL, L"open", reinterpret_cast<const wchar_t *>(QCoreApplication::applicationFilePath().replace("/", "\\").utf16()), NULL, NULL, SW_SHOW);
            ExitApp();
        }
    }
    void SaveData()
    {
        std::sort(exeItemList.begin(), exeItemList.end(), CmpBySeconds);
        QJsonArray jsonArray;
        for (const ExeItem* item : exeItemList)
        {
            QJsonObject jsonObject;
            jsonObject["Name"] = item->Name;
            jsonObject["ExePath"] = item->ExePath;
            jsonObject["IconPath"] = item->IconPath;
            jsonObject["Percentage"] = item->Seconds * 100 / totalSeconds;
            jsonObject["TimeText"] = SecondsToQString(item->Seconds);
            jsonObject["Seconds"] = item->Seconds;

            jsonArray.append(jsonObject);
        }
        QJsonDocument jsonDoc(jsonArray);
        QFile file(absluteJsonDataFilePath);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(jsonDoc.toJson());
            file.close();
        }
    }
    static bool CmpBySeconds(const ExeItem* a, const ExeItem* b)
    {
        return a->Seconds > b->Seconds;
    }
    void RefreshListWidget()
    {
        std::sort(exeItemList.begin(), exeItemList.end(), CmpBySeconds);
        ui->statusbar->showMessage("总时间：" + SecondsToQString(totalSeconds));
        if (totalSeconds == 0)
            return;
        ui->listWidget->clear();
        for (auto &exe : exeItemList)
        {
            ExeItemWidget* item = new ExeItemWidget(exe->Name, exe->ExePath, exe->IconPath, SecondsToQString(exe->Seconds), exe->Seconds*100/totalSeconds);
//            item->setSizeHint(item->Widget->sizeHint());
            ui->listWidget->addItem(item);
            ui->listWidget->setItemWidget(item, item->Widget);
        }
    }
    void GetTopWindow()
    {
        totalSeconds += settings.GetTopWindowInterval_s;
        HWND activeWindowHandle = GetForegroundWindow();
        DWORD pid = 0;
        GetWindowThreadProcessId(activeWindowHandle, &pid);
        if (pid == 0)
        {return;}
        // 获取路径
        QString filePath;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess)
            return;
        WCHAR processPath[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, NULL, processPath, MAX_PATH) > 0)
        {
            CloseHandle(hProcess);
            filePath = QString::fromWCharArray(processPath);
        }
        else
        {
            CloseHandle(hProcess);
            return;
        }
        for (int i = 0; i < exeItemList.count(); i++)
        {
            if (QString::compare(exeItemList[i]->ExePath, filePath) == 0)
            {
                exeItemList[i]->Seconds += settings.GetTopWindowInterval_s;
                return;
            }
        }
//        没有找到，新建项目
        QFileInfo fileInfo(filePath);
        QString iconPath = filePath; //不能直接用filePath去remove，replace，会修改filePath
        iconPath = QString("%1\\%2.png").arg(absluteExeIconFolderPath, iconPath.remove(':').replace('\\', '$'));
        QFile file(iconPath);
        if (!file.exists()) //如果不存在图片就创建
        {
            QFileIconProvider iconProvider;
            QIcon icon = iconProvider.icon(filePath);
            if (icon.isNull())
            {
                iconPath = ":/img/img/unknowfile.png";
            }
            else
            {
                QPixmap pixmap = icon.pixmap(40, 40);
                pixmap.save(iconPath);
            }
        }
        ExeItem* item = new ExeItem();
        item->Name = fileInfo.baseName();
        item->ExePath = filePath;
        item->IconPath = iconPath;
        item->Seconds = settings.GetTopWindowInterval_s;
        exeItemList.append(item);
    }
    void InitializeTray()
    {
        menu = new QMenu(this);
        QIcon icon(":/img/img/32.ico");
        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setIcon(icon);
        menu->addAction(ui->actionSelfStarting);
        menu->addAction(ui->actionOpenAppDir);
        menu->addSeparator();
        menu->addAction(ui->actionExitApp);
        connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::on_activatedSysTrayIcon);
        trayIcon->setContextMenu(menu);
        trayIcon->show();
    }
    virtual void closeEvent(QCloseEvent* event) override
    {
        SaveData();
        //如果有托盘图标就隐藏窗口否则关闭窗口
        if(trayIcon->isVisible())
        {
            HideWindow();
            event->ignore();
        }
        else
        {
            event->accept();
    //        this->ExitApp();
        }
    }
};
#endif // MAINWINDOW_H
