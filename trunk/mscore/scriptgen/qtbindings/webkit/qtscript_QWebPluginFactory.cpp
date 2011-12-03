#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qwebpluginfactory.h>
#include <QVariant>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qlist.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qurl.h>
#include <qwebpluginfactory.h>

#include "qtscriptshell_QWebPluginFactory.h"

static const char * const qtscript_QWebPluginFactory_function_names[] = {
    "QWebPluginFactory"
    // static
    // prototype
    , "create"
    , "plugins"
    , "refreshPlugins"
    , "toString"
};

static const char * const qtscript_QWebPluginFactory_function_signatures[] = {
    "QObject parent"
    // static
    // prototype
    , "String mimeType, QUrl url, List argumentNames, List argumentValues"
    , ""
    , ""
""
};

static QScriptValue qtscript_QWebPluginFactory_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QWebPluginFactory::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QWebPluginFactory*)
Q_DECLARE_METATYPE(QtScriptShell_QWebPluginFactory*)
Q_DECLARE_METATYPE(QWebPluginFactory::Plugin)
Q_DECLARE_METATYPE(QList<QWebPluginFactory::Plugin>)

//
// QWebPluginFactory
//

static QScriptValue qtscript_QWebPluginFactory_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 3;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QWebPluginFactory* _q_self = qscriptvalue_cast<QWebPluginFactory*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QWebPluginFactory.%0(): this object is not a QWebPluginFactory")
            .arg(qtscript_QWebPluginFactory_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 4) {
        QString _q_arg0 = context->argument(0).toString();
        QUrl _q_arg1 = qscriptvalue_cast<QUrl>(context->argument(1));
        QStringList _q_arg2;
        qScriptValueToSequence(context->argument(2), _q_arg2);
        QStringList _q_arg3;
        qScriptValueToSequence(context->argument(3), _q_arg3);
        QObject* _q_result = _q_self->create(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        QList<QWebPluginFactory::Plugin> _q_result = _q_self->plugins();
        return qScriptValueFromSequence(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 0) {
        _q_self->refreshPlugins();
        return context->engine()->undefinedValue();
    }
    break;

    case 3: {
    QString result = QString::fromLatin1("QWebPluginFactory");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QWebPluginFactory_throw_ambiguity_error_helper(context,
        qtscript_QWebPluginFactory_function_names[_id+1],
        qtscript_QWebPluginFactory_function_signatures[_id+1]);
}

static QScriptValue qtscript_QWebPluginFactory_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QWebPluginFactory(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QtScriptShell_QWebPluginFactory* _q_cpp_result = new QtScriptShell_QWebPluginFactory();
        QScriptValue _q_result = context->engine()->newQObject(context->thisObject(), (QWebPluginFactory*)_q_cpp_result, QScriptEngine::AutoOwnership);
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    } else if (context->argumentCount() == 1) {
        QObject* _q_arg0 = context->argument(0).toQObject();
        QtScriptShell_QWebPluginFactory* _q_cpp_result = new QtScriptShell_QWebPluginFactory(_q_arg0);
        QScriptValue _q_result = context->engine()->newQObject(context->thisObject(), (QWebPluginFactory*)_q_cpp_result, QScriptEngine::AutoOwnership);
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QWebPluginFactory_throw_ambiguity_error_helper(context,
        qtscript_QWebPluginFactory_function_names[_id],
        qtscript_QWebPluginFactory_function_signatures[_id]);
}

static QScriptValue qtscript_QWebPluginFactory_toScriptValue(QScriptEngine *engine, QWebPluginFactory* const &in)
{
    return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

static void qtscript_QWebPluginFactory_fromScriptValue(const QScriptValue &value, QWebPluginFactory* &out)
{
    out = qobject_cast<QWebPluginFactory*>(value.toQObject());
}

QScriptValue qtscript_create_QWebPluginFactory_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        1
        // static
        // prototype
        , 4
        , 0
        , 0
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QWebPluginFactory*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QWebPluginFactory*)0));
    proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    for (int i = 0; i < 4; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QWebPluginFactory_prototype_call, function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QWebPluginFactory_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    qScriptRegisterMetaType<QWebPluginFactory*>(engine, qtscript_QWebPluginFactory_toScriptValue, 
        qtscript_QWebPluginFactory_fromScriptValue, proto);

    QScriptValue ctor = engine->newFunction(qtscript_QWebPluginFactory_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));

    return ctor;
}
