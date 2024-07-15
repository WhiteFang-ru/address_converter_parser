#include "saveall.h"

#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QToolButton>
#include <QSlider>
#include <QAction>
#include <QTimeEdit>
#include <QMainWindow>

#include <QString>
#include <QObject>
#include <QCoreApplication>
#include <QDir>

namespace SaveAll {
QRegularExpression saveOfKey(){
    return QRegularExpression("(SaveOff)|(_sof)");
}
enum class Type : qint8{
    Save = 0,
    Load
};
void runSaveLoad(QObject *obj, QSettings &settings, const Type &type){
    for(auto &l : SaveAll::listSettings()){
        bool ok{false};
        for(auto &n : l)
            ok |= (type == Type::Load ? n.second(obj, settings) :  n.first(obj, settings));
        //if(ok)
        //    break;
    }
}
void saveLoad(QObject *obj, QSettings &settings, const Type &type, const uint &_cur){

    if(obj->objectName().contains(saveOfKey()))
        return;
    settings.beginGroup(obj->objectName().isEmpty() ? obj->metaObject()->className() + QString("%0").arg(_cur) : obj->objectName());
    runSaveLoad(obj, settings, type);
    if(!(qobject_cast<QAbstractSpinBox*>(obj) || qobject_cast<QTimeEdit*>(obj))){
        uint cur{0};
        for(auto child : obj->children())
            saveLoad(child, settings, type, cur++);
    }
    settings.endGroup();
}

struct InitDefaulSaveLoad{
    InitDefaulSaveLoad(){
        SAVE_ALL_REGISTER_WIDGET(QLineEdit, setText, text);
        SAVE_ALL_REGISTER_WIDGET(QSpinBox, setValue, value);
        SAVE_ALL_REGISTER_WIDGET(QDoubleSpinBox, setValue, value);
        SAVE_ALL_REGISTER_WIDGET(QSlider, setValue, value);
        SAVE_ALL_REGISTER_WIDGET(QCheckBox, setChecked, isChecked);
        SAVE_ALL_REGISTER_WIDGET(QRadioButton, setChecked, isChecked);
        SAVE_ALL_REGISTER_WIDGET(QComboBox, setCurrentIndex, currentIndex);
        SAVE_ALL_REGISTER_WIDGET(QToolButton, setChecked, isChecked);
        SAVE_ALL_REGISTER_WIDGET(QGroupBox, setChecked, isChecked);
        //SAVE_ALL_REGISTER_WIDGET(QGroupBox, setTitle, title);
        SAVE_ALL_REGISTER_WIDGET(QAction, setChecked, isChecked);
        SAVE_ALL_REGISTER_WIDGET(QMainWindow, restoreGeometry, saveGeometry);
        SAVE_ALL_REGISTER_WIDGET_ARGS(QMainWindow, restoreState, saveState, 0);
    }
};

void init(){
    const static InitDefaulSaveLoad init;
}

void save(QObject *obj, QString nameFile, QSettings::Format format){
    init();
    QSettings settings(nameFile, format);
    uint cur{0};
    saveLoad(obj, settings, Type::Save, cur);
}

void load(QObject *obj, QString nameFile, QSettings::Format format){
    init();
    QSettings settings(nameFile, format);
    uint cur{0};
    saveLoad(obj, settings, Type::Load, cur); ;
}


QString defaultSettingsName(){
    static QString fileName(QFileInfo(QCoreApplication::applicationFilePath())
                        #ifdef WIN32
                            .completeBaseName()
                        #else
                            .fileName()
                        #endif
                            .remove(
                            #ifdef GIT_VERSION
                                "_" + QString(GIT_VERSION).replace('.','_')
                            #else
                                ""
                            #endif
                                ) + ".ini");
    return fileName;
}



QMap<size_t, QMap<QString, QPair<SetObj, SetObj> > > &listSettings()
{
    static QMap<size_t, QMap<QString, QPair<SetObj, SetObj>>> retVal;
    return retVal;
}
}
