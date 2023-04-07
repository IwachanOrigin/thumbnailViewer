
#include "clipinfo.h"

ClipInfo::ClipInfo(const QString &clipName, QListWidget *parent)
  : QListWidgetItem(clipName, parent)
  , m_clipName(clipName)
{
}

ClipInfo::ClipInfo(const QIcon &icon, const QString &clipName, QListWidget *parent)
  : QListWidgetItem(icon, clipName, parent)
  , m_clipName(clipName)
{
}
