#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qpixmapcache.h>
#include <QVariant>
#include <qpixmap.h>

#include "qtscriptshell_QPixmapCache.h"

static const char * const qtscript_QPixmapCache_function_names[] = {
    "QPixmapCache"
    // static
    , "cacheLimit"
    , "clear"
    , "find"
    , "insert"
    , "remove"
    , "setCacheLimit"
    // prototype
    , "toString"
};

static const char * const qtscript_QPixmapCache_function_signatures[] = {
    ""
    // static
    , ""
    , ""
    , "String key, QPixmap arg__2"
    , "String key, QPixmap arg__2"
    , "String key"
    , "int arg__1"
    // prototype
""
};

static QScriptValue qtscript_QPixmapCache_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QPixmapCache::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QPixmapCache*)
Q_DECLARE_METATYPE(QtScriptShell_QPixmapCache*)

//
// QPixmapCache
//

static QScriptValue qtscript_QPixmapCache_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 0;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QPixmapCache* _q_self = qscriptvalue_cast<QPixmapCache*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QPixmapCache.%0(): this object is not a QPixmapCache")
            .arg(qtscript_QPixmapCache_function_names[_id+7]));
    }

    switch (_id) {
    case 0: {
    QString result = QString::fromLatin1("QPixmapCache");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QPixmapCache_throw_ambiguity_error_helper(context,
        qtscript_QPixmapCache_function_names[_id+7],
        qtscript_QPixmapCache_function_signatures[_id+7]);
}

static QScriptValue qtscript_QPixmapCache_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QPixmapCache(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QtScriptShell_QPixmapCache* _q_cpp_result = new QtScriptShell_QPixmapCache();
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QPixmapCache*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        int _q_result = QPixmapCache::cacheLimit();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 0) {
        QPixmapCache::clear();
        return context->engine()->undefinedValue();
    }
    break;

    case 3:
    if (context->argumentCount() == 2) {
        QString _q_arg0 = context->argument(0).toString();
        QPixmap _q_arg1 = qscriptvalue_cast<QPixmap>(context->argument(1));
        bool _q_result = QPixmapCache::find(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 4:
    if (context->argumentCount() == 2) {
        QString _q_arg0 = context->argument(0).toString();
        QPixmap _q_arg1 = qscriptvalue_cast<QPixmap>(context->argument(1));
        bool _q_result = QPixmapCache::insert(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 5:
    if (context->argumentCount() == 1) {
        QString _q_arg0 = context->argument(0).toString();
        QPixmapCache::remove(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 6:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QPixmapCache::setCacheLimit(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QPixmapCache_throw_ambiguity_error_helper(context,
        qtscript_QPixmapCache_function_names[_id],
        qtscript_QPixmapCache_function_signatures[_id]);
}

QScriptValue qtscript_create_QPixmapCache_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        0
        // static
        , 0
        , 0
        , 2
        , 2
        , 1
        , 1
        // prototype
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QPixmapCache*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QPixmapCache*)0));

    engine->setDefaultPrototype(qMetaTypeId<QPixmapCache*>(), proto);

    QScriptValue ctor = engine->newFunction(qtscript_QPixmapCache_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));
    for (int i = 0; i < 6; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QPixmapCache_static_call,
            function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i+1)));
        ctor.setProperty(QString::fromLatin1(qtscript_QPixmapCache_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    return ctor;
}
