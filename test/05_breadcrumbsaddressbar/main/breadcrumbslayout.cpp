
#include "breadcrumbslayout.h"
#include <QHBoxLayout>
#include <QWidget>
#include <QTimer>

LeftHBoxLayout::LeftHBoxLayout(QWidget* parent, const int firstVisible, const float minimalSpace)
  : QHBoxLayout(parent)
  , m_firstVisible(firstVisible)
  , m_minimalSpace(minimalSpace)
{
  this->setSpaceWidget();
  this->setMinimalSpace(minimalSpace);
}

LeftHBoxLayout::~LeftHBoxLayout()
{
}

void LeftHBoxLayout::setSpaceWidget(QWidget* widget, const int stretch)
{
  QHBoxLayout::takeAt(this->count());
  if (widget)
  {
    QHBoxLayout::addWidget(widget);
  }
  else
  {
    addStretch(stretch);
  }
}

LeftHBoxLayout& LeftHBoxLayout::getSpaceWidget()
{
  return this[this->count()];
}

void LeftHBoxLayout::setGeometry(QRect rc)
{
  QHBoxLayout::setGeometry(rc);
  float minSpace = this->getMinimalSpace();
  if (minSpace < 1)
  {
    minSpace *= rc.width();
  }

  float freeSpace = this[this->count()].geometry().width() - minSpace;
  if (freeSpace < 0 && this->countVisible() > 1)
  {
    auto widget = this[m_firstVisible].widget();
    widget->hide();
    m_firstVisible += 1;
    emit signalWidgetStateChanged(widget, false);
  }
  else if (freeSpace > 0 && this->countHidden())
  {
    auto widget = this[m_firstVisible - 1].widget();
    float w_width = widget->width() + this->spacing();
    if (w_width <= freeSpace)
    {
      QTimer::singleShot(0, this, SLOT(widget->show()));
      m_firstVisible -= 1;
      emit signalWidgetStateChanged(widget, true);
    }
  }
}

int LeftHBoxLayout::countVisible() const
{
  return this->count(CountType::VISIBLE);
}

int LeftHBoxLayout::countHidden() const
{
  return this->count(CountType::HIDDEN);
}

QSize LeftHBoxLayout::minimumSize() const
{
  QMargins margins = this->contentsMargins();
  return QSize(margins.left() + margins.right(), margins.top() + 24 + margins.bottom());
}

void LeftHBoxLayout::addWidget(QWidget* widget, const int stretch, Qt::Alignment alignment)
{
  QHBoxLayout::insertWidget(this->count(), widget, stretch, alignment);
}

int LeftHBoxLayout::count(const CountType type) const
{
  int count = QHBoxLayout::count() - 1;
  if (type == CountType::ALL)
  {
    return count;
  }

  if (type == CountType::VISIBLE)
  {
    return count - m_firstVisible;
  }

  // Hidden items
  return m_firstVisible;
}

void LeftHBoxLayout::setMinimalSpace(const float value)
{
  m_minimalSpace = value;
  this->invalidate();
}

QLayoutItem* LeftHBoxLayout::getItem(int index)
{
  if (index < 0)
  {
    index = this->count() + index;
  }
  return this->itemAt(index);
}

QLayoutItem* LeftHBoxLayout::takeAt(const int index)
{
  if (index < m_firstVisible)
  {
    m_firstVisible -= 1;
  }

  auto item = QHBoxLayout::takeAt(index);
  emit signalWidgetStateChanged(item->widget(), false);
  return item;
}
