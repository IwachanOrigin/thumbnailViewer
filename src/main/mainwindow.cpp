
#include "mainwindow.h"
#include "clipinfo.h"
#include "clipinfodelegate.h"
#include "output_thumbnail.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSize>
#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);

  ui.listWidgetMain->setItemDelegate(new ClipInfoDelegate());
  ui.listWidgetMain->setIconSize(QSize(200, 150));
  ui.listWidgetMain->setResizeMode(QListWidget::Adjust);

  QObject::connect(ui.toolButtonSearchDir, &QToolButton::pressed, this, &MainWindow::slotSelectDir);
  QObject::connect(ui.listWidgetMain, &QListWidget::itemClicked, this, &MainWindow::slotClipSelected);
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
        OutputThumbnail ot;
        ot.createAPI();
        QImage thumbnail;
        ot.open(info.absoluteFilePath().toStdString(), thumbnail);
        QPixmap pixmap = QPixmap::fromImage(thumbnail);
        ui.listWidgetMain->addItem(new ClipInfo(QIcon(pixmap), info.fileName(), info.absoluteFilePath()));
        ot.destroyAPI();
      }
      else
      {
        ui.listWidgetMain->addItem(new ClipInfo(QIcon(info.absoluteFilePath()), info.fileName(), info.absoluteFilePath()));
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
    ui.plainTextEditFilePath->setPlainText(dirName);
  }
}

void MainWindow::slotClipSelected(QListWidgetItem* item)
{
  ClipInfo* clipInfo = static_cast<ClipInfo*>(item);
  QFileInfo info(clipInfo->getClipFileName());
  QMimeDatabase mimeDatabase;
  QMimeType mimeType;
  mimeType = mimeDatabase.mimeTypeForFile(info);
  QString str = "";
  QTextStream stream(&str);
  if (this->isMp4(info))
  {
    stream << "width          : " << "" << Qt::endl;
    stream << "height         : " << "" << Qt::endl;
    stream << "depth          : " << "" << Qt::endl;
    stream << "size           : " << info.size() << Qt::endl;
    stream << "file type      : " << mimeType.preferredSuffix() << Qt::endl;
    stream << "birth date     : " << info.birthTime().toString("yyyy/MM/dd hh:mm:ss.zzz") << Qt::endl;
    stream << "last modifiyed : " << info.lastModified().toString("yyyy/MM/dd hh:mm:ss.zzz") << Qt::endl;
    ui.labelInfomation->setText(str);
  }
  else
  {
    QImage img(clipInfo->getClipFileName());
    stream << "width          : " << img.width() << Qt::endl;
    stream << "height         : " << img.height() << Qt::endl;
    stream << "depth          : " << img.depth() << Qt::endl;
    stream << "size           : " << info.size() << Qt::endl;
    stream << "file type      : " << mimeType.preferredSuffix() << Qt::endl;
    stream << "birth date     : " << info.birthTime().toString("yyyy/MM/dd hh:mm:ss.zzz") << Qt::endl;
    stream << "last modifiyed : " << info.lastModified().toString("yyyy/MM/dd hh:mm:ss.zzz") << Qt::endl;
    ui.labelInfomation->setText(str);
  }
}
