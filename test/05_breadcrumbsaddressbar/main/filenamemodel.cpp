
#include "filenamemodel.h"

FilenameModel::FilenameModel(const QStringList& strings, QObject* parent)
  : QStringListModel(strings, parent)
{
}

FilenameModel::~FilenameModel()
{
}

QVariant FilenameModel::data(const QModelIndex &index, int role) const
{
  // Get names/icons of files
  if (role == Qt::DecorationRole)
  {
    
  }

  if (role == Qt::DisplayRole)
  {
    
  }
}
