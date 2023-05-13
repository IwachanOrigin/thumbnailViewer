
#ifndef FILE_NAME_MODEL_H_
#define FILE_NAME_MODEL_H_

#include <QObject>
#include <QStringList>
#include <QStringListModel>
#include <QModelIndex>
#include <QIcon>
#include <QFileIconProvider>
#include <QVariant>

class FilenameModel : public QStringListModel
{
public:
  explicit FilenameModel(const QStringList& strings = QStringList(), const bool isDir = false, QObject* parent = nullptr);
  virtual ~FilenameModel();

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  QIcon getIcon(const QString& path) const;

public slots:
  void setPathPrefix(const QString& prefix);

private:
  QFileIconProvider m_fileIconProvider;
  bool m_isDir;
  QString m_currentPath;

  QStringList getFileList(const QString& path);
};

#endif // FILE_NAME_MODEL_H_
