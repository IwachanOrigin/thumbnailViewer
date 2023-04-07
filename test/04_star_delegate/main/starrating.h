
#ifndef STAR_RATING_H_
#define STAR_RATING_H_

#include <QPainter>
#include <QPolygonF>
#include <QSize>

namespace star_rate
{

class StarRating
{
public:
  enum class EditMode
  {
    Editable = 0
    , ReadOnly
  };

  explicit StarRating(int starCount = 1, int maxStarCount = 5);
  ~StarRating() = default;

  void paint(QPainter* painter, const QRect& rect, const QPalette& palette, EditMode mode) const;
  QSize sizeHint() const;

  int starCount() const { return m_myStarCount; }
  int maxStarCount() const { return m_myMaxStarCount; }
  void setStarCount(const int starCount) { m_myStarCount = starCount; }
  void setMaxStarCount(const int maxStarCount) { m_myMaxStarCount = maxStarCount; }

private:
  int m_myStarCount;
  int m_myMaxStarCount;

  QPolygonF m_starPolygon;
  QPolygonF m_diamondPolygon;
};

Q_DECLARE_METATYPE(StarRating);

} // star_rate

#endif // STAR_RATING_H_
