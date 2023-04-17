
#include "styleproxy.h"
#include <QStyleOptionToolButton>
#include <QStyleOption>
#include <QPainter>

StyleProxy::StyleProxy(const QPixmap& pixmap, QStyle* style)
  : QProxyStyle(style)
  , m_arrowPixmap(pixmap)
  , m_styleName(this->baseStyle()->objectName())
  , m_winModen({"windowsxp", "windowsvista"})
{
}

StyleProxy::~StyleProxy()
{
}

void StyleProxy::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
  for (auto stylename : m_winModen)
  {
    if (m_styleName == stylename && element == this->PE_IndicatorButtonDropDown)
    {
      element = this->PE_IndicatorArrowDown;
      break;
    }
  }

  if (element == this->PE_IndicatorArrowDown)
  {
    QStyleOptionComplex opt = QStyleOptionToolButton();
    QRect rect = QProxyStyle::subControlRect(this->CC_ToolButton, &opt, this->SC_ToolButtonMenu, widget);
    rect.moveTop(int((rect.height() - rect.width())) / 2);
    rect.setHeight(rect.width());
    painter->drawPixmap(rect, m_arrowPixmap, QRect());
  }
  else
  {
    QProxyStyle::drawPrimitive(element, option, painter, widget);
  }
}

QRect	StyleProxy::subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex* option, QStyle::SubControl sc, const QWidget* widget) const
{
  QRect rect = QProxyStyle::subControlRect(cc, option, sc, widget);
  for (auto stylename : m_winModen)
  {
    if (m_styleName == stylename && sc == this->SC_ToolButtonMenu)
    {
      rect.adjust(-2, 0, 0, 0);
    }
  }
  return rect;
}
