
#include "clipinfodelegate.h"

ClipInfoDelegate::ClipInfoDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_width(120)
    , m_height(120)
{
}

void ClipInfoDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QRect r = option.rect;

	if (option.state & QStyle::State_Selected)
    {
		painter->setBrush(QColor::fromRgb(154, 154, 54));
		painter->drawRect(r);
	}
	else
    {
		painter->setBrush(QColor::fromRgb(32, 32, 32));
		painter->drawRect(r);
	}

	QIcon icon = QIcon(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
	QString name = index.data(Qt::DisplayRole).toString();
	QString uuid = index.data(Qt::UserRole + 0).toString();

	const int space = 5;
	int iw = 0;
	int ih = 0;
	if (!icon.isNull())
    {
		r = option.rect.adjusted(space, space, -space, -space);
		icon.paint(painter, r, Qt::AlignVCenter | Qt::AlignLeft);
		iw = space + icon.actualSize(QSize(100, 64)).width() + space;
		ih = space + icon.actualSize(QSize(100, 64)).height() + space;
	}

	QPen fontPen(QColor::fromRgb(200, 200, 200), 1, Qt::SolidLine);
	painter->setPen(fontPen);
	painter->setFont(QFont("Lucida Grande", 11, QFont::Normal));

	r = option.rect.adjusted(space + iw, space + ((m_height / 3) * 1), -space, -((m_height / 3) * 1));
	painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignLeft | Qt::AlignVCenter, name, &r);
}

QSize ClipInfoDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    return QSize(m_width, m_height);
}

