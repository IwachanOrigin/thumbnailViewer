
#include "breadcrumbsaddressbar.h"
#include "styleproxy.h"
#include "filenamemodel.h"
#include "menulistview.h"
#include "breadcrumbslayout.h"

#include <QSize>
#include <QSizePolicy>
#include <QStyleFactory>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QMenu>
#include <QAbstractItemView>
#include <QPainter>
#include <QStandardPaths>
#include <QDir>
#include <QToolButton>
#include <QApplication>

#include <filesystem>

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

  m_pathIcon = new QLabel(this);
  m_layout->addWidget(m_pathIcon);

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
  if (event->button() != Qt::LeftButton)
  {
    return;
  }
  this->showAddressField(true);
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

void BreadCrumbsAddressBar::initRootMenuPlaces(QMenu* menu)
{
  if (menu == nullptr)
  {
    return;
  }

  menu->addSeparator();
  QVector<QPair<QString, QString>> mapStandardPaths = {
    {"Desktop", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)}
    , {"Home", QStandardPaths::writableLocation(QStandardPaths::HomeLocation)}
    , {"Documents", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)}
    , {"Downloads", QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)}
  };

  for (auto item : mapStandardPaths)
  {
    auto action = menu->addAction(this->getIcon(item.second), item.first);
    action->setData(item.second);
    QObject::connect(action, &QAction::trigger, this, &BreadCrumbsAddressBar::setPath);
  }
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
  // QObject::sender retrieves the object that issued the signal immediately before.
  auto menu = qobject_cast<QMenu*>(this->sender());
  if (!menu)
  {
    return;
  }

  if (!m_actionsHiddenCrumbs.isEmpty())
  {
    for (auto i : m_actionsHiddenCrumbs)
    {
      menu->removeAction(i);
    }
    m_actionsHiddenCrumbs.clear();
  }

  auto actions = menu->actions();
  auto firstAction = actions.at(0);
  for (int i = 0; i < m_crumbsPanel->layout()->count(); i++)
  {
    auto item = m_crumbsPanel->layout()->itemAt(i);
    // Is this item a widget? -> QToolButton
    if (auto widget = item->widget())
    {
      auto btn = qobject_cast<QToolButton*>(widget);
      if (btn)
      {
        if (btn->isHidden())
        {
          QVariant var = btn->property("path");
          QString _path = var.toString();
          if (!_path.isEmpty())
          {
            auto action = new QAction(this->getIcon(_path), btn->text(), menu);
            action->setData(_path);
            QObject::connect(action, &QAction::triggered, this, &BreadCrumbsAddressBar::setPath);
            menu->insertAction(firstAction, action);
            m_actionsHiddenCrumbs.append(action);
            firstAction = action;
          }
        }
      }
    }
  }
}

bool BreadCrumbsAddressBar::setPath(const QString& path)
{
  // Convert to the object that skipped the signal immediately before and obtain the file path from the data contained in the result.
  auto action = qobject_cast<QAction*>(this->sender());
  QString _path = path;
  if (_path.isEmpty() && action)
  {
    _path = action->data().toString();
  }

  std::filesystem::path fsPath(_path.toStdString());
  bool emitErr = false;

  try
  {
    fsPath = fsPath.lexically_normal();
  }
  catch(const std::filesystem::filesystem_error&)
  {
    emitErr = true;
    emit signalListDirError(_path);
  }

  if (!std::filesystem::exists(fsPath))
  {
    emitErr = true;
    emit signalPathError(_path);
  }

  this->cancelEdit();

  if (emitErr)
  {
    return false;
  }

  m_path = _path;
  m_lineAddress->setText(_path);
  this->insertCrumbs(_path);
  for (auto it = fsPath.begin(); it != fsPath.end(); it++)
  {
    if (*it == std::filesystem::current_path())
    {
      break;
    }
    this->insertCrumbs(QString::fromStdString((*it).string()));
  }
  m_pathIcon->setPixmap(this->getIcon(m_path).pixmap(16, 16));
  emit signalPathSelected(m_path);

  return true;
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
    icon.paint(painter, 0, 0, TRANSPARENT_ICON_SIZE.width(), TRANSPARENT_ICON_SIZE.height());
    painter->end();
    icon = QIcon(*pixmap);
  }
  return icon;
}

void BreadCrumbsAddressBar::clearCrumbs()
{
  auto layout = m_crumbsPanel->layout();
  for (int i = 0; i < layout->count(); i++)
  {
    auto widget = layout->takeAt(0)->widget();
    if (widget)
    {
      widget->setStyle(nullptr);
      widget->deleteLater();
    }
  }
}

void BreadCrumbsAddressBar::insertCrumbs(const QString& path)
{
  auto btn = new QToolButton(m_crumbsPanel);
  btn->setAutoRepeat(true);
  btn->setPopupMode(QToolButton::MenuButtonPopup);
  // btn->setStyle();
  btn->setMouseTracking(true);
  btn->setText("");
  btn->setProperty("path", path);
  QObject::connect(btn, &QToolButton::clicked, this, &BreadCrumbsAddressBar::slotCrumbClicked);

  auto menu = new MenuListView(btn);
  QObject::connect(menu, &MenuListView::aboutToShow, this, &BreadCrumbsAddressBar::slotCrumbMenuShow);
  menu->setModel(m_filenameModel);
  QObject::connect(menu, &MenuListView::clicked, this, &BreadCrumbsAddressBar::slotCrumbMenuItemClicked);
  QObject::connect(menu, &MenuListView::activated, this, &BreadCrumbsAddressBar::slotCrumbMenuItemClicked);
  QObject::connect(menu, &MenuListView::aboutToHide, m_mousePosTimer, &QTimer::stop);
  btn->setMenu(menu);
  auto retrievedLayout = m_crumbsPanel->layout();
  QHBoxLayout* boxlayout = dynamic_cast<QHBoxLayout*>(retrievedLayout);
  if (boxlayout)
  {
    boxlayout->insertWidget(0, btn);
  }
  else
  {
    retrievedLayout->addWidget(btn);
  }
  btn->setMinimumSize(btn->minimumSizeHint());
  auto sp = btn->sizePolicy();
  sp.setVerticalPolicy(sp.Minimum);
  btn->setSizePolicy(sp);
}

void BreadCrumbsAddressBar::slotCrumbClicked()
{
  auto action = qobject_cast<QAction*>(this->sender());
  QString _path = "";
  if (action)
  {
    _path = action->data().toString();
  }
  this->setPath(_path);
}

void BreadCrumbsAddressBar::slotCrumbMenuShow()
{
  auto menu = qobject_cast<QMenu*>(this->sender());
  m_filenameModel->setPathPrefix(menu->parent()->property("path").toString());
  // menu.clearsection
  m_mousePosTimer->start(100);
}

void BreadCrumbsAddressBar::slotCrumbMenuItemClicked(const QModelIndex& index)
{
  this->setPath(index.data(Qt::EditRole).toString());
}
