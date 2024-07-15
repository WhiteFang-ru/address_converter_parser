#include "widget.h"
#include "ui_widget.h"


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
//    ui->leStorageDir->setText("Директория"); // актуально для первичного запуска проги
    //  активируем guiLog() сразу
    LM::Manager::instance(this)->addPlainTextEdit(ui->pteLogs);
    bufferForOutput = QApplication::clipboard();
    timerForEditingOutput = new QTimer(this);
    table = new ModelForTable();
    // .ini-шник берем из папки проекта:
    QDir::setCurrent(qApp->applicationDirPath());
    loadDirectory();

    storeDir = ui->leStorageDir->text();

    ui->tvMainTable->setModel(table);
    ui->tvMainTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvMainTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tvMainTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

    ui->pteLogs->setReadOnly(1); // bool function, 1 is 'true' for 'ReadOnly' state of 'Logs' and other widgets
    ui->leStorageDir->setReadOnly(1);

    connect(ui->pbAddInputFiles, &QPushButton::clicked, this, &Widget::chooseInputFiles);
    connect(ui->pbRemoveSelected, &QPushButton::clicked, this, &Widget::removeSelectedRows);
    connect(ui->pbChooseDirectory, &QPushButton::clicked , this, &Widget::chooseStoreDirectory);
    connect(ui->pbProcess, &QPushButton::clicked, this, &Widget::processingRows);
    connect(ui->tvMainTable->selectionModel(), &QItemSelectionModel::selectionChanged,this, &Widget::outputPreview);
    connect(ui->leStorageDir, &QLineEdit::textChanged, this, &Widget::saveDirectory);
    connect(timerForEditingOutput, &QTimer::timeout, this,  &Widget::editedOutputHighlighted);
    connect (ui->pbSaveEditing, &QPushButton::clicked, this, &Widget::updateEditedOutput);
    connect(ui->pbSaveAs, &QPushButton::clicked, this, &Widget::saveWithDifName);
    connect(ui->pbCancelEditing, &QPushButton::clicked, this, &Widget::outputPreview);
    connect(qApp, &QApplication::aboutToQuit, this, &Widget::saveBeforeQuiting);
}

void Widget::chooseInputFiles()
{
    newNotes.clear(); // preventive clearing

    QDir::setCurrent(qApp->applicationDirPath());
    table->saveCurrentTable();

    table->setNotes(newNotes);

    table->loadPreviousTable(newNotes);
    table->setNotes(newNotes); // эту команду вызываем вне if(dialog.exec()) и до него, чтобы модель прогрузилась
    // в tableView даже если новые файлы не выбраны (т е если dialog.exec() не отработает до конца и будет прерван нажатием на cancel)

    int sizeBeforeChoosingInput = newNotes.size();

    // чтобы были доступны для выбора только файлы с определенным расширением:
    QStringList filters;
    filters << "Text files (*.txt *.v *.h *.odt *.vh)"; // нулевое (и пока единственное) вхождение в этом списке
    QFileDialog dialog(this);
    dialog.setNameFilters(filters);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setViewMode(QFileDialog::List);

    NotesForTable note;
    QStringList tempList;
    if(dialog.exec()) {
        tempList.append(dialog.selectedFiles()); // вариант для накопления списка , если вызовов pushButton > 1
        for (int i=0; i<tempList.size(); ++i) {
            newNotes << note;
            newNotes[sizeBeforeChoosingInput + i].inputFile = tempList[i];//.section('/', -2); //  выводим часть пути - имя директории+имя файла (т е отсечение QString по второму '/' с конца)
        }
        table->setNotes(newNotes); // если успешно отработал dialog.exec(), т е  если новые файлы выбраны
        guiLog() << "Some input files were added to the list.";
    }
    newNotes.clear();
}

void Widget::chooseStoreDirectory()
{
    // по умолчанию итоговые файлы падают в build, поэтому нужно сделать выбор директории
    QFileDialog dialogDir(this);
    dialogDir.setOption(QFileDialog::ShowDirsOnly, true);
    dialogDir.setFileMode(QFileDialog::Directory);
    dialogDir.setViewMode(QFileDialog::List);
    if(dialogDir.exec()) {
        QStringList list = dialogDir.selectedFiles();
        if (!list.isEmpty()) {
            storeDir = list[0];
            ui->leStorageDir->setText(storeDir);
            guiLog() << "Store directory was chosen: " + storeDir;
        }
        else {
            guiLog() << "Directory for storing is not found.";
        }
    }
    table->setStoreDir(storeDir);
}

QList<int> Widget::getUnrepeatedRows()
{
    QList<int> fullListOfRows;
    selectedIdxs = ui->tvMainTable->selectionModel()->selectedIndexes();
    for(QModelIndex temp: qAsConst(selectedIdxs)) {
        fullListOfRows.append(temp.row());
    }
    QList<int>::iterator it = fullListOfRows.begin();
    selectedRows.append(*it); // нужно положить в список хотя бы один эл-т, чтобы сравнивать последующий с ним, т е с последним эл-том
    while (it != fullListOfRows.end()) {
        if(selectedRows.last() != *it) {
            selectedRows.append(*it);
            }
        it++;
    }
    return selectedRows;
}

