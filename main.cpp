/*  GUI для AddrConverter.
Внутри GUI - все для user-input через терминал.
*/
#include "widget.h"
#include <QApplication>

#include <QCommandLineParser>
//#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include "createfillnamespace.h"
#include "userinput.h"
#include "inputfileparsing.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget w;
    w.show();
    return app.exec();

    /* // далее: все для user-input через терминал
    //argc - количество аргументов, argv - значение аргумента, получаемого из командной строки
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.parse(app.arguments());
    QStringList cmdlineArgs = parser.positionalArguments();
    // QStringList options = parser.optionNames();



    InputFileParsing IFP;
    QMap<QString, QString> regnameAndAddr = IFP.parsing(cmdlineArgs[0]); // item [0] - название входного файла

    CreateFillNamespace CFNS;
    // item [1] - название будущего пространства имен, item [2] - название итогового файла,
    // item [3] - абсолютный путь той директории, где вы хотите сохранить итоговый файл
    CFNS.fillNSFile(cmdlineArgs[1], cmdlineArgs[2],  cmdlineArgs[3], regnameAndAddr);
    QFile file(cmdlineArgs[2]);
    if (file.exists())
        qDebug() << "output file now exists\n";
    else qDebug() << "creating output file failed\n";

    return app.exec();

// input files: project_address_map.v, bpvo_setup.h, CONFIG.h, config_ukazchik.h
// target file: trial... .h
// target directory is: /home/user/projects/AddrConverter/storage_dir
    */
}












