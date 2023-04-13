
#include "filenamemodel.h"

#include <QDir>

FilenameModel::FilenameModel(const QStringList& strings, const bool isDir, QObject* parent)
  : QStringListModel(strings, parent)
  , m_isDir(isDir)
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
    return defaultValue.toString();
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
    retList.push_back(path + "/" +  i);
  }
  return retList;
}

