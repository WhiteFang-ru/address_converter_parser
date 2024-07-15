#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QDebug>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QColor>
#include <QDateTime>
#include <QPlainTextEdit>
#include <stdio.h>
#include <QStringRef>

#define guiLog *LM::Manager::instance

namespace LM {
class Manager;
class Interception;
class Color : public QColor{
public:
    using QColor::QColor;
    const short mAligment{0};
};
//не знаю, насколько это нужно и правильно...

enum Type{
    txt = 0,
    html
};

using InterceptionLambda = std::function<Interception &(Interception&)>;
typedef Interception & (*InterceptionFunction)(Interception &);// manipulator function
class Interception : private QDebug
{
public:
    template<typename T>
    explicit Interception(Manager *logManager, InterceptionLambda prefix, InterceptionLambda suffix, T t)
        : QDebug(&mBufer), mLogManager(logManager), mSuffix(suffix){
        *this << prefix << t;
    }
    
    ~Interception();
    inline Interception &operator << (Color lColor){
        beginColor(lColor);
        return *this;
    }

    inline Interception &operator<<(QString t) {
        (*static_cast<QDebug*>(this)) << reinterpret_cast<const char*>(t.toUtf8().data());
        return *this;
    }
    inline Interception &operator<<(QStringRef t) {
        (*static_cast<QDebug*>(this)) << reinterpret_cast<const char*>(t.toUtf8().data());
        return *this;
    }
    inline Interception &operator<<(QStringView s) {
        (*static_cast<QDebug*>(this)) << reinterpret_cast<const char*>(s.toUtf8().data());
        return *this;
    }
    inline Interception &operator<<(QLatin1String t) {
        (*static_cast<QDebug*>(this)) << reinterpret_cast<const char*>(t.data());
        return *this;
    }
    template<typename T >
    inline Interception &operator<<(T t) {
        (*static_cast<QDebug*>(this)) << t;
        return *this;
    }
    inline Interception &operator<<(InterceptionLambda f)
    { return (f)(*this); }
    inline Interception &operator<<(InterceptionFunction f)
    { return (*f)(*this); }
    void initLogManager();
protected:
    Manager *mLogManager{nullptr};
    Type mType{html};
    bool mInitColor{false};
    QString mBufer{""};
    InterceptionLambda mSuffix;
    
    static QTextStream &st(QTextStream &s){
        return s;
    }
    void beginColor(const Color &lColor){
        endColor();
        if(mType){
            nospace().noquote() << "<font color=\"" + lColor.name() + "\">";
            setAutoInsertSpaces(true);
            quote();
            mInitColor = true;
        }
    }
    void endColor(){
        if(!mInitColor)
            return;
        const bool endMsg{endMessage()};
        nospace().noquote() <<  (endMsg ? "</font> " : "</font>");
        setAutoInsertSpaces(true);
        quote();
        mInitColor = false;
    }
    bool endMessage(){
        if(autoInsertSpaces() && mBufer.endsWith(QLatin1Char(' '))){
            mBufer.chop(1);
            return true;
        }else{
            return false;
        }
    }
};
#define registerColorLogManager(color) \
const InterceptionLambda color{[](Interception &l)->Interception&{l << Color(Qt::color); return l;}};
registerColorLogManager(color0)
registerColorLogManager(color1)
registerColorLogManager(black)
registerColorLogManager(white)
registerColorLogManager(darkGray)
registerColorLogManager(gray)
registerColorLogManager(lightGray)
registerColorLogManager(red)
registerColorLogManager(green)
registerColorLogManager(blue)
registerColorLogManager(cyan)
registerColorLogManager(magenta)
registerColorLogManager(yellow)
registerColorLogManager(darkRed)
registerColorLogManager(darkGreen)
registerColorLogManager(darkBlue)
registerColorLogManager(darkCyan)
registerColorLogManager(darkMagenta)
registerColorLogManager(darkYellow)
registerColorLogManager(transparent)

Interception &warning(Interception &l);
Interception &error(Interception &l);
Interception &valid(Interception &l);
Interception &info(Interception &l);

class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(QObject *parent = nullptr);
    static Manager *instance(QObject *parent = nullptr);
    template<typename T>
    Interception operator<<(T t){
        return Interception(this, mFuncPrefix, mFuncSufix, t);
    }
signals:
    void changeLogTxt(const QString &txt);
    void changeLogHtml(const QString &txt);
public:
    void addDevice(QIODevice *dev){
        QTextStream(dev) << "<style>\
                            p {\
                            color: rgb(255,255,255);\
                            margin-top: 0.0em; \
                            margin-bottom: 0.0em; \
                            font-family: Courier New;\
                            }\
                            body{\
                            background-color: #333333;\
                            }\
                            </style>";
                            connect(this, &Manager::changeLogHtml, dev, [=](const QString &txt){
            if(mLogSave){
                QTextStream(dev) << txt;
            }
        });
    }
    void appendLog(QString &txt);
    void setFuncPrefix(const InterceptionLambda &funcPrefix);
    void setFuncSufix(const InterceptionLambda &funcSufix);
    void addPlainTextEdit(QPlainTextEdit *pte);
    void removePlainTextEdit(QPlainTextEdit *pte);
    void addTextEdit(QTextEdit *te);
    void removeTextEdit(QTextEdit *te);
    bool logSave() const;
    void setLogSave(bool logSave);
    void addImage(const QImage &image, QTextEdit *te);

protected:
    InterceptionLambda mFuncPrefix{[=](Interception &l)->Interception&{return (l <<  QDateTime::currentDateTime().toString("hh:mm:ss:zzz ") + "&lt;&lt;");}};
    InterceptionLambda mFuncSufix{[=](Interception &l)->Interception&{return l;}};
    bool mLogSave{true};
    Type mType{html};
};

}



#endif // LOGMANAGER_H
