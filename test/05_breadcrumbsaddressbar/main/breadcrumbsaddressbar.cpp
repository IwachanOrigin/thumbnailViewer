
#include <Windows.h>
#include <combaseapi.h>
#include <ShlObj_core.h>

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
#include <QFileDialog>
#include <QToolButton>
#include <QStorageInfo>
#include <QCompleter>
#include <QApplication>
#include <QDebug>

#include <filesystem>

const QSize TRANSPARENT_ICON_SIZE = QSize(40, 40);
const QString CWD_PATH = QString::fromStdString(std::filesystem::current_path().string());

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
  QObject::connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated), this, &BreadCrumbsAddressBar::setPath);

  m_crumbsContainer = new QWidget(this);
  auto crumbsContainerLayout = new QHBoxLayout(m_crumbsContainer);
  crumbsContainerLayout->setContentsMargins(0, 0, 0, 0);
  crumbsContainerLayout->setSpacing(0);
  m_layout->addWidget(m_crumbsContainer);

  m_btnRootCrumb = new QToolButton(this);
  m_btnRootCrumb->setAutoRaise(true);
  m_btnRootCrumb->setPopupMode(QToolButton::InstantPopup);
  m_btnRootCrumb->setArrowType(Qt::RightArrow);
  m_btnRootCrumb->setStyleSheet("");
  m_btnRootCrumb->setMinimumSize(m_btnRootCrumb->minimumSizeHint());
  crumbsContainerLayout->addWidget(m_btnRootCrumb);

  m_menu = new QMenu(m_btnRootCrumb);
  m_btnRootCrumb->setMenu(m_menu);
  this->initRootMenuPlaces(m_menu);
  this->updateRootMenuDevices();

  m_crumbsPanel = new QWidget(this);
  auto crumbsLayout = new LeftHBoxLayout(m_crumbsPanel);
  QObject::connect(crumbsLayout, &LeftHBoxLayout::signalWidgetStateChanged, this, &BreadCrumbsAddressBar::slotCrumbHideShow);
  crumbsLayout->setContentsMargins(0, 0, 0, 0);
  crumbsLayout->setSpacing(0);
  crumbsContainerLayout->addWidget(m_crumbsPanel);

  crumbsLayout->setSpaceWidget(m_switchSpace);

  m_btnBrowse->setAutoRaise(true);
  m_btnBrowse->setText("...");
  m_btnBrowse->setToolTip("Browse for folder");
  QObject::connect(m_btnBrowse, &QToolButton::clicked, this, &BreadCrumbsAddressBar::slotBrowseForFolder);
  m_layout->addWidget(m_btnBrowse);

  this->setMaximumHeight(m_lineAddress->height());

  this->setPath(CWD_PATH);
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
    this->setPath(m_lineAddress->text());
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
  auto popup = qobject_cast<QListView*>(m_completer->popup());
  if (popup)
  {
    popup->setUniformItemSizes(true);
    popup->setLayoutMode(QListView::Batched);
  }
  lineAddress->setCompleter(m_completer);
  QObject::connect(lineAddress, &QLineEdit::textEdited, model, &FilenameModel::setPathPrefix);
  
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
    auto name = item.second;
    name = this->getPathLabel(name.replace("/", "\\"));
    auto action = menu->addAction(this->getIcon(item.second), name);
    action->setData(item.second);
    QObject::connect(action, &QAction::triggered, this, [this, action]() {
      this->setPath(action->data().toString());
      });
  }
}

std::vector<std::pair<QString, QString>> BreadCrumbsAddressBar::listNetworkLocations()
{
  std::vector<std::pair<QString, QString>> result;

  QString userFolder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
  QString networkShortcuts = userFolder + "/AppData/Roaming/Microsoft/Windows/Network Shortcuts";
  QDir networkShortcutsDir(networkShortcuts);
  for (const QFileInfo& entry : networkShortcutsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
  {
    if (!entry.isDir())
    {
      continue;
    }

    QString targetLinkPath = entry.filePath() + "/target.lnk";
    QFileInfo targetLink(targetLinkPath);
    if (!targetLink.exists())
    {
      continue;
    }

    QString path = targetLink.symLinkTarget();
    if (!path.isEmpty())
    {
      result.push_back(std::make_pair(entry.fileName(), path));
    }
  }
  return result;
}

void BreadCrumbsAddressBar::updateRootMenuDevices()
{
  auto menu = m_btnRootCrumb->menu();
  if (!m_actionsDevices.empty())
  {
    for (auto action : m_actionsDevices)
    {
      menu->removeAction(action);
    }
  }
  m_actionsDevices.clear();
  m_actionsDevices.push_back(menu->addSeparator());

  for (const auto& i : QStorageInfo::mountedVolumes())
  {
    QString path = i.rootPath();
    QString label = i.displayName();
    path.replace("/", "\\");
    if (label == path)
    {
      label = this->getPathLabel(path);
    }

    QString caption = QString("%1 (%2)").arg(label, path.trimmed());
    auto action = menu->addAction(this->getIcon(path), caption);
    action->setData(path);
    QObject::connect(action, &QAction::triggered, this, [this, action]() {
      this->setPath(action->data().toString());
      });
    m_actionsDevices.push_back(action);
  }

  // Call a function(listNetworkLocations) and store the return value in two values with a structured bound.
  // c++17 or higher.
  for (const auto& [label, path] : listNetworkLocations())
  {
    auto action = menu->addAction(this->getIcon(path), label);
    action->setData(path);
    QObject::connect(action, &QAction::triggered, this, [this, action]() {
      this->setPath(action->data().toString());
      });
    m_actionsDevices.push_back(action);
  }
}

QString BreadCrumbsAddressBar::getPathLabel(const QString& drivePath)
{
  PIDLIST_ABSOLUTE idlist = nullptr;

  HRESULT hr = SHParseDisplayName(drivePath.toStdWString().c_str(), nullptr, &idlist, 0, nullptr);
  if (FAILED(hr))
  {
    throw std::runtime_error("Exception in SHParseDisplayName.");
  }

  PWSTR name = nullptr;
  hr = SHGetNameFromIDList(idlist, SIGDN_PARENTRELATIVEEDITING, &name);
  if (FAILED(hr))
  {
    throw std::runtime_error("Exception in SHGetNameFromIDList.");
  }
  std::wstring label(name);
  CoTaskMemFree(name);
  ILFree(idlist);
  QString ret = QString::fromStdWString(label);
  return ret;
}

QString BreadCrumbsAddressBar::pathTitle(const QString& path)
{
  QString ret = path;
  if (!ret.isEmpty())
  {
    ret.replace("//", "");
  }
  return ret;
}

void BreadCrumbsAddressBar::slotBrowseForFolder()
{
  auto path = QFileDialog::getExistingDirectory(
    this
    , "Choose folder"
    , m_path
    );
  if (!path.isEmpty())
  {
    this->setPath(path);
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

void BreadCrumbsAddressBar::slotCrumbHideShow()
{
  auto retrievedLayout = m_crumbsPanel->layout();
  LeftHBoxLayout* boxLayout = dynamic_cast<LeftHBoxLayout*>(retrievedLayout);
  auto arrow = boxLayout->countHidden() > 0 ? Qt::LeftArrow : Qt::RightArrow;
  m_btnRootCrumb->setArrowType(arrow);
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
            QObject::connect(action, &QAction::trigger, this, [this, action]() {
              this->setPath(action->data().toString());
              });
            menu->insertAction(firstAction, action);
            m_actionsHiddenCrumbs.append(action);
            firstAction = action;
          }
        }
      }
    }
  }
}

