#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qwebframe.h>
#include <QVariant>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qicon.h>
#include <qlist.h>
#include <qnetworkrequest.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qprinter.h>
#include <qrect.h>
#include <qregion.h>
#include <qsize.h>
#include <qurl.h>
#include <qwebframe.h>
#include <qwebpage.h>

static const char * const qtscript_QWebFrame_function_names[] = {
    "QWebFrame"
    // static
    // prototype
    , "addToJavaScriptWindowObject"
    , "childFrames"
    , "frameName"
    , "geometry"
    , "hitTestContent"
    , "load"
    , "page"
    , "parentFrame"
    , "pos"
    , "render"
    , "renderTreeDump"
    , "scrollBarMaximum"
    , "scrollBarMinimum"
    , "scrollBarPolicy"
    , "scrollBarValue"
    , "setContent"
    , "setHtml"
    , "setScrollBarPolicy"
    , "setScrollBarValue"
    , "toHtml"
    , "toPlainText"
    , "toString"
};

static const char * const qtscript_QWebFrame_function_signatures[] = {
    ""
    // static
    // prototype
    , "String name, QObject object"
    , ""
    , ""
    , ""
    , "QPoint pos"
    , "QNetworkRequest request, Operation operation, QByteArray body\nQUrl url"
    , ""
    , ""
    , ""
    , "QPainter painter\nQPainter painter, QRegion clip"
    , ""
    , "Orientation orientation"
    , "Orientation orientation"
    , "Orientation orientation"
    , "Orientation orientation"
    , "QByteArray data, String mimeType, QUrl baseUrl"
    , "String html, QUrl baseUrl"
    , "Orientation orientation, ScrollBarPolicy policy"
    , "Orientation orientation, int value"
    , ""
    , ""
""
};

static QScriptValue qtscript_QWebFrame_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QWebFrame::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QWebFrame*)
Q_DECLARE_METATYPE(QList<QWebFrame*>)
Q_DECLARE_METATYPE(QWebHitTestResult)
Q_DECLARE_METATYPE(QNetworkAccessManager::Operation)
Q_DECLARE_METATYPE(QWebPage*)
Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(Qt::Orientation)
Q_DECLARE_METATYPE(Qt::ScrollBarPolicy)

//
// QWebFrame
//

