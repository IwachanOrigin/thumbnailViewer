
#include "breadcrumbsaddressbar.h"

#include <QSize>
#include <QStyleFactory>
#include <QApplication>

const QSize TRANSPARENT_ICON_SIZE = QSize(40, 40);

BreadCrumbsAddressBar::BreadCrumbsAddressBar(QWidget* parent, Qt::WindowFlags f)
  : QFrame(parent, f)
  , m_styleProxy(nullptr)
{
  m_styleProxy = new StyleProxy(QPixmap(":/icons/assets/arrow-right_128x128.png"), QStyleFactory::create(qApp->style()->objectName()));
}

BreadCrumbsAddressBar::~BreadCrumbsAddressBar()
{
  if (m_styleProxy)
  {
    delete m_styleProxy;
    m_styleProxy = nullptr;
  }
}
