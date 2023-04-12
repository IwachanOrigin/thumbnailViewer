
#ifndef FILE_NAME_MODEL_H_
#define FILE_NAME_MODEL_H_

#include <QObject>
#include <QStringList>
#include <QStringListModel>
#include <QModelIndex>
#include <QVariant>

class FilenameModel : public QStringListModel
{
public:
  explicit FilenameModel(const QStringList& strings, QObject* parent = nullptr);
  virtual ~FilenameModel();

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // FILE_NAME_MODEL_H_