void Widget::removeSelectedRows()
{
    newNotes.clear(); // preventive clearing

    selectedRows = getUnrepeatedRows();

    QDir::setCurrent(qApp->applicationDirPath());
    table->saveCurrentPartialTable(selectedRows);
    table->setNotes(newNotes);

    QDir::setCurrent(qApp->applicationDirPath());
    table->loadPreviousTable(newNotes);
    table->setNotes(newNotes);

    newNotes.clear();
}

bool Widget::searchJobDone(QString &inputNameToSearch)
{
    // ищу, есть ли конкретный входящий файл среди ранее успешно обработанных
    QDir::setCurrent(storeDir);
    QFile file(table->successfullyProcessed);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString allJobsDone = in.readAll();
            if (allJobsDone.contains(inputNameToSearch)) {return true;}
            file.close();
        }
        else {
            guiLog() << "Problems with file " << table->successfullyProcessed;
        }
    }
    return false;
}


void Widget::processingRows()
{
    // исключаем для пользователя возможность работать в папке build
    // в итоге он вынужден выбрать конкретную свою папку для работы
    QFileInfo fi(ui->leStorageDir->text());
    if (!fi.isDir()) {
        guiLog() << "Directory line edit holds this text: " << ui->leStorageDir->text() << '\n' << "You cannot continue until you choose the target directory.";
        return;
    }
    selectedRows = getUnrepeatedRows();
    selectedIdxs = ui->tvMainTable->selectionModel()->selectedIndexes();
    QDir::setCurrent(storeDir);
    for (const QModelIndex temp: qAsConst(selectedIdxs))  {
        QString dataOfSelected = temp.data().toString();
        if ((temp.column() == 0) && (searchJobDone(dataOfSelected)) == true) { // ... уже содержится к .ini с перечнем уже обработанных inputFiles
            int n = QMessageBox::warning(0, "Warning", "Input file " + dataOfSelected.section('/', -2) + " has already been processed. Do you want to process it once again?",
                                         QMessageBox::Yes | QMessageBox::No );
            // убрать из списка под обработку номер того ряда, по которому отработал QFileDialog  с нажатием на "NO"
            if (n == QMessageBox::No) { selectedRows.removeOne(temp.row());}
        }
    }
    table->processing(selectedRows);
    // чтобы после (успешного) завершения процессинга строки (когда выделена одна строка) , запускалась сразу в outputPreview
    if (selectedRows.size() == 1) {
        outputPreview();
    }
    // возвращаюсь в папку, откуда запускается прога
    QDir::setCurrent(qApp->applicationDirPath());
}

void Widget::saveDirectory()
{
    // возвращаюсь в build, чтобы избежать ошибок при работе с папками
    QDir::setCurrent(qApp->applicationDirPath());
    QSettings saveDir(SaveAll::defaultSettingsName(), QSettings::IniFormat);
    saveDir.beginGroup("Storage");
    saveDir.value("Directory").toString();
    // value каждый раз перезаписывается, старое value удаляется
    saveDir.setValue("Directory", ui->leStorageDir->text());
    saveDir.endGroup();
}

void Widget::loadDirectory()
{
    // возвращаюсь в build, чтобы избежать ошибок при работе с папками
    QDir::setCurrent(qApp->applicationDirPath());
    QString iniFileName = qApp->applicationName() + ".ini";
    QFile file(iniFileName);
    if (file.exists()) { // этот файл может не существовать, если программа запускается впервые на конкретном компе
        QSettings getDir(SaveAll::defaultSettingsName(), QSettings::IniFormat);
        getDir.beginGroup("Storage");
        QString dirForLoading = getDir.value("Directory").toString();
        ui->leStorageDir->setText(dirForLoading);
        guiLog() << "Storage directory name loaded from previous session: " << dirForLoading;
    }
}

