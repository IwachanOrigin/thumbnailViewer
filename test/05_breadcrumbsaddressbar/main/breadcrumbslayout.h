
#ifndef BREAD_CRUMBS_LAYOUT_H_
#define BREAD_CRUMBS_LAYOUT_H_

#include <QHBoxLayout>
#include <QRect>
#include <QWidget>

class LeftHBoxLayout : public QHBoxLayout
{
  Q_OBJECT

  enum class CountType
  {
      ALL = 0
    , VISIBLE = 1
    , HIDDEN = 2
  };

public:
  explicit LeftHBoxLayout(QWidget* parent, const int firstVisible = 0, const float minimalSpace = 0.1);
  virtual ~LeftHBoxLayout();

  void setSpaceWidget(QWidget* widget = nullptr, const int stretch = 1);
  LeftHBoxLayout& getSpaceWidget();

  void setGeometry(QRect rc);
  int countVisible() const;
  int countHidden() const;
  QSize minimumSize() const;
  void addWidget(QWidget* widget, const int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

  float getMinimalSpace() { return m_minimalSpace; }

  QLayoutItem* getItem(int index);
  QLayoutItem* takeAt(const int index);

private:
  int m_firstVisible;
  float m_minimalSpace;

  int count(const CountType type = CountType::ALL) const;
  void setMinimalSpace(const float value);

signals:
  void signalWidgetStateChanged(QWidget* widget, bool state);
};

#endif // BREAD_CRUMBS_LAYOUT_H_
