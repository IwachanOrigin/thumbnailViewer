
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

class QFileInfo;

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
  bool isMp4(const QFileInfo& fileInfo);

private slots:
  void slotSelectDir();
};

#endif // MAIN_WINDOW_H_
