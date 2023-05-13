
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
    QString _path = "";
    if (path.endsWith("\\"))
    {
      /*
       * If there is a '\\' at the end, it is assigned twice.
       * As a result, QCompleter fails to complete.
       */
      _path = path + i;
    }
    else
    {
      _path = path + "\\" + i;
    }
    retList.push_back(_path);
  }
  return retList;
}

void FilenameModel::setPathPrefix(const QString& prefix)
{
  QString path = prefix;

  if (!path.endsWith("\\"))
  {
    std::filesystem::path filePath(path.toStdWString());
    std::filesystem::path parentPath = filePath.parent_path();
    path = QString::fromStdWString(parentPath.wstring()); // ex. path = C://mypath
  }

  QFileInfo fileInfo2(path);
  if (!fileInfo2.exists())
  {
    return;
  }
  this->setStringList(this->getFileList(path));
  qDebug() << "stringList = " << this->stringList();
  m_currentPath = path; // ex. m_currentPath = c://mypath/
}
