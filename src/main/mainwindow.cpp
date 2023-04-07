
#include "mainwindow.h"
#include "clipinfo.h"
#include "clipinfodelegate.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSize>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);

  ui.listWidgetMain->setItemDelegate(new ClipInfoDelegate());
  ui.listWidgetMain->setIconSize(QSize(200, 150));
  ui.listWidgetMain->setResizeMode(QListWidget::Adjust);

  QObject::connect(ui.toolButtonSearchDir, &QToolButton::pressed, this, &MainWindow::slotSelectDir);

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
      if (this->isMp4(info))
      {
        ui.listWidgetMain->addItem(new ClipInfo(QIcon(info.absoluteFilePath()), info.fileName()));
      }
      else
      {
        ui.listWidgetMain->addItem(new ClipInfo(QIcon(info.absoluteFilePath()), info.fileName()));
      }
    }
  }
}

bool MainWindow::isMp4(const QFileInfo& fileInfo)
{
  QMimeDatabase mimeDatabase;
  QMimeType mimeType;

  mimeType = mimeDatabase.mimeTypeForFile(fileInfo);

  if (mimeType.inherits("video/mp4"))
    return true;

  return false;
}

void MainWindow::slotSelectDir()
{
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::Directory);
  if (dialog.exec())
  {
    ui.listWidgetMain->clear();
    QString dirName = dialog.directory().absolutePath();
    this->dirFilesInfo(dirName);
  }
}


