
#include "clipinfo.h"

ClipInfo::ClipInfo(const QString &clipName, const QString& clipFullPath, QListWidget *parent)
  : QListWidgetItem(clipName, parent)
  , m_clipName(clipName)
  , m_clipFullPath(clipFullPath)
{
}

ClipInfo::ClipInfo(const QIcon &icon, const QString &clipName, const QString& clipFullPath, QListWidget *parent)
  : QListWidgetItem(icon, clipName, parent)
  , m_clipName(clipName)
  , m_clipFullPath(clipFullPath)
{
}

