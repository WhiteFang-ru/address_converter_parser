#ifndef MODELFORTABLE_H
#define MODELFORTABLE_H

//#include <QRegExp>
#include <QAbstractTableModel>
#include "notesfortable.h"
#include "logmanager/logmanager.h"
#include "processingfiles.h"
#include <iostream>
#include <QFile>
#include <QDataStream>
#include <QItemSelectionModel>
#include <QSettings>
#include "SaveAll/saveall.h"

//class Test{
//public:
//    explicit Test(int a): inside(a){}
//    int inside;

//};

class ModelForTable : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ModelForTable(QObject *parent = nullptr);
    ~ModelForTable();
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QStringList setHdrList() const; // list of headers for columns (horizontal orientation)
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void setNotes(QList<NotesForTable> &newNotes);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    void setStoreDir(QString &storage);
    void processing(QList<int> &rowsForProcess);
    void saveCurrentTable() const;
    void saveCurrentPartialTable(QList<int> &specificRows);
    void loadPreviousTable(QList<NotesForTable> &notes);
    QString successfullyProcessed = "output_processed_to_this_dir.txt"; // файл для хранения списка ранее успешно обработанных итоговых файлов
    void saveJobDone(QString &inputSuccess);
    void searchJobDone(QString &inputNameToSearch);

private:
    const int countOfColumns{4};
    QList<NotesForTable> notes;
    QString storeDir; // storage directory
    QString exchangeContents = "notes_to_datastream.bin"; // файл для хранения предыдущего содержания таблицы (которое было до окончания предыдущей сессии)

};

#endif // MODELFORTABLE_H
