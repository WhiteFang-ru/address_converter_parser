#include "logmanager.h"

LM::Interception::~Interception(){
    endColor();
    *this << mSuffix;
    endMessage();
    mBufer.replace('\n', "<p  style=\"margin-top: 0.0em; margin-bottom: 0em;\">");
    mLogManager->appendLog(mBufer);
}

void LM::Interception::initLogManager(){
    if(mLogManager)
        mLogManager = Manager::instance();
}

LM::Manager::Manager(QObject *parent)
    : QObject(parent)
{
}

LM::Manager *LM::Manager::instance(QObject *parent){
    static Manager *obj = new Manager;
    if(parent){
        obj->setParent(parent);
    }
    return obj;
}
void LM::Manager::appendLog(QString &txt)
{
    emit changeLogTxt(Qt::convertFromPlainText(txt, Qt::WhiteSpaceNormal));
    emit changeLogHtml(/*"<p>" + */txt/* + "</p>"*/);
}

void LM::Manager::setFuncPrefix(const InterceptionLambda &funcPrefix)
{
    mFuncPrefix = funcPrefix;
}

void LM::Manager::setFuncSufix(const InterceptionLambda &funcSufix)
{
    mFuncSufix = funcSufix;
}

void LM::Manager::addPlainTextEdit(QPlainTextEdit *pte){
    connect(this, &Manager::changeLogHtml, pte, [=](const QString &txt){pte->appendHtml(txt);});
}

void LM::Manager::removePlainTextEdit(QPlainTextEdit *pte){
    disconnect(pte);
    pte->disconnect(this);
}

void LM::Manager::addTextEdit(QTextEdit *te)
{
    connect(this, &Manager::changeLogHtml, te, [=](const QString &txt){te->append(txt);});
}

void LM::Manager::removeTextEdit(QTextEdit *te)
{
    disconnect(te);
    te->disconnect(this);
}

bool LM::Manager::logSave() const
{
    return mLogSave;
}

void LM::Manager::setLogSave(bool logSave)
{
    mLogSave = logSave;
}

void LM::Manager::addImage(const QImage &image, QTextEdit *te){
    image.save("tmp.png");
    te->append("");
    QTextDocument *textDocument = te->document();
    textDocument->addResource(QTextDocument::ImageResource, QUrl("tmp.png"), QVariant ( image ) );
    QTextCursor cursor = te->textCursor();
    QTextImageFormat imageFormat;
    imageFormat.setWidth( image.width() );
    imageFormat.setHeight( image.height() );
    imageFormat.setName("tmp.png");
    cursor.insertImage(imageFormat);
}





LM::Interception &LM::warning(LM::Interception &l){
    return (l << LM::Color(Qt::darkYellow));
}

LM::Interception &LM::error(LM::Interception &l){
    return (l << LM::Color(Qt::red));
}

LM::Interception &LM::valid(LM::Interception &l){
    return (l << LM::Color(Qt::green));
}

LM::Interception &LM::info(LM::Interception &l){
    return (l << LM::Color(0,0x99,0x99));
}
