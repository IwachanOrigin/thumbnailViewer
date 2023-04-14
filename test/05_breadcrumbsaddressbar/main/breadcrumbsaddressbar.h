
#ifndef BREAD_CRUMBS_ADDRESS_BAR_H_
#define BREAD_CRUMBS_ADDRESS_BAR_H_

#include <QAbstractItemModel>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QFileIconProvider>
#include <QFocusEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QString>
#include <QTimer>
#include <QToolButton>
#include <QWidget>
#include <QMenu>
#include <QVector>
#include <QAction>
#include <QIcon>

class StyleProxy;
class FilenameModel;

class BreadCrumbsAddressBar : public QFrame
{
public:
  explicit BreadCrumbsAddressBar(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~BreadCrumbsAddressBar();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  bool m_ignoreRaise;
  FilenameModel* m_filenameModel;
  QCompleter* m_completer;
  QFileIconProvider* m_iconProvider;
  QHBoxLayout* m_layout;
  QString m_path;
  QTimer* m_mousePosTimer;
  QToolButton* m_btnBrowse;
  QWidget* m_switchSpace;
  QWidget* m_crumbsContainer;
  QWidget* m_crumbsPanel;
  StyleProxy* m_styleProxy;
  QLineEdit* m_lineAddress;
  QMenu* m_menu;
  bool m_lineAddressIsContextMenu;
  QVector<QAction*> m_actionsHiddenCrumbs;

  void initCompleter(FilenameModel* model, QLineEdit* lineAddress);
  void cancelEdit();
  void showAddressField(const bool show);
  void eventConnect();
  void eventDisconnect();
  void setPath();
  QIcon getIcon(const QString path);

private slots:
  void hiddenCrumbsMenuShow();
};

#endif // BREAD_CRUMBS_ADDRESS_BAR_H_
