
#ifndef STYLE_PROXY_H_
#define STYLE_PROXY_H_

#include <QProxyStyle>
#include <QString>
#include <QStringList>
#include <QPixmap>

class StyleProxy : public QProxyStyle
{
public:
  explicit StyleProxy(const QPixmap& pixmap, QStyle* style = nullptr);
  virtual ~StyleProxy();

  void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
  QRect	subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex* option, QStyle::SubControl sc, const QWidget* widget) const override;

private:
  QPixmap m_arrowPixmap;
  QString m_styleName;
  QStringList m_winModern;
};

#endif // STYLE_PROXY_H_