static QScriptValue qtscript_QWebFrame_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 21;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QWebFrame* _q_self = qscriptvalue_cast<QWebFrame*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QWebFrame.%0(): this object is not a QWebFrame")
            .arg(qtscript_QWebFrame_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 2) {
        QString _q_arg0 = context->argument(0).toString();
        QObject* _q_arg1 = context->argument(1).toQObject();
        _q_self->addToJavaScriptWindowObject(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        QList<QWebFrame*> _q_result = _q_self->childFrames();
        return qScriptValueFromSequence(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->frameName();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 3:
    if (context->argumentCount() == 0) {
        QRect _q_result = _q_self->geometry();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 4:
    if (context->argumentCount() == 1) {
        QPoint _q_arg0 = qscriptvalue_cast<QPoint>(context->argument(0));
        QWebHitTestResult _q_result = _q_self->hitTestContent(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 5:
    if (context->argumentCount() == 1) {
        if ((qMetaTypeId<QNetworkRequest>() == context->argument(0).toVariant().userType())) {
            QNetworkRequest _q_arg0 = qscriptvalue_cast<QNetworkRequest>(context->argument(0));
            _q_self->load(_q_arg0);
            return context->engine()->undefinedValue();
        } else if ((qMetaTypeId<QUrl>() == context->argument(0).toVariant().userType())) {
            QUrl _q_arg0 = qscriptvalue_cast<QUrl>(context->argument(0));
            _q_self->load(_q_arg0);
            return context->engine()->undefinedValue();
        }
    }
    if (context->argumentCount() == 2) {
        QNetworkRequest _q_arg0 = qscriptvalue_cast<QNetworkRequest>(context->argument(0));
        QNetworkAccessManager::Operation _q_arg1 = qscriptvalue_cast<QNetworkAccessManager::Operation>(context->argument(1));
        _q_self->load(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 3) {
        QNetworkRequest _q_arg0 = qscriptvalue_cast<QNetworkRequest>(context->argument(0));
        QNetworkAccessManager::Operation _q_arg1 = qscriptvalue_cast<QNetworkAccessManager::Operation>(context->argument(1));
        QByteArray _q_arg2 = qscriptvalue_cast<QByteArray>(context->argument(2));
        _q_self->load(_q_arg0, _q_arg1, _q_arg2);
        return context->engine()->undefinedValue();
    }
    break;

    case 6:
    if (context->argumentCount() == 0) {
        QWebPage* _q_result = _q_self->page();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 7:
    if (context->argumentCount() == 0) {
        QWebFrame* _q_result = _q_self->parentFrame();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 8:
    if (context->argumentCount() == 0) {
        QPoint _q_result = _q_self->pos();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 9:
    if (context->argumentCount() == 1) {
        QPainter* _q_arg0 = qscriptvalue_cast<QPainter*>(context->argument(0));
        _q_self->render(_q_arg0);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 2) {
        QPainter* _q_arg0 = qscriptvalue_cast<QPainter*>(context->argument(0));
        QRegion _q_arg1 = qscriptvalue_cast<QRegion>(context->argument(1));
        _q_self->render(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 10:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->renderTreeDump();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 11:
    if (context->argumentCount() == 1) {
        Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
        int _q_result = _q_self->scrollBarMaximum(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 12:
    if (context->argumentCount() == 1) {
        Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
        int _q_result = _q_self->scrollBarMinimum(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 13:
    if (context->argumentCount() == 1) {
        Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
        Qt::ScrollBarPolicy _q_result = _q_self->scrollBarPolicy(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 14:
    if (context->argumentCount() == 1) {
        Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
        int _q_result = _q_self->scrollBarValue(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 15:
    if (context->argumentCount() == 1) {
        QByteArray _q_arg0 = qscriptvalue_cast<QByteArray>(context->argument(0));
        _q_self->setContent(_q_arg0);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 2) {
        QByteArray _q_arg0 = qscriptvalue_cast<QByteArray>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        _q_self->setContent(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 3) {
        QByteArray _q_arg0 = qscriptvalue_cast<QByteArray>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        QUrl _q_arg2 = qscriptvalue_cast<QUrl>(context->argument(2));
        _q_self->setContent(_q_arg0, _q_arg1, _q_arg2);
        return context->engine()->undefinedValue();
    }
    break;

    case 16:
    if (context->argumentCount() == 1) {
        QString _q_arg0 = context->argument(0).toString();
        _q_self->setHtml(_q_arg0);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 2) {
        QString _q_arg0 = context->argument(0).toString();
        QUrl _q_arg1 = qscriptvalue_cast<QUrl>(context->argument(1));
        _q_self->setHtml(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 17:
    if (context->argumentCount() == 2) {
        Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
        Qt::ScrollBarPolicy _q_arg1 = qscriptvalue_cast<Qt::ScrollBarPolicy>(context->argument(1));
        _q_self->setScrollBarPolicy(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 18:
    if (context->argumentCount() == 2) {
        Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
        int _q_arg1 = context->argument(1).toInt32();
        _q_self->setScrollBarValue(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 19:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->toHtml();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 20:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->toPlainText();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 21: {
    QString result = QString::fromLatin1("QWebFrame");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QWebFrame_throw_ambiguity_error_helper(context,
        qtscript_QWebFrame_function_names[_id+1],
        qtscript_QWebFrame_function_signatures[_id+1]);
}

static QScriptValue qtscript_QWebFrame_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    return context->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QWebFrame cannot be constructed"));
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QWebFrame_throw_ambiguity_error_helper(context,
        qtscript_QWebFrame_function_names[_id],
        qtscript_QWebFrame_function_signatures[_id]);
}

static QScriptValue qtscript_QWebFrame_toScriptValue(QScriptEngine *engine, QWebFrame* const &in)
{
    return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

static void qtscript_QWebFrame_fromScriptValue(const QScriptValue &value, QWebFrame* &out)
{
    out = qobject_cast<QWebFrame*>(value.toQObject());
}

QScriptValue qtscript_create_QWebFrame_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        0
        // static
        // prototype
        , 2
        , 0
        , 0
        , 0
        , 1
        , 3
        , 0
        , 0
        , 0
        , 2
        , 0
        , 1
        , 1
        , 1
        , 1
        , 3
        , 2
        , 2
        , 2
        , 0
        , 0
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QWebFrame*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QWebFrame*)0));
    proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    for (int i = 0; i < 22; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QWebFrame_prototype_call, function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QWebFrame_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    qScriptRegisterMetaType<QWebFrame*>(engine, qtscript_QWebFrame_toScriptValue, 
        qtscript_QWebFrame_fromScriptValue, proto);

    QScriptValue ctor = engine->newFunction(qtscript_QWebFrame_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));

    return ctor;
}
