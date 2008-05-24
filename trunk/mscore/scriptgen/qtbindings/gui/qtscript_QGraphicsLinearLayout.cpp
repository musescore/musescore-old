#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qgraphicslinearlayout.h>
#include <QVariant>
#include <qcoreevent.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicslinearlayout.h>
#include <qrect.h>
#include <qsize.h>
#include <qsizepolicy.h>

#include "qtscriptshell_QGraphicsLinearLayout.h"

static const char * const qtscript_QGraphicsLinearLayout_function_names[] = {
    "QGraphicsLinearLayout"
    // static
    // prototype
    , "addItem"
    , "addStretch"
    , "alignment"
    , "insertItem"
    , "insertStretch"
    , "itemSpacing"
    , "removeItem"
    , "setAlignment"
    , "setItemSpacing"
    , "setStretchFactor"
    , "stretchFactor"
    , "toString"
};

static const char * const qtscript_QGraphicsLinearLayout_function_signatures[] = {
    "QGraphicsLayoutItem parent\nOrientation orientation, QGraphicsLayoutItem parent"
    // static
    // prototype
    , "QGraphicsLayoutItem item"
    , "int stretch"
    , "QGraphicsLayoutItem item"
    , "int index, QGraphicsLayoutItem item"
    , "int index, int stretch"
    , "int index"
    , "QGraphicsLayoutItem item"
    , "QGraphicsLayoutItem item, Alignment alignment"
    , "int index, qreal spacing"
    , "QGraphicsLayoutItem item, int stretch"
    , "QGraphicsLayoutItem item"
""
};

static QScriptValue qtscript_QGraphicsLinearLayout_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QGraphicsLinearLayout*)
Q_DECLARE_METATYPE(QtScriptShell_QGraphicsLinearLayout*)
Q_DECLARE_METATYPE(QGraphicsLayoutItem*)
Q_DECLARE_METATYPE(QFlags<Qt::AlignmentFlag>)
Q_DECLARE_METATYPE(Qt::Orientation)
Q_DECLARE_METATYPE(QGraphicsLayout*)

//
// QGraphicsLinearLayout
//

static QScriptValue qtscript_QGraphicsLinearLayout_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 11;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QGraphicsLinearLayout* _q_self = qscriptvalue_cast<QGraphicsLinearLayout*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QGraphicsLinearLayout.%0(): this object is not a QGraphicsLinearLayout")
            .arg(qtscript_QGraphicsLinearLayout_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 1) {
        QGraphicsLayoutItem* _q_arg0 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0));
        _q_self->addItem(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        _q_self->addStretch();
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        _q_self->addStretch(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 2:
    if (context->argumentCount() == 1) {
        QGraphicsLayoutItem* _q_arg0 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0));
        QFlags<Qt::AlignmentFlag> _q_result = _q_self->alignment(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 3:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        QGraphicsLayoutItem* _q_arg1 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(1));
        _q_self->insertItem(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 4:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        _q_self->insertStretch(_q_arg0);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        _q_self->insertStretch(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 5:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        qreal _q_result = _q_self->itemSpacing(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 6:
    if (context->argumentCount() == 1) {
        QGraphicsLayoutItem* _q_arg0 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0));
        _q_self->removeItem(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 7:
    if (context->argumentCount() == 2) {
        QGraphicsLayoutItem* _q_arg0 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0));
        QFlags<Qt::AlignmentFlag> _q_arg1 = qscriptvalue_cast<QFlags<Qt::AlignmentFlag> >(context->argument(1));
        _q_self->setAlignment(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 8:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        qreal _q_arg1 = qscriptvalue_cast<qreal>(context->argument(1));
        _q_self->setItemSpacing(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 9:
    if (context->argumentCount() == 2) {
        QGraphicsLayoutItem* _q_arg0 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0));
        int _q_arg1 = context->argument(1).toInt32();
        _q_self->setStretchFactor(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 10:
    if (context->argumentCount() == 1) {
        QGraphicsLayoutItem* _q_arg0 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0));
        int _q_result = _q_self->stretchFactor(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 11: {
    QString result = QString::fromLatin1("QGraphicsLinearLayout");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QGraphicsLinearLayout_throw_ambiguity_error_helper(context,
        qtscript_QGraphicsLinearLayout_function_names[_id+1],
        qtscript_QGraphicsLinearLayout_function_signatures[_id+1]);
}

static QScriptValue qtscript_QGraphicsLinearLayout_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QGraphicsLinearLayout(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QtScriptShell_QGraphicsLinearLayout* _q_cpp_result = new QtScriptShell_QGraphicsLinearLayout();
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QGraphicsLinearLayout*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    } else if (context->argumentCount() == 1) {
        if (qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0))) {
            QGraphicsLayoutItem* _q_arg0 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(0));
            QtScriptShell_QGraphicsLinearLayout* _q_cpp_result = new QtScriptShell_QGraphicsLinearLayout(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QGraphicsLinearLayout*)_q_cpp_result));
            _q_cpp_result->__qtscript_self = _q_result;
            return _q_result;
        } else if ((qMetaTypeId<Qt::Orientation>() == context->argument(0).toVariant().userType())) {
            Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
            QtScriptShell_QGraphicsLinearLayout* _q_cpp_result = new QtScriptShell_QGraphicsLinearLayout(_q_arg0);
            QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QGraphicsLinearLayout*)_q_cpp_result));
            _q_cpp_result->__qtscript_self = _q_result;
            return _q_result;
        }
    } else if (context->argumentCount() == 2) {
        Qt::Orientation _q_arg0 = qscriptvalue_cast<Qt::Orientation>(context->argument(0));
        QGraphicsLayoutItem* _q_arg1 = qscriptvalue_cast<QGraphicsLayoutItem*>(context->argument(1));
        QtScriptShell_QGraphicsLinearLayout* _q_cpp_result = new QtScriptShell_QGraphicsLinearLayout(_q_arg0, _q_arg1);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QGraphicsLinearLayout*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QGraphicsLinearLayout_throw_ambiguity_error_helper(context,
        qtscript_QGraphicsLinearLayout_function_names[_id],
        qtscript_QGraphicsLinearLayout_function_signatures[_id]);
}

QScriptValue qtscript_create_QGraphicsLinearLayout_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        2
        // static
        // prototype
        , 1
        , 1
        , 1
        , 2
        , 2
        , 1
        , 1
        , 2
        , 2
        , 2
        , 1
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QGraphicsLinearLayout*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QGraphicsLinearLayout*)0));
    proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QGraphicsLayout*>()));
    for (int i = 0; i < 12; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QGraphicsLinearLayout_prototype_call, function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QGraphicsLinearLayout_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    engine->setDefaultPrototype(qMetaTypeId<QGraphicsLinearLayout*>(), proto);

    QScriptValue ctor = engine->newFunction(qtscript_QGraphicsLinearLayout_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));

    return ctor;
}
