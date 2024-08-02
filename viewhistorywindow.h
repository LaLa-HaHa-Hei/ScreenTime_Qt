#ifndef VIEWHISTORYWINDOW_H
#define VIEWHISTORYWINDOW_H

#include <QMainWindow>
#include <mainwindow.h>

namespace Ui {
class ViewHistoryWindow;
}

class ViewHistoryWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ViewHistoryWindow(QString jsonFilePath, QWidget *parent = nullptr);
    ~ViewHistoryWindow();

private:
    Ui::ViewHistoryWindow *ui;
    QString jsonFilePath;
};

#endif // VIEWHISTORYWINDOW_H
