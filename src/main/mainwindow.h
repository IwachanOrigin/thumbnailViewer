
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"

class QFileInfo;
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
  bool isMovie(const QFileInfo& fileInfo);

private slots:
  void slotSetDirPath(const QString& dirPath);
  void slotClipSelected(QListWidgetItem* item);
};

#endif // MAIN_WINDOW_H_
