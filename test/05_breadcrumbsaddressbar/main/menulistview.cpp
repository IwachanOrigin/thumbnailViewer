
#include "menulistview.h"

#include <QScrollBar>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QAbstractItemModel>

constexpr int MAX_VISIBLE_ITEMS = 16;

MenuListView::MenuListView(QWidget* parent)
  : QMenu(parent)
  , m_mouseLeftPressed(false)
{
  m_listView = new QListView(this);
  m_listView->setFrameShape(QFrame::NoFrame);
  m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_palette = m_listView->palette();
  m_palette.setColor(m_palette.Base, this->palette().color(m_palette.Window));
  m_listView->setPalette(m_palette);

  m_widgetAction = new QWidgetAction(this);
  m_widgetAction->setDefaultWidget(m_listView);
  this->addAction(m_widgetAction);

  m_listView->setMouseTracking(true);
  m_listView->setFocusPolicy(Qt::NoFocus);
  m_listView->setFocus();

  QObject::connect(m_listView, &QListView::activated, this, &MenuListView::activated);
  QObject::connect(m_listView, &QListView::clicked, this, &MenuListView::clicked);
}

MenuListView::~MenuListView()
{
  if (m_widgetAction)
  {
    delete m_widgetAction;
    m_widgetAction = nullptr;
  }
}

QSize	MenuListView::sizeHint() const
{
  int width = m_listView->sizeHintForColumn(0);
  width += m_listView->verticalScrollBar()->sizeHint().width();

  int visibleRows = std::min(MAX_VISIBLE_ITEMS, m_listView->model()->rowCount());
  return QSize(width, visibleRows * m_listView->sizeHintForRow(0));
}

void MenuListView::setModel(QAbstractItemModel* model)
{
  m_listView->setModel(model);
}

void MenuListView::keyPressEvent(QKeyEvent* e)
{
  int key = e->key();
  switch (key)
  {
  case Qt::Key_Return:
  case Qt::Key_Enter:
  {
    if (m_lastIndex.isValid())
    {
      emit activated(m_lastIndex);
    }
    this->close();
  }
  break;

  case Qt::Key_Escape:
  {
    this->close();
  }
  break;

  case Qt::Key_Down:
  case Qt::Key_Up:
  {
    auto model = m_listView->model();
    int rowFrom = 0;
    int rowTo = model->rowCount() - 1;
    if (key == Qt::Key_Down)
    {
      int wk = rowFrom;
      rowFrom = rowTo;
      rowTo = wk;
    }

    QModelIndex index;
    bool findCheck = false;
    for (int i = -1; i <= rowFrom; i++)
    {
      if (i == m_lastIndex.row())
      {
        index = model->index(rowTo, 0);
        findCheck = true;
        break;
      }
    }

    if (!findCheck)
    {
      int shift = 1;
      if (key != Qt::Key_Down)
      {
        shift = -1;
      }
      index = model->index(m_lastIndex.row() + shift, 0);
    }

    m_listView->setCurrentIndex(index);
    m_lastIndex = index;
  }
  break;

  }
}

void MenuListView::leaveEvent(QEvent* e)
{
  m_listView->clearSelection();
  m_listView->setCurrentIndex(QModelIndex());
  m_lastIndex = QModelIndex();
}

void MenuListView::mouseMoveEvent(QMouseEvent* e)
{
  this->updateCurrentIndex(e->pos());
}

void MenuListView::mousePressEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton)
  {
    m_mouseLeftPressed = true;
    this->updateCurrentIndex(e->pos());
  }
}

void MenuListView::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() == Qt::LeftButton && m_mouseLeftPressed)
  {
    m_mouseLeftPressed = false;
    if (m_lastIndex.isValid())
    {
      emit clicked(m_lastIndex);
    }
    this->close();
  }
}

void MenuListView::updateCurrentIndex(const QPoint& point)
{
  m_lastIndex = m_listView->indexAt(point);
  m_listView->setCurrentIndex(m_lastIndex);
}
