
#ifndef BREAD_CRUMBS_ADDRESS_BAR_H_
#define BREAD_CRUMBS_ADDRESS_BAR_H_

#include <QFrame>
#include <QWidget>
#include <QString>
#include <QHBoxLayout>
#include <QFileIconProvider>
#include <QTimer>
#include <QToolButton>

class StyleProxy;
class FilenameModel;

class BreadCrumbsAddressBar : public QFrame
{
public:
  explicit BreadCrumbsAddressBar(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~BreadCrumbsAddressBar();

private:
  StyleProxy* m_styleProxy;
  QHBoxLayout* m_layout;
  QFileIconProvider* m_iconProvider;
  FilenameModel* m_filenameModel;
  QTimer* m_mousePosTimer;
  QWidget* m_switchSpace;
  QToolButton* m_btnBrowse;
  bool m_ignoreRaise;
  QString m_path;
};

#endif // BREAD_CRUMBS_ADDRESS_BAR_H_
