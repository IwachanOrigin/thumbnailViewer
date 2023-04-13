
#include "menulistview.h"

#include <QScrollBar>

constexpr int MAX_VISIBLE_ITEMS = 16;

MenuListView::MenuListView(QWidget* parent)
  : QMenu(parent)
  , m_mouseLeftPressed(false)
{
  m_listView.setFrameShape(m_listView.NoFrame);
  m_listView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_palette = m_listView.palette();
  m_palette.setColor(m_palette.Base, this->palette().color(m_palette.Window));
  m_listView.setPalette(m_palette);

  m_widgetAction = new QWidgetAction(this);
  m_widgetAction->setDefaultWidget(&m_listView);
  this->addAction(m_widgetAction);

  // QMenu -> QListView Signal - slot

  m_listView.setMouseTracking(true);
  m_listView.setFocusPolicy(Qt::NoFocus);
  m_listView.setFocus();
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
  int width = m_listView.sizeHintForColumn(0);
  width += m_listView.verticalScrollBar()->sizeHint().width();

  int visibleRows = std::min(MAX_VISIBLE_ITEMS, m_listView.model()->rowCount());
  return QSize(width, visibleRows * m_listView.sizeHintForRow(0));
}

void MenuListView::keyPressEvent(QKeyEvent* e)
{
  
}

void MenuListView::leaveEvent(QEvent* e)
{
  
}

void MenuListView::mouseMoveEvent(QMouseEvent* e)
{
  
}

void MenuListView::mousePressEvent(QMouseEvent* e)
{
  
}

void MenuListView::mouseReleaseEvent(QMouseEvent* e)
{
  
}
