
#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include "output_thumbnail.h"

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = Q_NULLPTR);

protected:
  void closeEvent(QCloseEvent* e);

private:
  Ui::MainWindow ui;
  OutputThumbnail m_ot;
  bool m_init;

private slots:
  void slotPushButtonSelectInputFile();
  void slotPushButtonSelectOutputDir();
};

#endif // MAIN_WINDOW_H_
