#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qnetworkproxy.h>
#include <QVariant>
#include <qnetworkproxy.h>

static const char * const qtscript_QNetworkProxy_function_names[] = {
    "QNetworkProxy"
    // static
    , "applicationProxy"
    , "setApplicationProxy"
    // prototype
    , "hostName"
    , "isCachingProxy"
    , "isTransparentProxy"
    , "equals"
    , "password"
    , "port"
    , "setHostName"
    , "setPassword"
    , "setPort"
    , "setType"
    , "setUser"
    , "type"
    , "user"
    , "toString"
};

static const char * const qtscript_QNetworkProxy_function_signatures[] = {
    "\nProxyType type, String hostName, unsigned short port, String user, String password\nQNetworkProxy other"
    // static
    , ""
    , "QNetworkProxy proxy"
    // prototype
    , ""
    , ""
    , ""
    , "QNetworkProxy other"
    , ""
    , ""
    , "String hostName"
    , "String password"
    , "unsigned short port"
    , "ProxyType type"
    , "String userName"
    , ""
    , ""
""
};

static QScriptValue qtscript_QNetworkProxy_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QNetworkProxy)
Q_DECLARE_METATYPE(QNetworkProxy*)
Q_DECLARE_METATYPE(QNetworkProxy::ProxyType)

static QScriptValue qtscript_create_enum_class_helper(
    QScriptEngine *engine,
    QScriptEngine::FunctionSignature construct,
    QScriptEngine::FunctionSignature valueOf,
    QScriptEngine::FunctionSignature toString)
{
    QScriptValue proto = engine->newObject();
    proto.setProperty(QString::fromLatin1("valueOf"),
        engine->newFunction(valueOf), QScriptValue::SkipInEnumeration);
    proto.setProperty(QString::fromLatin1("toString"),
        engine->newFunction(toString), QScriptValue::SkipInEnumeration);
    return engine->newFunction(construct, proto, 1);
}

//
// QNetworkProxy::ProxyType
//

static const QNetworkProxy::ProxyType qtscript_QNetworkProxy_ProxyType_values[] = {
    QNetworkProxy::DefaultProxy
    , QNetworkProxy::Socks5Proxy
    , QNetworkProxy::NoProxy
    , QNetworkProxy::HttpProxy
    , QNetworkProxy::HttpCachingProxy
    , QNetworkProxy::FtpCachingProxy
};

static const char * const qtscript_QNetworkProxy_ProxyType_keys[] = {
    "DefaultProxy"
    , "Socks5Proxy"
    , "NoProxy"
    , "HttpProxy"
    , "HttpCachingProxy"
    , "FtpCachingProxy"
};

static QString qtscript_QNetworkProxy_ProxyType_toStringHelper(QNetworkProxy::ProxyType value)
{
    if ((value >= QNetworkProxy::DefaultProxy) && (value <= QNetworkProxy::FtpCachingProxy))
        return qtscript_QNetworkProxy_ProxyType_keys[static_cast<int>(value)];
    return QString();
}

static QScriptValue qtscript_QNetworkProxy_ProxyType_toScriptValue(QScriptEngine *engine, const QNetworkProxy::ProxyType &value)
{
    QScriptValue clazz = engine->globalObject().property(QString::fromLatin1("QNetworkProxy"));
    return clazz.property(qtscript_QNetworkProxy_ProxyType_toStringHelper(value));
}

static void qtscript_QNetworkProxy_ProxyType_fromScriptValue(const QScriptValue &value, QNetworkProxy::ProxyType &out)
{
    out = qvariant_cast<QNetworkProxy::ProxyType>(value.toVariant());
}

static QScriptValue qtscript_construct_QNetworkProxy_ProxyType(QScriptContext *context, QScriptEngine *engine)
{
    int arg = context->argument(0).toInt32();
    if ((arg >= QNetworkProxy::DefaultProxy) && (arg <= QNetworkProxy::FtpCachingProxy))
        return qScriptValueFromValue(engine,  static_cast<QNetworkProxy::ProxyType>(arg));
    return context->throwError(QString::fromLatin1("ProxyType(): invalid enum value (%0)").arg(arg));
}

static QScriptValue qtscript_QNetworkProxy_ProxyType_valueOf(QScriptContext *context, QScriptEngine *engine)
{
    QNetworkProxy::ProxyType value = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->thisObject());
    return QScriptValue(engine, static_cast<int>(value));
}

static QScriptValue qtscript_QNetworkProxy_ProxyType_toString(QScriptContext *context, QScriptEngine *engine)
{
    QNetworkProxy::ProxyType value = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->thisObject());
    return QScriptValue(engine, qtscript_QNetworkProxy_ProxyType_toStringHelper(value));
}

static QScriptValue qtscript_create_QNetworkProxy_ProxyType_class(QScriptEngine *engine, QScriptValue &clazz)
{
    QScriptValue ctor = qtscript_create_enum_class_helper(
        engine, qtscript_construct_QNetworkProxy_ProxyType,
        qtscript_QNetworkProxy_ProxyType_valueOf, qtscript_QNetworkProxy_ProxyType_toString);
    qScriptRegisterMetaType<QNetworkProxy::ProxyType>(engine, qtscript_QNetworkProxy_ProxyType_toScriptValue,
        qtscript_QNetworkProxy_ProxyType_fromScriptValue, ctor.property(QString::fromLatin1("prototype")));
    for (int i = 0; i < 6; ++i) {
        clazz.setProperty(QString::fromLatin1(qtscript_QNetworkProxy_ProxyType_keys[i]),
            engine->newVariant(qVariantFromValue(qtscript_QNetworkProxy_ProxyType_values[i])),
            QScriptValue::ReadOnly | QScriptValue::Undeletable);
    }
    return ctor;
}

