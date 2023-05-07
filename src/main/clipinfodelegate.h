
#include <QPainter>
#include <QAbstractItemDelegate>

class ClipInfoDelegate : public QAbstractItemDelegate
{
  Q_OBJECT

public:
  explicit ClipInfoDelegate(QObject *parent = nullptr);
  virtual ~ClipInfoDelegate() = default;

  void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
  QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private:
  const int m_width;
  const int m_height;
};

