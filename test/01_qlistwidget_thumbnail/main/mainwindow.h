
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);

protected:
    void closeEvent(QCloseEvent* e);

private:
    Ui::MainWindow ui;

    void dirFilesInfo(const QString& dirpath);

private slots:
    void slotSelectDir();
    void slotClipSelected(QListWidgetItem* item);
    void slotSetIconView();
    void slotSetListView();
};

#endif // MAIN_WINDOW_H_
