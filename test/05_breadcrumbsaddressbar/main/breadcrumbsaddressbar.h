
#ifndef BREAD_CRUMBS_ADDRESS_BAR_H_
#define BREAD_CRUMBS_ADDRESS_BAR_H_

#include <QFrame>
#include "styleproxy.h"

class BreadCrumbsAddressBar : public QFrame
{
public:
  explicit BreadCrumbsAddressBar(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~BreadCrumbsAddressBar();

private:
  StyleProxy* m_styleProxy;
};

#endif // BREAD_CRUMBS_ADDRESS_BAR_H_
