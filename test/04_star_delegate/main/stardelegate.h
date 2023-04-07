
#ifndef STAR_DELEGATE_H_
#define STAR_DELEGATE_H_

#include <QStyledItemDelegate>

namespace star_rate
{

class StarDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private slots:
  void commitAndCloseEditor();
};

} // star_rate

#endif // STAR_DELEGATE_H_
