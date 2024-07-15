#ifndef PROCESSINGFILES_H
#define PROCESSINGFILES_H
#include <QFile>
#include <QDir>
#include <QMap>
#include "notesfortable.h"

class ProcessingFiles {
    QString regName, addrHex;
    QMap<QString, QString> regNameAndAddr; // key - regName, value - addr

public:
    NotesForTable note;
    QMap<QString, QString> parsing (QString inputFiles, Status *statusPtr);
    void fillOutputFile (QString nSpace, QString outputFile, QString storeDir, QMap<QString, QString> regnameAndAddr, Status *statusPtr);
    Status statusAfterProcessing();
};

#endif // PROCESSINGFILES_H
