
#include "breadcrumbsaddressbar.h"
#include "styleproxy.h"
#include "filenamemodel.h"
#include "breadcrumbslayout.h"

#include <QSize>
#include <QStyleFactory>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QMenu>
#include <QAbstractItemView>
#include <QPainter>
#include <QApplication>

const QSize TRANSPARENT_ICON_SIZE = QSize(40, 40);

BreadCrumbsAddressBar::BreadCrumbsAddressBar(QWidget* parent, Qt::WindowFlags f)
  : QFrame(parent, f)
  , m_styleProxy(nullptr)
  , m_layout(new QHBoxLayout(this))
  , m_iconProvider(new QFileIconProvider())
  , m_filenameModel(new FilenameModel(QStringList(), true))
  , m_mousePosTimer(new QTimer(this))
  , m_switchSpace(new QWidget(this))
  , m_btnBrowse(new QToolButton(this))
  , m_ignoreRaise(false)
  , m_path("")
  , m_completer(nullptr)
  , m_lineAddressIsContextMenu(false)
{
  m_styleProxy = new StyleProxy(QPixmap(":/icons/assets/arrow-right_128x128.png"), QStyleFactory::create(qApp->style()->objectName()));

  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, palette.color(QPalette::Base));
  this->setPalette(palette);
  this->setAutoFillBackground(true);
  this->setFrameShape(this->StyledPanel);
  this->layout()->setContentsMargins(4, 0, 0, 0);
  this->layout()->setSpacing(0);

  auto pathIcon = new QLabel(this);
  m_layout->addWidget(pathIcon);

  m_lineAddress = new QLineEdit(this);
  m_lineAddress->setFrame(false);
  m_lineAddress->hide();
  m_layout->addWidget(m_lineAddress);

  this->initCompleter(m_filenameModel, m_lineAddress);

  m_crumbsContainer = new QWidget(this);
  auto crumbsContainerLayout = new QHBoxLayout(m_crumbsContainer);
  crumbsContainerLayout->setContentsMargins(0, 0, 0, 0);
  crumbsContainerLayout->setSpacing(0);
  m_layout->addWidget(m_crumbsContainer);

  auto btnRootCrumb = new QToolButton(this);
  btnRootCrumb->setAutoRaise(true);
  btnRootCrumb->setPopupMode(QToolButton::InstantPopup);
  btnRootCrumb->setArrowType(Qt::RightArrow);
  btnRootCrumb->setStyleSheet("");
  btnRootCrumb->setMinimumSize(btnRootCrumb->minimumSizeHint());
  crumbsContainerLayout->addWidget(btnRootCrumb);

  m_menu = new QMenu(btnRootCrumb);
  btnRootCrumb->setMenu(m_menu);

  m_crumbsPanel = new QWidget(this);
  auto crumbsLayout = new LeftHBoxLayout(m_crumbsPanel);
  crumbsLayout->setContentsMargins(0, 0, 0, 0);
  crumbsLayout->setSpacing(0);
  crumbsContainerLayout->addWidget(m_crumbsPanel);

  crumbsLayout->setSpaceWidget(m_switchSpace);

  m_btnBrowse->setAutoRaise(true);
  m_btnBrowse->setText("...");
  m_btnBrowse->setToolTip("Browse for folder");
  m_layout->addWidget(m_btnBrowse);

  this->setMaximumHeight(m_lineAddress->height());
}

BreadCrumbsAddressBar::~BreadCrumbsAddressBar()
{
  if (m_styleProxy)
  {
    delete m_styleProxy;
    m_styleProxy = nullptr;
  }
}

void BreadCrumbsAddressBar::keyPressEvent(QKeyEvent* event)
{
  switch(event->key())
  {
  case Qt::Key_Escape:
  {
    this->cancelEdit();
  }
  break;

  case Qt::Key_Return:
  case Qt::Key_Enter:
  {
    // 
    this->showAddressField(false);
  }
  break;

  }
}

void BreadCrumbsAddressBar::focusOutEvent(QFocusEvent* event)
{
  if (m_lineAddressIsContextMenu)
  {
    m_lineAddressIsContextMenu = false;
    this->cancelEdit();
  }
}

void BreadCrumbsAddressBar::contextMenuEvent(QContextMenuEvent* event)
{
  m_lineAddressIsContextMenu = true;
}

void BreadCrumbsAddressBar::mouseReleaseEvent(QMouseEvent* event)
{
  
}

void BreadCrumbsAddressBar::initCompleter(FilenameModel* model, QLineEdit* lineAddress)
{
  m_completer = new QCompleter(lineAddress);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_completer->setModel(model);
  lineAddress->setCompleter(m_completer);
}

void BreadCrumbsAddressBar::cancelEdit()
{
  m_lineAddress->setText(m_path);
  this->showAddressField(false);
}

void BreadCrumbsAddressBar::showAddressField(const bool show)
{
  if (show)
  {
    m_crumbsContainer->hide();
    m_lineAddress->show();
    m_lineAddress->setFocus();
    m_lineAddress->selectAll();
  }
  else
  {
    m_lineAddress->hide();
    m_crumbsContainer->show();
  }
}

void BreadCrumbsAddressBar::hiddenCrumbsMenuShow()
{
  if (!m_mousePosTimer)
  {
    return;
  }
  m_mousePosTimer->start(100);
  // QObject::sender is 
  auto menu = qobject_cast<QMenu*>(this->sender());
  if (!m_actionsHiddenCrumbs.isEmpty())
  {
    for (auto i : m_actionsHiddenCrumbs)
    {
      menu->removeAction(i);
    }
    m_actionsHiddenCrumbs.clear();
  }

  auto firstAction = menu->actions()[0];
  
}

void BreadCrumbsAddressBar::setPath()
{
  
}

void BreadCrumbsAddressBar::eventConnect()
{
  QObject::connect(m_lineAddress, &QLineEdit::textEdited, m_filenameModel, &FilenameModel::setPathPrefix);
  
}

void BreadCrumbsAddressBar::eventDisconnect()
{
  
}

QIcon BreadCrumbsAddressBar::getIcon(const QString path)
{
  QFileInfo fileinfo(path);
  auto icon = m_iconProvider->icon(fileinfo);
  if (fileinfo.isHidden())
  {
    auto pixmap = new QPixmap(TRANSPARENT_ICON_SIZE);
    pixmap->fill(Qt::transparent);
    auto painter = new QPainter(pixmap);
    painter->setOpacity(0.5);
    //icon.paint(painter, 0, 0, TRANSPARENT_ICON_SIZE);
    
  }
}