//
// QNetworkProxy
//

static QScriptValue qtscript_QNetworkProxy_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 13;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QNetworkProxy* _q_self = qscriptvalue_cast<QNetworkProxy*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QNetworkProxy.%0(): this object is not a QNetworkProxy")
            .arg(qtscript_QNetworkProxy_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->hostName();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        bool _q_result = _q_self->isCachingProxy();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 0) {
        bool _q_result = _q_self->isTransparentProxy();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 3:
    if (context->argumentCount() == 1) {
        QNetworkProxy _q_arg0 = qscriptvalue_cast<QNetworkProxy>(context->argument(0));
        bool _q_result = _q_self->operator==(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 4:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->password();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 5:
    if (context->argumentCount() == 0) {
        unsigned short _q_result = _q_self->port();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 6:
    if (context->argumentCount() == 1) {
        QString _q_arg0 = context->argument(0).toString();
        _q_self->setHostName(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 7:
    if (context->argumentCount() == 1) {
        QString _q_arg0 = context->argument(0).toString();
        _q_self->setPassword(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 8:
    if (context->argumentCount() == 1) {
        unsigned short _q_arg0 = qscriptvalue_cast<unsigned short>(context->argument(0));
        _q_self->setPort(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 9:
    if (context->argumentCount() == 1) {
        QNetworkProxy::ProxyType _q_arg0 = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->argument(0));
        _q_self->setType(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 10:
    if (context->argumentCount() == 1) {
        QString _q_arg0 = context->argument(0).toString();
        _q_self->setUser(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 11:
    if (context->argumentCount() == 0) {
        QNetworkProxy::ProxyType _q_result = _q_self->type();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 12:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->user();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 13: {
    QString result = QString::fromLatin1("QNetworkProxy");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QNetworkProxy_throw_ambiguity_error_helper(context,
        qtscript_QNetworkProxy_function_names[_id+3],
        qtscript_QNetworkProxy_function_signatures[_id+3]);
}

static QScriptValue qtscript_QNetworkProxy_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QNetworkProxy(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QNetworkProxy _q_cpp_result;
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
        return _q_result;
    } else if (context->argumentCount() == 1) {
        if ((qMetaTypeId<QNetworkProxy::ProxyType>() == context->argument(0).toVariant().userType())) {
            QNetworkProxy::ProxyType _q_arg0 = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->argument(0));
            QNetworkProxy _q_cpp_result(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
            return _q_result;
        } else if ((qMetaTypeId<QNetworkProxy>() == context->argument(0).toVariant().userType())) {
            QNetworkProxy _q_arg0 = qscriptvalue_cast<QNetworkProxy>(context->argument(0));
            QNetworkProxy _q_cpp_result(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
            return _q_result;
        }
    } else if (context->argumentCount() == 2) {
        QNetworkProxy::ProxyType _q_arg0 = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        QNetworkProxy _q_cpp_result(_q_arg0, _q_arg1);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
        return _q_result;
    } else if (context->argumentCount() == 3) {
        QNetworkProxy::ProxyType _q_arg0 = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        unsigned short _q_arg2 = qscriptvalue_cast<unsigned short>(context->argument(2));
        QNetworkProxy _q_cpp_result(_q_arg0, _q_arg1, _q_arg2);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
        return _q_result;
    } else if (context->argumentCount() == 4) {
        QNetworkProxy::ProxyType _q_arg0 = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        unsigned short _q_arg2 = qscriptvalue_cast<unsigned short>(context->argument(2));
        QString _q_arg3 = context->argument(3).toString();
        QNetworkProxy _q_cpp_result(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
        return _q_result;
    } else if (context->argumentCount() == 5) {
        QNetworkProxy::ProxyType _q_arg0 = qscriptvalue_cast<QNetworkProxy::ProxyType>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        unsigned short _q_arg2 = qscriptvalue_cast<unsigned short>(context->argument(2));
        QString _q_arg3 = context->argument(3).toString();
        QString _q_arg4 = context->argument(4).toString();
        QNetworkProxy _q_cpp_result(_q_arg0, _q_arg1, _q_arg2, _q_arg3, _q_arg4);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
        return _q_result;
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        QNetworkProxy _q_result = QNetworkProxy::applicationProxy();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 1) {
        QNetworkProxy _q_arg0 = qscriptvalue_cast<QNetworkProxy>(context->argument(0));
        QNetworkProxy::setApplicationProxy(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QNetworkProxy_throw_ambiguity_error_helper(context,
        qtscript_QNetworkProxy_function_names[_id],
        qtscript_QNetworkProxy_function_signatures[_id]);
}

QScriptValue qtscript_create_QNetworkProxy_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        5
        // static
        , 0
        , 1
        // prototype
        , 0
        , 0
        , 0
        , 1
        , 0
        , 0
        , 1
        , 1
        , 1
        , 1
        , 1
        , 0
        , 0
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QNetworkProxy*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QNetworkProxy*)0));
    for (int i = 0; i < 14; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QNetworkProxy_prototype_call, function_lengths[i+3]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QNetworkProxy_function_names[i+3]),
            fun, QScriptValue::SkipInEnumeration);
    }

    engine->setDefaultPrototype(qMetaTypeId<QNetworkProxy>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<QNetworkProxy*>(), proto);

    QScriptValue ctor = engine->newFunction(qtscript_QNetworkProxy_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));
    for (int i = 0; i < 2; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QNetworkProxy_static_call,
            function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i+1)));
        ctor.setProperty(QString::fromLatin1(qtscript_QNetworkProxy_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    ctor.setProperty(QString::fromLatin1("ProxyType"),
        qtscript_create_QNetworkProxy_ProxyType_class(engine, ctor));
    return ctor;
}
