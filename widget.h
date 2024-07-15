#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "modelfortable.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDebug>
#include <list>
#include <QSettings>
#include "SaveAll/saveall.h"
#include <QTextDocument>
#include <QTimer>
#include <QClipboard>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
    QString storeDir;
    QList<NotesForTable> newNotes;
    QList<QModelIndex> selectedIdxs;
    QList<int> selectedRows;
    QTimer* timerForEditingOutput;
    QClipboard* bufferForOutput;

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QList<int> getUnrepeatedRows();
    void chooseInputFiles ();
    void removeSelectedRows();
    void chooseStoreDirectory();
    void processingRows();
    void saveDirectory();
    void loadDirectory();
    void updateEditedOutput();
    void saveBeforeQuiting();
    void saveWithDifName();
    bool searchJobDone(QString &inputNameToSearch);

public slots:
    void outputPreview();
    void editedOutputHighlighted();

private:
    Ui::Widget *ui;
    ModelForTable* table;
};
#endif // WIDGET_H
