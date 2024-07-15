#include <QRegularExpression>
#include "processingfiles.h"
#include "logmanager/logmanager.h"


QMap<QString, QString> ProcessingFiles::parsing(QString inputFile, Status *statusPtr)
{
    QFile file(inputFile);
    QTextStream inStream(&file);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QString checkContents = inStream.readAll();
        if (checkContents.contains("32'h")) {
            file.seek(0); // возвращаемся к нулевой позиции, чтобы снова прочитать файл от начала
            while (!inStream.atEnd()) {
                QString addrLine = inStream.readLine();
                if (!addrLine.contains("//`define") && addrLine.contains("32'h")) {
                    addrLine.remove("`define");
                    QStringList list = addrLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    regName = list[0].trimmed();
                    addrHex = list[1].trimmed();
                    if(addrHex.contains('_')) {
                        addrHex.remove('_');
                    }
                    addrHex.replace(0, 4, "0x"); // replace 4 symbols (i e "32'h") with "0x" starting from 0 position
                    regNameAndAddr.insert(regName, addrHex); // insert entries to QMap (keys, values)
                }
            }
            guiLog() << "File parsing is done successfully for " << inputFile.section('/', -1);
        }
        else {
            *statusPtr = Status::ERROR_INPUT;
            guiLog() << inputFile.section('/', -1) << " doesn't contain any 32'h addresses ";
        }
        file.close();
    }
    else {
        *statusPtr = Status::ERROR_INPUT;
        guiLog() << "Cannot read input file: " << inputFile.section('/', -1);
        }

    return regNameAndAddr;
}

void ProcessingFiles::fillOutputFile(QString nSpace, QString outputFile, QString storeDir, QMap<QString, QString> regNameAndAddr, Status *statusPtr)
{
    QString defineStr = nSpace.toUpper() + "_H"; // for 'define' caption
    QDir::setCurrent(storeDir); // направляем в директорию, которую польз-ль выбрал через pushButton + QFileDialog
    QFile file(outputFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fout(&file); // поток записываемых данных направляем в файл
        fout << "#ifndef " << defineStr << '\n' << "#define " << defineStr << '\n' << "#include <cstdint>\n\n";
        fout << "namespace " << nSpace << " {" << '\n';
        QMapIterator<QString, QString> iter(regNameAndAddr); // iterator for map output
        while(iter.hasNext()) {
            iter.next();
            fout << "\tconst uint32_t " << iter.key() << " = " << iter.value() << ";\n";
        }
        fout << "}\n\n#endif\n";
        guiLog() << "Output file " << outputFile << " is created and filled.";

        *statusPtr = Status::OK;

        guiLog() << "Status is 'success'.";
        file.close();
    }
    else {
        *statusPtr = Status::NOT_PROCESSED;

        guiLog() << "Cannot write to target file: " << outputFile;
        guiLog() << "File status: 'not processed'";
    }
}



