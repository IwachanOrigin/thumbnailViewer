
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

  auto lineAddress = new QLineEdit(this);
  lineAddress->setFrame(false);
  lineAddress->hide();
  m_layout->addWidget(lineAddress);

  //auto completer =

  auto crumbsContainer = new QWidget(this);
  auto crumbsContainerLayout = new QHBoxLayout(crumbsContainer);
  crumbsContainerLayout->setContentsMargins(0, 0, 0, 0);
  crumbsContainerLayout->setSpacing(0);
  m_layout->addWidget(crumbsContainer);

  auto btnRootCrumb = new QToolButton(this);
  btnRootCrumb->setAutoRaise(true);
  btnRootCrumb->setPopupMode(QToolButton::InstantPopup);
  btnRootCrumb->setArrowType(Qt::RightArrow);
  btnRootCrumb->setStyleSheet("");
  btnRootCrumb->setMinimumSize(btnRootCrumb->minimumSizeHint());
  crumbsContainerLayout->addWidget(btnRootCrumb);

  auto menu = new QMenu(btnRootCrumb);
  btnRootCrumb->setMenu(menu);

  auto crumbsPanel = new QWidget(this);
  auto crumbsLayout = new LeftHBoxLayout(crumbsPanel);
  crumbsLayout->setContentsMargins(0, 0, 0, 0);
  crumbsLayout->setSpacing(0);
  crumbsContainerLayout->addWidget(crumbsPanel);

  crumbsLayout->setSpaceWidget(m_switchSpace);

  m_btnBrowse->setAutoRaise(true);
  m_btnBrowse->setText("...");
  m_btnBrowse->setToolTip("Browse for folder");
  m_layout->addWidget(m_btnBrowse);

  this->setMaximumHeight(lineAddress->height());
}

BreadCrumbsAddressBar::~BreadCrumbsAddressBar()
{
  if (m_styleProxy)
  {
    delete m_styleProxy;
    m_styleProxy = nullptr;
  }
}
