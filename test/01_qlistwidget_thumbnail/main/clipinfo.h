
#ifndef CLIPINFO_H
#define CLIPINFO_H

#include <QListWidgetItem>

class ClipInfo : public QListWidgetItem
{
public:
    ClipInfo(const QString &clipName, QListWidget *parent = 0);
    ClipInfo(const QIcon & icon, const QString &clipName, QListWidget *parent = 0);
    ~ClipInfo() = default;

    QString getClipName() const { return m_clipName; }
    QString getClipFileName() const { return "ThisIsTheFullFilePathName"; }
private:
    QString m_clipName;
};

#endif

