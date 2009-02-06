#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qhostaddress.h>
#include <QVariant>
#include <qdatastream.h>
#include <qhostaddress.h>

static const char * const qtscript_QHostAddress_function_names[] = {
    "QHostAddress"
    // static
    // prototype
    , "clear"
    , "isNull"
    , "equals"
    , "protocol"
    , "readFrom"
    , "scopeId"
    , "setAddress"
    , "setScopeId"
    , "toIPv4Address"
    , "toIPv6Address"
    , "toString"
    , "writeTo"
};

static const char * const qtscript_QHostAddress_function_signatures[] = {
    "\nSpecialAddress address\nQHostAddress copy\nQIPv6Address ip6Addr\nString address\nunsigned int ip4Addr"
    // static
    // prototype
    , ""
    , ""
    , "SpecialAddress address\nQHostAddress address"
    , ""
    , "QDataStream arg__1"
    , ""
    , "QIPv6Address ip6Addr\nString address\nunsigned int ip4Addr"
    , "String id"
    , ""
    , ""
    , ""
    , "QDataStream arg__1"
};

static QScriptValue qtscript_QHostAddress_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QHostAddress)
Q_DECLARE_METATYPE(QHostAddress*)
Q_DECLARE_METATYPE(QHostAddress::SpecialAddress)
Q_DECLARE_METATYPE(QAbstractSocket::NetworkLayerProtocol)
Q_DECLARE_METATYPE(QDataStream*)
Q_DECLARE_METATYPE(QIPv6Address)

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
// QHostAddress::SpecialAddress
//

static const QHostAddress::SpecialAddress qtscript_QHostAddress_SpecialAddress_values[] = {
    QHostAddress::Null
    , QHostAddress::Broadcast
    , QHostAddress::LocalHost
    , QHostAddress::LocalHostIPv6
    , QHostAddress::Any
    , QHostAddress::AnyIPv6
};

static const char * const qtscript_QHostAddress_SpecialAddress_keys[] = {
    "Null"
    , "Broadcast"
    , "LocalHost"
    , "LocalHostIPv6"
    , "Any"
    , "AnyIPv6"
};

static QString qtscript_QHostAddress_SpecialAddress_toStringHelper(QHostAddress::SpecialAddress value)
{
    if ((value >= QHostAddress::Null) && (value <= QHostAddress::AnyIPv6))
        return qtscript_QHostAddress_SpecialAddress_keys[static_cast<int>(value)];
    return QString();
}

static QScriptValue qtscript_QHostAddress_SpecialAddress_toScriptValue(QScriptEngine *engine, const QHostAddress::SpecialAddress &value)
{
    QScriptValue clazz = engine->globalObject().property(QString::fromLatin1("QHostAddress"));
    return clazz.property(qtscript_QHostAddress_SpecialAddress_toStringHelper(value));
}

static void qtscript_QHostAddress_SpecialAddress_fromScriptValue(const QScriptValue &value, QHostAddress::SpecialAddress &out)
{
    out = qvariant_cast<QHostAddress::SpecialAddress>(value.toVariant());
}

static QScriptValue qtscript_construct_QHostAddress_SpecialAddress(QScriptContext *context, QScriptEngine *engine)
{
    int arg = context->argument(0).toInt32();
    if ((arg >= QHostAddress::Null) && (arg <= QHostAddress::AnyIPv6))
        return qScriptValueFromValue(engine,  static_cast<QHostAddress::SpecialAddress>(arg));
    return context->throwError(QString::fromLatin1("SpecialAddress(): invalid enum value (%0)").arg(arg));
}

static QScriptValue qtscript_QHostAddress_SpecialAddress_valueOf(QScriptContext *context, QScriptEngine *engine)
{
    QHostAddress::SpecialAddress value = qscriptvalue_cast<QHostAddress::SpecialAddress>(context->thisObject());
    return QScriptValue(engine, static_cast<int>(value));
}

static QScriptValue qtscript_QHostAddress_SpecialAddress_toString(QScriptContext *context, QScriptEngine *engine)
{
    QHostAddress::SpecialAddress value = qscriptvalue_cast<QHostAddress::SpecialAddress>(context->thisObject());
    return QScriptValue(engine, qtscript_QHostAddress_SpecialAddress_toStringHelper(value));
}

static QScriptValue qtscript_create_QHostAddress_SpecialAddress_class(QScriptEngine *engine, QScriptValue &clazz)
{
    QScriptValue ctor = qtscript_create_enum_class_helper(
        engine, qtscript_construct_QHostAddress_SpecialAddress,
        qtscript_QHostAddress_SpecialAddress_valueOf, qtscript_QHostAddress_SpecialAddress_toString);
    qScriptRegisterMetaType<QHostAddress::SpecialAddress>(engine, qtscript_QHostAddress_SpecialAddress_toScriptValue,
        qtscript_QHostAddress_SpecialAddress_fromScriptValue, ctor.property(QString::fromLatin1("prototype")));
    for (int i = 0; i < 6; ++i) {
        clazz.setProperty(QString::fromLatin1(qtscript_QHostAddress_SpecialAddress_keys[i]),
            engine->newVariant(qVariantFromValue(qtscript_QHostAddress_SpecialAddress_values[i])),
            QScriptValue::ReadOnly | QScriptValue::Undeletable);
    }
    return ctor;
}

//
// QHostAddress
//

