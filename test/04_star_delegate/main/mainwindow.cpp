
#include "mainwindow.h"

#include "stardelegate.h"
#include "stareditor.h"
#include "starrating.h"

using namespace star_rate;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
  ui.setupUi(this);


  ui.tableWidget->setItemDelegate(new StarDelegate());
  ui.tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
  ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui.tableWidget->setHorizontalHeaderLabels({"Title", "Genre", "Artist", "Rating"});

  this->populateTableWidget();

  ui.tableWidget->resizeColumnsToContents();
  ui.tableWidget->resize(500, 300);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
}

void MainWindow::populateTableWidget()
{
  static constexpr struct {
    const char* title;
    const char* genre;
    const char* artist;
    int rating;
  } staticData[] = {
    { "Mass in B-Minor", "Baroque", "J.S. Bach", 5 },
    { "Three More Foxes", "Jazz", "Maynard Ferguson", 4 },
    { "Sex Bomb", "Pop", "Tom Jones", 3 },
    { "Barbie Girl", "Pop", "Aqua", 5 },
    { nullptr, nullptr, nullptr, 0 }
  };

  for (int row = 0; staticData[row].title != nullptr; row++)
  {
    QTableWidgetItem* item0 = new QTableWidgetItem(staticData[row].title);
    QTableWidgetItem* item1 = new QTableWidgetItem(staticData[row].genre);
    QTableWidgetItem* item2 = new QTableWidgetItem(staticData[row].artist);
    QTableWidgetItem* item3 = new QTableWidgetItem();
    item3->setData(0, QVariant::fromValue(StarRating(staticData[row].rating)));

    ui.tableWidget->setItem(row, 0, item0);
    ui.tableWidget->setItem(row, 1, item1);
    ui.tableWidget->setItem(row, 2, item2);
    ui.tableWidget->setItem(row, 3, item3);
  }
}