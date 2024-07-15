#ifndef SAVEALL_H
#define SAVEALL_H

#include <QObject>
#include <QSettings>
#include <QHash>
#include <QVariant>
#include <QMetaProperty>
#include <QMetaObject>
#include <QDir>
#include <QRegularExpression>
#include <typeindex>

//сохранение/загрузка типа
#define SAVE_ALL_SAVE(x) SaveAll::SAVE(x, #x)
#define SAVE_ALL_LOAD(x) SaveAll::LOAD(x, #x)
//регистрация виджета
#define SAVE_ALL_REGISTER_WIDGET(Class, setValue, value) SaveAll::registerWidget<Class>(&Class::setValue, &Class::value, #value)
//регистрация виджета с дополнительными аргументами
#define SAVE_ALL_REGISTER_WIDGET_ARGS(Class, setValue, value, args...) SaveAll::registerWidget<Class>(&Class::setValue, &Class::value, #value, args)
//регистрация виджета с передачей управления Settings
#define SAVE_ALL_REGISTER_WIDGET_SETTINGS(Class, setValue, value) SaveAll::registerWidgetSettings<Class>(&Class::setValue, &Class::value, #value)
//исключить виджет
#define SAVE_ALL_UNREGISTER_WIDGET(Class) SaveAll::unRegisterWidget<Class>()