void BreadCrumbsAddressBar::setPath(const QString& path)
{
  // Convert to the object that skipped the signal immediately before and obtain the file path from the data contained in the result.
  auto action = qobject_cast<QAction*>(this->sender());
  QString _path = path;
  if (_path.isEmpty() && action)
  {
    _path = action->data().toString();
  }
  std::filesystem::path fsPath(_path.toStdWString());
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
    return;
  }

  this->clearCrumbs();
  _path = _path.replace("/", "\\");
  m_path = _path;
  m_lineAddress->setText(_path);
  m_filenameModel->setPathPrefix(_path);
  QString fullpath = "";
  for (auto it = fsPath.begin(); it != fsPath.end(); it++)
  {
    QString itPath = QString::fromStdWString((*it).wstring());
    if ("\\" == itPath)
    {
      fullpath += "\\";
      continue;
    }
    else
    {
      fullpath = fullpath + itPath + "\\";
    }
    this->insertCrumbs(fullpath, itPath);
  }
  m_pathIcon->setPixmap(this->getIcon(m_path).pixmap(16, 16));
  emit signalPathSelected(m_path);

}

QIcon BreadCrumbsAddressBar::getIcon(const QString& path)
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
  auto retrievedLayout = m_crumbsPanel->layout();
  LeftHBoxLayout* boxLayout = dynamic_cast<LeftHBoxLayout*>(retrievedLayout);
  if (!boxLayout)
  {
    return;
  }

  auto maxCount = boxLayout->countAll();
  for (int i = 0; i < maxCount; i++)
  {
    auto widget = boxLayout->takeAt(0)->widget();
    QToolButton* btn = dynamic_cast<QToolButton*>(widget);
    if (btn)
    {
      btn->setStyle(nullptr);
      btn->deleteLater();
    }
  }
}

void BreadCrumbsAddressBar::insertCrumbs(const QString& fullpath, const QString& labelName)
{
  auto btn = new QToolButton(m_crumbsPanel);
  btn->setAutoRepeat(true);
  btn->setPopupMode(QToolButton::MenuButtonPopup);
  // btn->setStyle();
  btn->setMouseTracking(true);
  btn->setText(this->pathTitle(labelName));
  btn->setProperty("path", fullpath);
  QObject::connect(btn, &QToolButton::clicked, this, &BreadCrumbsAddressBar::slotCrumbClicked);

  auto menu = new MenuListView(btn);
  QObject::connect(menu, &MenuListView::aboutToShow, this, &BreadCrumbsAddressBar::slotCrumbMenuShow);
  menu->setModel(m_filenameModel);
  QObject::connect(menu, &MenuListView::clicked, this, &BreadCrumbsAddressBar::slotCrumbMenuItemClicked);
  QObject::connect(menu, &MenuListView::activated, this, &BreadCrumbsAddressBar::slotCrumbMenuItemClicked);
  QObject::connect(menu, &MenuListView::aboutToHide, m_mousePosTimer, &QTimer::stop);
  btn->setMenu(menu);
  auto retrievedLayout = m_crumbsPanel->layout();
  LeftHBoxLayout* boxLayout = dynamic_cast<LeftHBoxLayout*>(retrievedLayout);
  if (boxLayout)
  {
    // Add to the end
    boxLayout->addWidget(btn);
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
  auto menu = qobject_cast<MenuListView*>(this->sender());
  if (!menu)
  {
    return;
  }
  // QToolButton class is parent of MenuListView class.
  m_filenameModel->setPathPrefix(menu->parent()->property("path").toString());
  menu->clearSelection();
  m_mousePosTimer->start(100);
}

void BreadCrumbsAddressBar::slotCrumbMenuItemClicked(const QModelIndex& index)
{
  this->setPath(index.data(Qt::EditRole).toString());
}
