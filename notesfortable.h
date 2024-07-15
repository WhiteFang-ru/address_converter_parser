#ifndef NOTESFORTABLE_H
#define NOTESFORTABLE_H
#include <QStringList>

enum class Status {
    OK = 0,
    ERROR_INPUT = 1,
    NOT_PROCESSED = 2
};


class NotesForTable
{
public:
    NotesForTable();;
    NotesForTable(Status stat);
    Status status;
    QString inputFile;
    QString outputFile;
    QString nSpace;
    QString getProcessStatus() const;
};

#endif // NOTESFORTABLE_H
