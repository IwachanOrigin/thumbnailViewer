
#include "filenamemodel.h"

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QDebug>

#include <filesystem>

FilenameModel::FilenameModel(const QStringList& strings, const bool isDir, QObject* parent)
  : QStringListModel(strings, parent)
  , m_isDir(isDir)
  , m_currentPath("")
{
}

FilenameModel::~FilenameModel()
{
}

QVariant FilenameModel::data(const QModelIndex& index, int role) const
{
  // Get names/icons of files
  QVariant defaultValue = QStringListModel::data(index, role);
  if (role == Qt::DecorationRole)
  {
    QString filePath = defaultValue.toString();
    return this->getIcon(filePath);
  }

  if (role == Qt::DisplayRole)
  {
    // Since the actual path is a full path, only the trailing folder name is retrieved and displayed.
    std::filesystem::path filePath(defaultValue.toString().toStdWString());
    QString ret = QString::fromStdWString(filePath.filename().wstring());
    return ret;
  }

  return defaultValue;
}

QIcon FilenameModel::getIcon(const QString& path) const
{
  return m_fileIconProvider.icon(QFileInfo(path));
}

QStringList FilenameModel::getFileList(const QString& path)
{
  QDir dir(path);
  QDir::Filters filter;
  if (m_isDir)
  {
    filter = dir.Dirs;
  }
  else
  {
    filter = dir.AllEntries;
  }
  dir.setFilter(dir.NoDotAndDotDot | dir.Hidden | filter);
  QStringList names = dir.entryList(dir.NoFilter, dir.DirsFirst | dir.LocaleAware);
  QStringList retList;
  for (auto i : names)
  {
    retList.push_back(path + "\\" +  i);
  }
  return retList;
}

void FilenameModel::setPathPrefix(const QString& prefix)
{
  QString path = prefix;
#if 0
  QFileInfo fileInfo(prefix);
  if (fileInfo.isFile())
  {
    path = fileInfo.absoluteDir().absolutePath();
  }
#else
  if (!path.endsWith("\\"))
  {
    std::filesystem::path filePath(path.toStdWString());
    std::filesystem::path parentPath = filePath.parent_path();
    path = QString::fromStdWString(parentPath.wstring());
    qDebug() << "parent path = " << path;
  }
#endif
  if (path == m_currentPath)
  {
    qDebug() << "current path.";
    return;
  }

  QFileInfo fileInfo2(path);
  if (!fileInfo2.exists())
  {
    qDebug() << "not exist.";
    return;
  }
  this->setStringList(this->getFileList(path));
  qDebug() << "stringlist = " << this->stringList();
  m_currentPath = path;
}
