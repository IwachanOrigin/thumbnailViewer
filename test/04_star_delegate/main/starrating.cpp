
#include "starrating.h"

#include <QtWidgets>
#include <cmath>

namespace sr = star_rate;

constexpr int PAINTING_SCALE_FACTOR = 20;

sr::StarRating::StarRating(int starCount, int maxStarCount)
  : m_myStarCount(starCount)
  , m_myMaxStarCount(maxStarCount)
{
  m_starPolygon << QPointF(1.0f, 0.5f);
  for (int i = 1; i < m_myMaxStarCount; i++)
  {
    m_starPolygon << QPointF(0.5f + 0.5f * std::cos(0.8f * i * 3.14)
                             , 0.5f + 0.5f * std::sin(0.8f * i * 3.14));
  }
  m_diamondPolygon << QPointF(0.4f, 0.5f) << QPointF(0.5f, 0.4f)
                   << QPointF(0.6f, 0.5f) << QPointF(0.5f, 0.6f)
                   << QPointF(0.4f, 0.5f);
}

void  sr::StarRating::paint(QPainter* painter, const QRect& rect, const QPalette& palette, EditMode mode) const
{
  painter->save();

  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setPen(Qt::NoPen);
  painter->setBrush(mode == EditMode::Editable ? palette.highlight() : palette.windowText());
  const int yOffset = (rect.height() - PAINTING_SCALE_FACTOR) / 2;
  painter->translate(rect.x(), rect.y() + yOffset);
  painter->scale(PAINTING_SCALE_FACTOR, PAINTING_SCALE_FACTOR);

  for (int i = 0; i < m_myMaxStarCount; i++)
  {
    if (i < m_myStarCount)
    {
      painter->drawPolygon(m_starPolygon, Qt::WindingFill);
    }
    else if (mode == EditMode::Editable)
    {
      painter->drawPolygon(m_diamondPolygon, Qt::WindingFill);
    }
    painter->translate(1.0f, 0.0f);
  }
  painter->restore();
}

QSize sr::StarRating::sizeHint() const
{
  return PAINTING_SCALE_FACTOR * QSize(m_myMaxStarCount, 1);
}

