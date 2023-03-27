
#include "mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , m_init(false)
{
  ui.setupUi(this);

  // init api
  m_init = m_ot.createAPI();

  QObject::connect(ui.pushButtonSelectInputFile, &QPushButton::pressed, this, &MainWindow::slotPushButtonSelectInputFile);
  QObject::connect(ui.pushButtonSelectOutputDir, &QPushButton::pressed, this, &MainWindow::slotPushButtonSelectOutputDir);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
  if (m_init)
  {
    m_ot.destroyAPI();
  }
}

void MainWindow::slotPushButtonSelectInputFile()
{
  QString filename = "";
  filename = QFileDialog::getOpenFileName(this, tr("Open Movie"), "C:\\", tr("Movie Files (*.mp4)"));
  ui.labelInputFilePath->setText(filename);
}

void MainWindow::slotPushButtonSelectOutputDir()
{
  QString dirname = "";
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::Directory);
  fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
  if (fileDialog.exec())
  {
    QStringList filePaths = fileDialog.selectedFiles();
    ui.labelOutputFilePath->setText(filePaths.at(0));
  }
}