#include <QRegularExpression>
#include "modelfortable.h"
#include "processingfiles.h"


ModelForTable::ModelForTable(QObject *parent)
    : QAbstractTableModel(parent)
{
    loadPreviousTable(notes);
//    NotesForTable noteTmp(Status::ERROR_INPUT);
    //    qDebug() << "Status at launch: " << static_cast<int>(noteTmp.status) << '\t' << noteTmp.getProcessStatus();
}

ModelForTable::~ModelForTable()
{
}


QStringList ModelForTable::setHdrList() const {
    // чтобы не иниициализировать список каждый раз при вызове headerData
    QStringList hdrList = {"Входящий файл","Статус файла", "Исходящий файл", "Название namespace-а"};
    return hdrList;
}


QVariant ModelForTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        return setHdrList()[section];
    }
    return QVariant();
}

void ModelForTable::setNotes(QList<NotesForTable> &newNotes)
{
    notes = newNotes;

    emit layoutChanged();
    emit dataChanged(QModelIndex(), QModelIndex());
}


int ModelForTable::rowCount(const QModelIndex &) const
{
    return notes.size();
}

int ModelForTable::columnCount(const QModelIndex &) const
{
    return countOfColumns;
}


QVariant ModelForTable::data(const QModelIndex &index, int role) const
{
    // роль DisplayRole в таблице сначала только у нулевого и первого столбцов;
    // после окончания редактирования DisplayRole присваивается также и 2 и 3му столбцам
    if (index.isValid() && role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return notes[index.row()].inputFile;
        }
        if (index.column() == 1)  {
            return notes[index.row()].getProcessStatus();
        }
        if (index.column() == 2)  {
            return notes[index.row()].outputFile;
        }
        if (index.column() == 3)  {
            return notes[index.row()].nSpace;
        }
    }
    if (index.isValid() && role == Qt::EditRole) {
        if (index.column() == 2)  {
            return notes[index.row()].outputFile;
        }
        if (index.column() == 3)  {
            return notes[index.row()].nSpace;
        }
    }

    return QVariant();
}

constexpr int sizeOfExtension{2};
bool ModelForTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QString target, strForCheck;
    if (index.isValid() && role == Qt::EditRole) {
        if (index.column() == 2)  {
            target = value.toString();
            strForCheck = target.trimmed();
            if (strForCheck.remove(0, strForCheck.size()-sizeOfExtension) != ".h") { // для проверки расширения на равенство ".h"
                guiLog() << "Output file name is incorrect: it must have an '.h' extension.";
                return false;
            }
            else if (target.contains(QRegularExpression("[А-Яа-я]+"))) {
                    guiLog() << "Name of output file must not contain cyrillic symbols.";
                    return false;
            }
            else {
                notes[index.row()].outputFile = target;
                emit layoutChanged();
                emit dataChanged(QModelIndex(), QModelIndex());
                return true;
            }
            for (const QChar& c: qAsConst(target)) {
                if (!c.isLetter()) {
                    if ((c != '-') && (c != '_')) {
                        guiLog() << "Name of output file must only contain Latin letter symbols, '-' and '_', and must start with a letter.";
                        return false;
                    }
                }
            }
        }

        // коммит от 07 июня 22: название неймспейса может начинаться ТОЛЬКО с латин. буквы,
        // далее допустимы - латин. буквы, цифры, '-' и '_'
        if (index.column() == 3) {
            target = value.toString();
            strForCheck = target;
            strForCheck.truncate(1); // для проверки первого символа - это латин буква или нет
            if (target.contains(QRegularExpression("[А-Яа-я]+")) || (target.trimmed() == "namespace")){
                guiLog() << "Name of your namespace must not be equal to 'namespace' or contain cyrillic symbols.";
                return false;
            }
            else if (!strForCheck.contains(QRegularExpression("[A-za-z]+"))) {
                    guiLog() << "Name of your namespace must start with a letter.";
                    return false;
            }
            // для проверки всей строки, но без первого символа:
            for (QChar c: target.mid(1)) {
                if (!c.isLetter() && !c.isDigit()) {
                    if ((c != '-') && (c != '_') ) {
                        guiLog() << "Name of your namespace must only contain Latin letter symbols, digits, '-' and '_'.";
                        return false;
                    }
                }
            }
            notes[index.row()].nSpace = value.toString();
            emit layoutChanged();
            emit dataChanged(QModelIndex(), QModelIndex());

            return true;
        }
        return false;
    }
    else {return false;}
}