namespace SaveAll {
// возврат дефолтного имени настроек
QString defaultSettingsName();
//методы сохранения/загрузки
void save(QObject *obj, QString nameFile = defaultSettingsName(), QSettings::Format format = QSettings::Format::IniFormat);
void load(QObject *obj, QString nameFile = defaultSettingsName(), QSettings::Format format = QSettings::Format::IniFormat);

// методы для макросов, смотреть выше
template <typename T>
void SAVE(const T& t, QString name){
    QSettings set(defaultSettingsName(), QSettings::Format::IniFormat);
    set.beginGroup("GLOBAL");
    set.setValue(name.replace(QRegularExpression("(\\:){1,2}"),".").remove(QRegularExpression("(\\()|(\\))")), t);
    set.endGroup();
}

template <typename T>
void LOAD(T& t, QString name){
    QSettings set(defaultSettingsName(), QSettings::Format::IniFormat);
    set.beginGroup("GLOBAL");
    if(set.contains(name.replace(QRegularExpression("(\\:){1,2}"),".").remove(QRegularExpression("(\\()|(\\))"))))
        t = set.value(name).value<T>();
    set.endGroup();
}

using SetObj = std::function<bool (QObject *wgt, QSettings &settings)>;
QMap<size_t, QMap<QString, QPair<SetObj, SetObj>>> &listSettings();

template <class Class, typename SetValue, typename Value, typename ...A>
void registerWidget(SetValue setValue, Value value, const QString &name, A...args){
    auto hashCode{std::type_index(typeid(Class)).hash_code()};
    auto it{SaveAll::listSettings().find(hashCode)};
    if(it != SaveAll::listSettings().end())
        if(it.value().contains(name))
            return;
    QPair<SetObj, SetObj> p{
        [=](QObject *wgt, QSettings &settings)->bool{
            if(auto o{qobject_cast<Class*>(wgt)}){
                settings.setValue(name, QVariant::fromValue((o->*value)(args...)));
                return true;
            }
            return false;
        },
        [=](QObject *wgt, QSettings &settings)->bool{
            if(auto o{qobject_cast<Class*>(wgt)}){
                if(settings.contains(name))
                    (o->*setValue)(QVariant(settings.value(name)).value<decltype((o->*value)(args...))>(), args...);
                return true;
            }
            return false;
        }
    };
    SaveAll::listSettings()[hashCode][name] = p;
}

template <class Class, typename Load, typename Save>
void registerWidgetSettings(Load load, Save save, const QString &name){
    auto hashCode{std::type_index(typeid(Class)).hash_code()};
    auto it{SaveAll::listSettings().find(hashCode)};
    if(it != SaveAll::listSettings().end())
        if(it.value().contains(name))
            return;
    QPair<SetObj, SetObj> p{
        [=](QObject *wgt, QSettings &settings)->bool{
            if(auto o{qobject_cast<Class*>(wgt)}){
                (o->*save)(settings);
                return true;
            }
            return false;
        },
        [=](QObject *wgt, QSettings &settings)->bool{
            if(auto o{qobject_cast<Class*>(wgt)}){
                (o->*load)(settings);
                return true;
            }
            return false;
        }
    };
    SaveAll::listSettings()[hashCode][name] = p;
}
template <typename T>
void unRegisterWidget(){
    SaveAll::listSettings().remove(std::type_index(typeid(T)).hash_code());
}
}
class SettingsSaver: public QSettings
{
    Q_OBJECT
    QObject *checkTarget(QObject *target){
        if(target){
            return target;
        }else{
            if(parent()){
                return parent();
            }else {
                return this;
            }
        }
    }
public:
    SettingsSaver(const QString &fileName, Format format, QObject *parent = nullptr)
        : QSettings(fileName, format, parent)
    {
    }
    explicit SettingsSaver(QObject *parent = nullptr)
        : QSettings(QDir::currentPath() + "/" +QString::fromUtf8(parent ? getMeta(parent)->className() : "Settings") + ".ini",QSettings::IniFormat, parent)
    {
    }
    ~SettingsSaver(){}
    void save(QObject *target = nullptr, QString extraGroupKey = "")
    {
        saveWithPath(checkTarget(target), extraGroupKey);
    }
    void load(QObject *target = nullptr, QString extraGroupKey = "")
    {
        loadFromPath(checkTarget(target), extraGroupKey);
    }
    void saveChildren()
    {
        saveChildren(this);
    }
    void loadChildren()
    {
        loadChildren(this);
    }
    void saveWithPath(QObject *target, QString extraGroupKey = "")
    {
        const QMetaObject* metaTarget = getMeta(target);
        propertiesList = collectPropsNames(target);

        int offset = metaTarget->propertyOffset();
        int end = metaTarget->propertyCount();
        beginGroup(QString::fromLocal8Bit("Settings_") + extraGroupKey);
        for (int i = offset; i < end; i++){
            QMetaProperty metaProp = metaTarget->property(i);
            if (metaProp.isEnumType()){
                QMetaEnum me = metaProp.enumerator();
                QString str = strFromChars(me.key(metaProp.read(target).toInt()));
                setValue(strFromChars(metaProp.name()), str);
                continue;
            }
            setValue(strFromChars(metaProp.name()), QVariant::fromValue(metaProp.read(target)));
        }
        endGroup();
    }
    void loadFromPath(QObject *target, QString extraGroupKey = "")
    {
        const QMetaObject* metaTarget = getMeta(target);
        propertiesList = collectPropsNames(target);
        QStringList propNames = collectPropsNames(target);
        beginGroup(QString::fromLocal8Bit("Settings_")+ extraGroupKey);
        for (QString& propName: propNames){
            QMetaProperty metaProp = metaTarget->property(metaTarget->indexOfProperty(propName.toLocal8Bit().data()));
            QVariant value = this->value(propName);
            if (!value.isNull()){
                metaProp.write(target, value);
            }
            QMetaMethod signal = metaProp.notifySignal();
            /*bool emitted = */signal.invoke(target);
        }
        endGroup();
    }
    const QMetaObject* getMeta(QObject* obj){
        return obj->metaObject();
    }
    QStringList collectPropsNames(const QObject* obj){
        const QMetaObject* metaObject = obj->metaObject();
        QStringList properties;
        for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
            properties   << strFromChars(metaObject->property(i).name());
        return properties;
    }
    static QString strFromChars(const char* array){
        return QString::fromUtf8(array);
    }

    QStringList propertiesList;
    QString setsName;

private:
    void saveChildren(QObject *target)
    {
        saveWithPath(target, target->metaObject()->className());
        QObjectList children = target->children();
        if (children.isEmpty()) return;
        foreach(QObject * child, children)
            saveChildren(child);

    }
    void loadChildren(QObject *target)
    {
        loadFromPath(target, target->metaObject()->className());
        QObjectList children = target->children();
        if (children.isEmpty()) return;
        foreach(QObject * child, children)
            loadChildren(child);
    }
};
#endif // SAVEALL_H
