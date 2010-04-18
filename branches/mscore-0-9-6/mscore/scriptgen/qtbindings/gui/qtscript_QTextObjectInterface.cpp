#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qabstracttextdocumentlayout.h>
#include <QVariant>
#include <qpainter.h>
#include <qrect.h>
#include <qsize.h>
#include <qtextdocument.h>
#include <qtextformat.h>

#include "qtscriptshell_QTextObjectInterface.h"

static const char * const qtscript_QTextObjectInterface_function_names[] = {
    "QTextObjectInterface"
    // static
    // prototype
    , "drawObject"
    , "intrinsicSize"
    , "toString"
};

static const char * const qtscript_QTextObjectInterface_function_signatures[] = {
    ""
    // static
    // prototype
    , "QPainter painter, QRectF rect, QTextDocument doc, int posInDocument, QTextFormat format"
    , "QTextDocument doc, int posInDocument, QTextFormat format"
""
};

static QScriptValue qtscript_QTextObjectInterface_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QTextObjectInterface::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QTextObjectInterface*)
Q_DECLARE_METATYPE(QtScriptShell_QTextObjectInterface*)
Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QTextDocument*)

//
// QTextObjectInterface
//

static QScriptValue qtscript_QTextObjectInterface_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 2;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QTextObjectInterface* _q_self = qscriptvalue_cast<QTextObjectInterface*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QTextObjectInterface.%0(): this object is not a QTextObjectInterface")
            .arg(qtscript_QTextObjectInterface_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 5) {
        QPainter* _q_arg0 = qscriptvalue_cast<QPainter*>(context->argument(0));
        QRectF _q_arg1 = qscriptvalue_cast<QRectF>(context->argument(1));
        QTextDocument* _q_arg2 = qscriptvalue_cast<QTextDocument*>(context->argument(2));
        int _q_arg3 = context->argument(3).toInt32();
        QTextFormat _q_arg4 = qscriptvalue_cast<QTextFormat>(context->argument(4));
        _q_self->drawObject(_q_arg0, _q_arg1, _q_arg2, _q_arg3, _q_arg4);
        return context->engine()->undefinedValue();
    }
    break;

    case 1:
    if (context->argumentCount() == 3) {
        QTextDocument* _q_arg0 = qscriptvalue_cast<QTextDocument*>(context->argument(0));
        int _q_arg1 = context->argument(1).toInt32();
        QTextFormat _q_arg2 = qscriptvalue_cast<QTextFormat>(context->argument(2));
        QSizeF _q_result = _q_self->intrinsicSize(_q_arg0, _q_arg1, _q_arg2);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 2: {
    QString result = QString::fromLatin1("QTextObjectInterface");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QTextObjectInterface_throw_ambiguity_error_helper(context,
        qtscript_QTextObjectInterface_function_names[_id+1],
        qtscript_QTextObjectInterface_function_signatures[_id+1]);
}

static QScriptValue qtscript_QTextObjectInterface_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QTextObjectInterface(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QtScriptShell_QTextObjectInterface* _q_cpp_result = new QtScriptShell_QTextObjectInterface();
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QTextObjectInterface*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QTextObjectInterface_throw_ambiguity_error_helper(context,
        qtscript_QTextObjectInterface_function_names[_id],
        qtscript_QTextObjectInterface_function_signatures[_id]);
}

QScriptValue qtscript_create_QTextObjectInterface_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        0
        // static
        // prototype
        , 5
        , 3
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QTextObjectInterface*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QTextObjectInterface*)0));
    for (int i = 0; i < 3; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QTextObjectInterface_prototype_call, function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QTextObjectInterface_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    engine->setDefaultPrototype(qMetaTypeId<QTextObjectInterface*>(), proto);

    QScriptValue ctor = engine->newFunction(qtscript_QTextObjectInterface_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));

    return ctor;
}