// возможность предпросмотра итогового файла, когда выделена одна строка таблицы
void Widget::outputPreview()
{
    // очищаю окно QPlainTextEdit перед тем, как отобразить очередной итоговый файл
    ui->pteOutputPreview->clear();
    // возвращаю свойства рамки окна к дефолтным (актуально, если перед этим было активно окно с отредактированным текстом и с синей рамкой):
    ui->pteOutputPreview->setStyleSheet("border-color: grey");
    selectedIdxs = ui->tvMainTable->selectionModel()->selectedIndexes();
    if(selectedIdxs.size() == 0) { // обход бага: при клике на пространство вне строк таблицы (т е фактически при снятии выделения строки таблицы) прога крашилась: ASSERT failure in QList::operator[]: "index out of range"
        return;
    }
    // извлекаем data (QVariant) из колонки 2 (т е название outputFile)
    QString output = selectedIdxs[2].data().toString();
    // чтобы ф-я предпросмотра подтягивала файлы не из build, а из той папки, куда их сохранил польз-ль:
    QDir::setCurrent(storeDir); // то есть ui->leStorageDir->text()
    if (output.trimmed().size() > 0) { // т е не заходим в этот if и не пишем ничего в guiLog(), когда в ячейке "итоговый файл" пусто
        QFile file(output);
        QTextStream inStream(&file);
        if (file.exists()) { // если по этой строке таблицы прошел процессинг, то итоговый файл уже существует
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                while (!inStream.atEnd()) {
                    QString tmpLine = inStream.readLine();
                    ui->pteOutputPreview->appendPlainText(tmpLine);
                }
                // TODO: отображать содержимое итогового файла в pteOutputPreview так, чтобы видно было не окончание, а начало
//                QTextCursor cursor(ui->pteOutputPreview->document());
//                cursor.moveOpetation(start) start();
                ui->pteOutputPreview->setReadOnly(0); // чтобы можно было редактировать
                // записываю в буфер все, собранное в наст моменте в pteOutputPreview:
                bufferForOutput->setText(ui->pteOutputPreview->toPlainText());
                file.close();
                guiLog() << "Output file " << output << " is previewed.";

                // таймер с циклом по 1000 мили сек для проверки, обновился ли текст в окне outputPreview вводом с клавы
                timerForEditingOutput->start(1000);
            }
            else {guiLog() << "Cannot open " << output << " file.";}
        }
        // когда итогового файла не существует в указанной папке, делаем setReadOnly(1) для pteOutputPreview:
        else {
            guiLog() << "Output file " << output << " doesn't exist in folder " << storeDir; // то есть в ui->leStorageDir->text();
            bufferForOutput->clear(); // чтобы сравнение шло так: в pte пусто == в буфере пусто
            ui->pteOutputPreview->setReadOnly(1);
        }
    }
    // когда  имя итогового файла не указано, делаем setReadOnly(1) для pteOutputPreview:
    else {
        bufferForOutput->clear(); // чтобы сравнение шло так: в pte пусто == в буфере пусто
        ui->pteOutputPreview->setReadOnly(1);
//        guiLog() << "Name of output file is not specified";
    }

    // возвращаемся в директорию приложения, т к в ней хранятся .ini и .bin
    QDir::setCurrent(qApp->applicationDirPath());

}

void Widget::editedOutputHighlighted()
{
//    guiLog() << "Timer checkout.";
    // по if подкрашивать рамку окна pteOutputPreview синим - когда есть изменения в тексте по сравнению с буфером
    if ((ui->pteOutputPreview->toPlainText()) != (bufferForOutput->text())) {
        ui->pteOutputPreview->setStyleSheet("border: 2px solid rgb(0, 120, 255);"); // подкрашивать рамку окна pteOutputPreview синим
//        guiLog() << "pteOutputPreview is being edited";
    }
    // чтобы рамка возвращалась к исходному цвету, когда изменений нет
    else {ui->pteOutputPreview->setStyleSheet("border-color: grey");};
}

void Widget::updateEditedOutput()
{
    selectedIdxs = ui->tvMainTable->selectionModel()->selectedIndexes();
    QString output = selectedIdxs[2].data().toString();
    // подтягиваю итоговый файл не из build, а из той папки, куда их сохранил польз-ль:
    QDir::setCurrent(storeDir); // то есть ui->leStorageDir->text()
    QFile file(output);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << ui->pteOutputPreview->toPlainText();
        guiLog() << "Output file " << output << " was updated.";
        file.close();
        ui->pteOutputPreview->setStyleSheet("border-color: grey");
        // актуализирую буфер до состояния измененного и сохраненного файла до следующего selectionChanged()
        bufferForOutput->setText(ui->pteOutputPreview->toPlainText());
    }
    else {
        guiLog() << "Cannot open " << output << " output file for updating.";
    }
}

void Widget::saveBeforeQuiting()
{
    if ((ui->pteOutputPreview->toPlainText()) != (bufferForOutput->text())) {
        int n = QMessageBox::warning(0, "Warning", "You've made changes to the output file. Do you want to save changes?", QMessageBox::Yes | QMessageBox::No);
        if (n == QMessageBox::Yes) {
            updateEditedOutput();
        }
    }
}

void Widget::saveWithDifName()
{
   // TODO: возможно ли как-то ограничить юзера в выборе расширения? Сейчас он может указать (напечатать) любое расширение и вообще оставить файл без расширения
    QString filename = QFileDialog::getSaveFileName(this, "Save file", storeDir, "Text files (*.txt *.v *.h *.odt *.vh)");
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << ui->pteOutputPreview->toPlainText();
        guiLog() << "Output file " << filename << " was saved.";
        file.close();
   }
}

Widget::~Widget()
{
    QDir::setCurrent(qApp->applicationDirPath());
    table->saveCurrentTable();
//    delete timerForEditingOutput;
    delete table;
    delete ui;
}








