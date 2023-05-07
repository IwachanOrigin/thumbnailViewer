
#ifndef CLIPINFO_H
#define CLIPINFO_H

#include <QListWidgetItem>

class ClipInfo : public QListWidgetItem
{
public:
  ClipInfo(const QString& clipName, const QString& clipFullPath, QListWidget* parent = nullptr);
  ClipInfo(const QIcon& icon, const QString& clipName, const QString& clipFullPath, QListWidget* parent = nullptr);
  virtual ~ClipInfo() = default;

  QString getClipName() const { return m_clipName; }
  QString getClipFileName() const { return m_clipFullPath; }

private:
  QString m_clipName;
  QString m_clipFullPath;
};

#endif

