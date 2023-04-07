
#include "stareditor.h"
#include "starrating.h"

#include <QtWidgets>

namespace sr = star_rate;

sr::StarEditor::StarEditor(QWidget* parent)
  : QWidget(parent)
{
  this->setMouseTracking(true);
  this->setAutoFillBackground(true);
}

QSize sr::StarEditor::sizeHint() const
{
  return m_myStarRating.sizeHint();
}

void sr::StarEditor::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  m_myStarRating.paint(&painter, this->rect(), this->palette(), StarRating::EditMode::Editable);
}

void sr::StarEditor::mouseMoveEvent(QMouseEvent* event)
{
  const int star = starAtPosition(event->x());
  if (star != m_myStarRating.starCount() && star != -1)
  {
    m_myStarRating.setStarCount(star);
    this->update();
  }
  QWidget::mouseMoveEvent(event);
}

void sr::StarEditor::mouseReleaseEvent(QMouseEvent* event)
{
  emit editingFinished();
  QWidget::mouseReleaseEvent(event);
}

int sr::StarEditor::starAtPosition(int x) const
{
  const int star = (x / (m_myStarRating.sizeHint().width() / m_myStarRating.maxStarCount())) + 1;
  if (star <= 0 || star > m_myStarRating.maxStarCount())
  {
    return -1;
  }
  return star;
}
