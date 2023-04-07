
#ifndef STAR_EDITOR_H_
#define STAR_EDITOR_H_

#include <QWidget>
#include "starrating.h"

namespace star_rate
{

class StarEditor : public QWidget
{
  Q_OBJECT

public:
  explicit StarEditor(QWidget* parent = nullptr);
  ~StarEditor() = default;

  QSize sizeHint() const override;
  void setStarRating(const StarRating& starRating)
  {
    m_myStarRating = starRating; 
  }
  StarRating starRating() { return m_myStarRating; }

signals:
  void editingFinished();
  
protected:
  void paintEvent(QPaintEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  int starAtPosition(int x ) const;

  StarRating m_myStarRating;
};

} // star_rate

#endif // STAR_EDITOR_H_
