
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

  painter->setRenderHint(QPainter::antialiasing, true);
}

QSize sr::StarRating::sizeHint() const
{
  return PAINTING_SCALE_FACTOR * QSize(m_myMaxStarCount, 1);
}

