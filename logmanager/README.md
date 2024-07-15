Вопрос: Для чего?
Ответ: для более простой записи логов в html формате QPlainTextEdit, QTextEdit и QFile или другой QIODevice с разными цветами.  
Вопрос: Как пользоваться?  
Ответ: Очень просто (использование логера похоже на QDebug):     
1.    **_локальное использование_**  
        //..code...  
        using namespace  LM;  
        Manager log;  
        log.addPlainTextEdit(&pte); // добавить QPlainTextEdit если нужно  
        //..code...  
        log.addDevice(&fLog); // добавить QFile если нужно  
        //..code...  
        log << "Hello world";  
        
2.    **_глобальное использование_**  
        //..code...  
        using namespace  LM;  
        Manager::instance(QObject*)->addPlainTextEdit(&pte);// указать родителя и добавить QPlainTextEdit если нужно  
        //..code..  
        Manager::instance()->addDevice(&fLog); // добавить QFile если нужно  
        guiLog() << "Hello world"; // можно вызывать из любого учатка кода  
 <img src="/exampleCode.gif" width="400" height="300"/>
 <img src="exampleGui.gif" width="400" height="300"/>