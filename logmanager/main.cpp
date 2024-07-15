#include <QApplication>
#include <QPlainTextEdit>

#include <QTimer>
#include <QFile>

#include "logmanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    using namespace LM; // область имен логера

    QPlainTextEdit pte; // создать диалоговое окно
    pte.show();

    Manager log; // создать локальный менеджер
    log.addPlainTextEdit(&pte); // добавить QPlainTextEdit для вывода текста
    log.setFuncPrefix([](Interception &l)->Interception&{return l << "sufix";}); // установить кастомный суфикс, по умолчанию - это время
    log.setFuncPrefix([](Interception &l)->Interception&{return l << "prefix";}); // установить кастомный префикс

    QFile f("log.html"); //создать и открыть файл для сохранения логера
    if(f.open(QIODevice::WriteOnly)){
        log.addDevice(&f); // добавить этот файл в логер для вывода
        log << valid << "Ok open file!";
    }

    //пример использования для запси логов
    QTimer timerLocal;
    timerLocal.start(1000);
    QObject::connect(&timerLocal, &QTimer::timeout, [&log](){
        log << "local" << LM::red << "red" << Color(Qt::blue) << "blue" << error << "error"; // цвет следующего текста можно задавать несколькими способами, как видно из примера
    });

    Manager::instance(&pte); // иницилизация глобального логера
    Manager::instance()->addPlainTextEdit(&pte); // добавить QPlainTextEdit для вывода текста

    QTimer timerGlobal;
    timerGlobal.start(1000);
    QObject::connect(&timerGlobal, &QTimer::timeout, [](){
        guiLog() << "global" << LM::red << "red" << Color(Qt::blue) << "blue" << error << "error"; // вызывать можно из любого участка кода
    });

    return a.exec();
}
