
#include "filenamemodel.h"

FilenameModel::FilenameModel(const QStringList& strings, QObject* parent)
  : QStringListModel(strings, parent)
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
    // return file path
  }

  return defaultValue;
}

QIcon FilenameModel::getIcon(const QString& path) const
{
  return m_fileIconProvider.icon(QFileInfo(path));
}

QStringList FilenameModel::getFileList(const QString& path)
{
  
}


QStringList FilenameModel::sortPaths(QStringList& paths)
{
  
}

void FilenameModel::setPathPrefix()
{
  
}
