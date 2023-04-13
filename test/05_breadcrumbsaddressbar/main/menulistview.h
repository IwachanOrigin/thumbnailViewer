
#ifndef MENU_LIST_VIEW_H_
#define MENU_LIST_VIEW_H_

#include <QListView>
#include <QMenu>
#include <QPalette>
#include <QWidgetAction>
#include <QModelIndex>
#include <QPoint>

class MenuListView : public QMenu
{
  Q_OBJECT

public:
  explicit MenuListView(QWidget* parent = nullptr);
  virtual ~MenuListView();

  QSize	sizeHint() const override;

protected:
  void keyPressEvent(QKeyEvent* e) override;
  void leaveEvent(QEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;

private:
  QListView m_listView;
  QPalette m_palette;
  QWidgetAction* m_widgetAction;
  bool m_mouseLeftPressed;
  QModelIndex m_lastIndex;

  void updateCurrentIndex(const QPoint& point);
};

#endif // MENU_LIST_VIEW_H_
