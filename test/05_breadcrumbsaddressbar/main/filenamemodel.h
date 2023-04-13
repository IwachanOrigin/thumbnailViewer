
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
  explicit FilenameModel(const QStringList& strings, const bool isDir = false, QObject* parent = nullptr);
  virtual ~FilenameModel();

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  QIcon getIcon(const QString& path) const;
  QStringList getFileList(const QString& path);

private:
  QFileIconProvider m_fileIconProvider;
  bool m_isDir;
};

#endif // FILE_NAME_MODEL_H_
