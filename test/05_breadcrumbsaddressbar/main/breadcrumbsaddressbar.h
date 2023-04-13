
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
  StyleProxy* m_styleProxy;
  QLineEdit* m_lineAddress;

  void initCompleter(FilenameModel* model, QLineEdit* lineAddress);
  void eventConnect();
  void eventDisconnect();
};

#endif // BREAD_CRUMBS_ADDRESS_BAR_H_
