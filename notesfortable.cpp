#include "notesfortable.h"
#include <QDebug>

NotesForTable::NotesForTable()
{
    status = static_cast<Status>(6);
}

NotesForTable::NotesForTable(Status stat)
{
    status = stat;
    qDebug() << "previous status: " << static_cast<int>(stat) << '\t' << getProcessStatus();
}

QString NotesForTable::getProcessStatus() const
{
    switch (status) {
    case Status::OK: return "Success";
        break;
    case Status::ERROR_INPUT: return "Error reading input file";
        break;
    case Status::NOT_PROCESSED: return "Not processed";
        break;
    default: return "Processing is not done yet";
    }
}