void ModelForTable::saveCurrentTable() const
{
    QFile file(exchangeContents);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        for(int i = 0; i < notes.size(); ++i) {
            out << notes[i].inputFile << static_cast<int>(notes[i].status) << notes[i].outputFile << notes[i].nSpace;
        }
        file.close();
    }
    else {
        guiLog() << "Current contents of the table was not saved: problems with file " << exchangeContents;
    }
}

void ModelForTable::saveCurrentPartialTable(QList<int> &specificRows)
{
    QFile file(exchangeContents);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        for(int i = 0; i < notes.size(); ++i) {
            if(!(specificRows.contains(i))) {
                out << notes[i].inputFile << static_cast<int>(notes[i].status) << notes[i].outputFile << notes[i].nSpace;
            }
        }
        file.close();
    }
    else {
        guiLog() << "Specific rows of the table were not saved: problems with file " << exchangeContents;
    }
}

void ModelForTable::loadPreviousTable(QList<NotesForTable> &notes)  {
    // файл с данными из прошлой сессии извлекается из build
    NotesForTable note;
    QFile file(exchangeContents);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream in(&file);
            int tempInt;
            for (int i=0; !in.atEnd(); ++i) {
                notes << note;
                in >> notes[i].inputFile >> tempInt >> notes[i].outputFile >> notes[i].nSpace;
                notes[i].status = static_cast<Status>(tempInt);
            }
            file.close();
        }
        else {
            guiLog() << "Previous contents of the table was not uploaded: cannot open .bin file.";
        }
    }
    else {
        guiLog() << "First launch of the application.";
    }
}


void ModelForTable::saveJobDone(QString &inputSuccess)
{
    // сохраняю перечень обработанных файлов в той же папке, где эти outputFiles были ранее созданы
    QDir::setCurrent(storeDir);
    QFile file(successfullyProcessed);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream strm(&file);
        // имя каждого inputSuccess должно сохраняться один раз
        if (!(strm.readAll()).contains(inputSuccess)) {
            strm << inputSuccess << '\n';
        }
        file.close();
    }
    else {
        guiLog() << "Problems with file " << successfullyProcessed;
    }
}


void ModelForTable::setStoreDir(QString &storage)
{
    storeDir = storage;
}

void ModelForTable::processing(QList<int> &rowsForProcess)
{
    ProcessingFiles PF;
    QMap<QString, QString> resultsOfParsing;

    for (int &i: rowsForProcess) {
        if((notes[i].nSpace.size() > 0) && (notes[i].outputFile.size() > 0)) {
            resultsOfParsing = PF.parsing(notes[i].inputFile, &notes[i].status);
            if (!resultsOfParsing.isEmpty()) {  // если ф-я parsing выполнилась успешно, то:
                PF.fillOutputFile(notes[i].nSpace, notes[i].outputFile, storeDir, resultsOfParsing, &notes[i].status);
               // добавить название только что обработанного файла в .txt с перечнем ранее успешно обработанных inputFiles
                if (notes[i].status == Status::OK) {
                    saveJobDone(notes[i].inputFile);
                }
                emit dataChanged(QModelIndex(), QModelIndex());
            }
            else {
                guiLog() << "Output file for " << notes[i].inputFile.section('/', -1) << " cannot be created because parsing of input failed";
            }
        }
        else {
             guiLog() << "Namespace and name of output file must be specified";
        }
    }
}

Qt::ItemFlags ModelForTable::flags(const QModelIndex &index) const
{
    if ((index.column() == 2) || (index.column() == 3))  {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
    else {
        return Qt::ItemIsSelectable | QAbstractTableModel::flags(index);
    }
}