static QScriptValue qtscript_QHostAddress_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 12;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QHostAddress* _q_self = qscriptvalue_cast<QHostAddress*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QHostAddress.%0(): this object is not a QHostAddress")
            .arg(qtscript_QHostAddress_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 0) {
        _q_self->clear();
        return context->engine()->undefinedValue();
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        bool _q_result = _q_self->isNull();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 1) {
        if ((qMetaTypeId<QHostAddress::SpecialAddress>() == context->argument(0).toVariant().userType())) {
            QHostAddress::SpecialAddress _q_arg0 = qscriptvalue_cast<QHostAddress::SpecialAddress>(context->argument(0));
            bool _q_result = _q_self->operator==(_q_arg0);
            return QScriptValue(context->engine(), _q_result);
        } else if ((qMetaTypeId<QHostAddress>() == context->argument(0).toVariant().userType())) {
            QHostAddress _q_arg0 = qscriptvalue_cast<QHostAddress>(context->argument(0));
            bool _q_result = _q_self->operator==(_q_arg0);
            return QScriptValue(context->engine(), _q_result);
        }
    }
    break;

    case 3:
    if (context->argumentCount() == 0) {
        QAbstractSocket::NetworkLayerProtocol _q_result = _q_self->protocol();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 4:
    if (context->argumentCount() == 1) {
        QDataStream* _q_arg0 = qscriptvalue_cast<QDataStream*>(context->argument(0));
        operator>>(*_q_arg0, *_q_self);
        return context->engine()->undefinedValue();
    }
    break;

    case 5:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->scopeId();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 6:
    if (context->argumentCount() == 1) {
        if ((qMetaTypeId<QIPv6Address>() == context->argument(0).toVariant().userType())) {
            QIPv6Address _q_arg0 = qscriptvalue_cast<QIPv6Address>(context->argument(0));
            _q_self->setAddress(_q_arg0);
            return context->engine()->undefinedValue();
        } else if (context->argument(0).isString()) {
            QString _q_arg0 = context->argument(0).toString();
            bool _q_result = _q_self->setAddress(_q_arg0);
            return QScriptValue(context->engine(), _q_result);
        } else if (context->argument(0).isNumber()) {
            uint _q_arg0 = context->argument(0).toUInt32();
            _q_self->setAddress(_q_arg0);
            return context->engine()->undefinedValue();
        }
    }
    break;

    case 7:
    if (context->argumentCount() == 1) {
        QString _q_arg0 = context->argument(0).toString();
        _q_self->setScopeId(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 8:
    if (context->argumentCount() == 0) {
        uint _q_result = _q_self->toIPv4Address();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 9:
    if (context->argumentCount() == 0) {
        QIPv6Address _q_result = _q_self->toIPv6Address();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 10:
    if (context->argumentCount() == 0) {
        QString _q_result = _q_self->toString();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 11:
    if (context->argumentCount() == 1) {
        QDataStream* _q_arg0 = qscriptvalue_cast<QDataStream*>(context->argument(0));
        operator<<(*_q_arg0, *_q_self);
        return context->engine()->undefinedValue();
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QHostAddress_throw_ambiguity_error_helper(context,
        qtscript_QHostAddress_function_names[_id+1],
        qtscript_QHostAddress_function_signatures[_id+1]);
}

static QScriptValue qtscript_QHostAddress_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QHostAddress(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QHostAddress _q_cpp_result;
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
        return _q_result;
    } else if (context->argumentCount() == 1) {
        if ((qMetaTypeId<QHostAddress::SpecialAddress>() == context->argument(0).toVariant().userType())) {
            QHostAddress::SpecialAddress _q_arg0 = qscriptvalue_cast<QHostAddress::SpecialAddress>(context->argument(0));
            QHostAddress _q_cpp_result(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
            return _q_result;
        } else if ((qMetaTypeId<QHostAddress>() == context->argument(0).toVariant().userType())) {
            QHostAddress _q_arg0 = qscriptvalue_cast<QHostAddress>(context->argument(0));
            QHostAddress _q_cpp_result(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
            return _q_result;
        } else if ((qMetaTypeId<QIPv6Address>() == context->argument(0).toVariant().userType())) {
            QIPv6Address _q_arg0 = qscriptvalue_cast<QIPv6Address>(context->argument(0));
            QHostAddress _q_cpp_result(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
            return _q_result;
        } else if (context->argument(0).isString()) {
            QString _q_arg0 = context->argument(0).toString();
            QHostAddress _q_cpp_result(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
            return _q_result;
        } else if (context->argument(0).isNumber()) {
            uint _q_arg0 = context->argument(0).toUInt32();
            QHostAddress _q_cpp_result(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue(_q_cpp_result));
            return _q_result;
        }
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QHostAddress_throw_ambiguity_error_helper(context,
        qtscript_QHostAddress_function_names[_id],
        qtscript_QHostAddress_function_signatures[_id]);
}

QScriptValue qtscript_create_QHostAddress_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        1
        // static
        // prototype
        , 0
        , 0
        , 1
        , 0
        , 1
        , 0
        , 1
        , 1
        , 0
        , 0
        , 0
        , 1
    };
    engine->setDefaultPrototype(qMetaTypeId<QHostAddress*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QHostAddress*)0));
    for (int i = 0; i < 12; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QHostAddress_prototype_call, function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QHostAddress_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    engine->setDefaultPrototype(qMetaTypeId<QHostAddress>(), proto);
    engine->setDefaultPrototype(qMetaTypeId<QHostAddress*>(), proto);

    QScriptValue ctor = engine->newFunction(qtscript_QHostAddress_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));

    ctor.setProperty(QString::fromLatin1("SpecialAddress"),
        qtscript_create_QHostAddress_SpecialAddress_class(engine, ctor));
    return ctor;
}
