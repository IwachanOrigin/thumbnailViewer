
#include <QIcon>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFileDialog>

#include "clipinfo.h"
#include "clipinfodelegate.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);

  ui.listWidgetMain->setItemDelegate(new ClipInfoDelegate());
  QObject::connect(ui.pushButtonSetListView, &QPushButton::pressed, this, &MainWindow::slotSetListView);
  QObject::connect(ui.pushButtonSetIconView, &QPushButton::pressed, this, &MainWindow::slotSetIconView);
  QObject::connect(ui.listWidgetMain, &QListWidget::itemClicked, this, &MainWindow::slotClipSelected);
  QObject::connect(ui.pushButtonSelectDir, &QPushButton::pressed, this, &MainWindow::slotSelectDir);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
}

void MainWindow::dirFilesInfo(const QString& dirpath)
{
  QDir dir(dirpath);
  if (dir.exists())
  {
    QFileInfoList infoList = dir.entryInfoList();
    for (int i = 0; i < infoList.size(); i++)
    {
      QFileInfo info = infoList.at(i);
      if (info.fileName().compare(".") == 0 || info.fileName().compare("..") == 0)
      {
        continue;
      }
      ui.listWidgetMain->addItem(new ClipInfo(info.fileName()));
    }
  }
}


void MainWindow::slotSelectDir()
{
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::Directory);
  if (dialog.exec())
  {
    QString dirName = dialog.directory().absolutePath();
    this->dirFilesInfo(dirName);
  }
}

void MainWindow::slotClipSelected(QListWidgetItem * item)
{
  ClipInfo *clipInfo = static_cast<ClipInfo *>(item);
  QMessageBox::information(this, clipInfo->getClipName(), clipInfo->getClipFileName());
}

void MainWindow::slotSetIconView()
{
  ui.listWidgetMain->setViewMode(QListView::IconMode);
}

void MainWindow::slotSetListView()
{
  ui.listWidgetMain->setViewMode(QListView::ListMode);
}

