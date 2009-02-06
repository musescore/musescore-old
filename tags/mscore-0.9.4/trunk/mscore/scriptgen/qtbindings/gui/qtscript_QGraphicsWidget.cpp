#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qgraphicswidget.h>
#include <QVariant>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qevent.h>
#include <qfont.h>
#include <qgraphicsitem.h>
#include <qgraphicslayout.h>
#include <qgraphicslayoutitem.h>
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicswidget.h>
#include <qlist.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qpalette.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>
#include <qregion.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtransform.h>
#include <qwidget.h>

#include "qtscriptshell_QGraphicsWidget.h"

static const char * const qtscript_QGraphicsWidget_function_names[] = {
    "QGraphicsWidget"
    // static
    , "setTabOrder"
    // prototype
    , "adjustSize"
    , "focusWidget"
    , "getWindowFrameMargins"
    , "isActiveWindow"
    , "layout"
    , "paintWindowFrame"
    , "rect"
    , "resize"
    , "setAttribute"
    , "setContentsMargins"
    , "setGeometry"
    , "setLayout"
    , "setStyle"
    , "setWindowFrameMargins"
    , "style"
    , "testAttribute"
    , "unsetLayoutDirection"
    , "unsetWindowFrameMargins"
    , "windowFrameGeometry"
    , "windowFrameRect"
    , "windowType"
    , "toString"
};

static const char * const qtscript_QGraphicsWidget_function_signatures[] = {
    "QGraphicsItem parent, WindowFlags wFlags"
    // static
    , "QGraphicsWidget first, QGraphicsWidget second"
    // prototype
    , ""
    , ""
    , "qreal left, qreal top, qreal right, qreal bottom"
    , ""
    , ""
    , "QPainter painter, QStyleOptionGraphicsItem option, QWidget widget"
    , ""
    , "qreal w, qreal h"
    , "WidgetAttribute attribute, bool on"
    , "qreal left, qreal top, qreal right, qreal bottom"
    , "qreal x, qreal y, qreal w, qreal h"
    , "QGraphicsLayout layout"
    , "QStyle style"
    , "qreal left, qreal top, qreal right, qreal bottom"
    , ""
    , "WidgetAttribute attribute"
    , ""
    , ""
    , ""
    , ""
    , ""
""
};

static QScriptValue qtscript_QGraphicsWidget_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QtScriptShell_QGraphicsWidget*)
Q_DECLARE_METATYPE(qreal*)
Q_DECLARE_METATYPE(QGraphicsLayout*)
Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QStyleOptionGraphicsItem*)
Q_DECLARE_METATYPE(Qt::WidgetAttribute)
Q_DECLARE_METATYPE(QStyle*)
Q_DECLARE_METATYPE(Qt::WindowType)
Q_DECLARE_METATYPE(QFlags<Qt::WindowType>)
Q_DECLARE_METATYPE(QGraphicsLayoutItem*)

//
// QGraphicsWidget
//

static QScriptValue qtscript_QGraphicsWidget_prototype_call(QScriptContext *context, QScriptEngine *)
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
    QGraphicsWidget* _q_self = qscriptvalue_cast<QGraphicsWidget*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QGraphicsWidget.%0(): this object is not a QGraphicsWidget")
            .arg(qtscript_QGraphicsWidget_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 0) {
        _q_self->adjustSize();
        return context->engine()->undefinedValue();
    }
    break;

    case 1:
    if (context->argumentCount() == 0) {
        QGraphicsWidget* _q_result = _q_self->focusWidget();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 4) {
        qreal* _q_arg0 = qscriptvalue_cast<qreal*>(context->argument(0));
        qreal* _q_arg1 = qscriptvalue_cast<qreal*>(context->argument(1));
        qreal* _q_arg2 = qscriptvalue_cast<qreal*>(context->argument(2));
        qreal* _q_arg3 = qscriptvalue_cast<qreal*>(context->argument(3));
        _q_self->getWindowFrameMargins(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        return context->engine()->undefinedValue();
    }
    break;

    case 3:
    if (context->argumentCount() == 0) {
        bool _q_result = _q_self->isActiveWindow();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 4:
    if (context->argumentCount() == 0) {
        QGraphicsLayout* _q_result = _q_self->layout();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 5:
    if (context->argumentCount() == 2) {
        QPainter* _q_arg0 = qscriptvalue_cast<QPainter*>(context->argument(0));
        QStyleOptionGraphicsItem* _q_arg1 = qscriptvalue_cast<QStyleOptionGraphicsItem*>(context->argument(1));
        _q_self->paintWindowFrame(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 3) {
        QPainter* _q_arg0 = qscriptvalue_cast<QPainter*>(context->argument(0));
        QStyleOptionGraphicsItem* _q_arg1 = qscriptvalue_cast<QStyleOptionGraphicsItem*>(context->argument(1));
        QWidget* _q_arg2 = qscriptvalue_cast<QWidget*>(context->argument(2));
        _q_self->paintWindowFrame(_q_arg0, _q_arg1, _q_arg2);
        return context->engine()->undefinedValue();
    }
    break;

    case 6:
    if (context->argumentCount() == 0) {
        QRectF _q_result = _q_self->rect();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 7:
    if (context->argumentCount() == 2) {
        qreal _q_arg0 = qscriptvalue_cast<qreal>(context->argument(0));
        qreal _q_arg1 = qscriptvalue_cast<qreal>(context->argument(1));
        _q_self->resize(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 8:
    if (context->argumentCount() == 1) {
        Qt::WidgetAttribute _q_arg0 = qscriptvalue_cast<Qt::WidgetAttribute>(context->argument(0));
        _q_self->setAttribute(_q_arg0);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 2) {
        Qt::WidgetAttribute _q_arg0 = qscriptvalue_cast<Qt::WidgetAttribute>(context->argument(0));
        bool _q_arg1 = context->argument(1).toBoolean();
        _q_self->setAttribute(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 9:
    if (context->argumentCount() == 4) {
        qreal _q_arg0 = qscriptvalue_cast<qreal>(context->argument(0));
        qreal _q_arg1 = qscriptvalue_cast<qreal>(context->argument(1));
        qreal _q_arg2 = qscriptvalue_cast<qreal>(context->argument(2));
        qreal _q_arg3 = qscriptvalue_cast<qreal>(context->argument(3));
        _q_self->setContentsMargins(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        return context->engine()->undefinedValue();
    }
    break;

    case 10:
    if (context->argumentCount() == 4) {
        qreal _q_arg0 = qscriptvalue_cast<qreal>(context->argument(0));
        qreal _q_arg1 = qscriptvalue_cast<qreal>(context->argument(1));
        qreal _q_arg2 = qscriptvalue_cast<qreal>(context->argument(2));
        qreal _q_arg3 = qscriptvalue_cast<qreal>(context->argument(3));
        _q_self->setGeometry(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        return context->engine()->undefinedValue();
    }
    break;

    case 11:
    if (context->argumentCount() == 1) {
        QGraphicsLayout* _q_arg0 = qscriptvalue_cast<QGraphicsLayout*>(context->argument(0));
        _q_self->setLayout(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 12:
    if (context->argumentCount() == 1) {
        QStyle* _q_arg0 = qscriptvalue_cast<QStyle*>(context->argument(0));
        _q_self->setStyle(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 13:
    if (context->argumentCount() == 4) {
        qreal _q_arg0 = qscriptvalue_cast<qreal>(context->argument(0));
        qreal _q_arg1 = qscriptvalue_cast<qreal>(context->argument(1));
        qreal _q_arg2 = qscriptvalue_cast<qreal>(context->argument(2));
        qreal _q_arg3 = qscriptvalue_cast<qreal>(context->argument(3));
        _q_self->setWindowFrameMargins(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        return context->engine()->undefinedValue();
    }
    break;

    case 14:
    if (context->argumentCount() == 0) {
        QStyle* _q_result = _q_self->style();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 15:
    if (context->argumentCount() == 1) {
        Qt::WidgetAttribute _q_arg0 = qscriptvalue_cast<Qt::WidgetAttribute>(context->argument(0));
        bool _q_result = _q_self->testAttribute(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 16:
    if (context->argumentCount() == 0) {
        _q_self->unsetLayoutDirection();
        return context->engine()->undefinedValue();
    }
    break;

    case 17:
    if (context->argumentCount() == 0) {
        _q_self->unsetWindowFrameMargins();
        return context->engine()->undefinedValue();
    }
    break;

    case 18:
    if (context->argumentCount() == 0) {
        QRectF _q_result = _q_self->windowFrameGeometry();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 19:
    if (context->argumentCount() == 0) {
        QRectF _q_result = _q_self->windowFrameRect();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 20:
    if (context->argumentCount() == 0) {
        Qt::WindowType _q_result = _q_self->windowType();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 21: {
    QString result = QString::fromLatin1("QGraphicsWidget");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QGraphicsWidget_throw_ambiguity_error_helper(context,
        qtscript_QGraphicsWidget_function_names[_id+2],
        qtscript_QGraphicsWidget_function_signatures[_id+2]);
}

static QScriptValue qtscript_QGraphicsWidget_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QGraphicsWidget(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QtScriptShell_QGraphicsWidget* _q_cpp_result = new QtScriptShell_QGraphicsWidget();
        QScriptValue _q_result = context->engine()->newQObject(context->thisObject(), (QGraphicsWidget*)_q_cpp_result, QScriptEngine::AutoOwnership);
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    } else if (context->argumentCount() == 1) {
        QGraphicsItem* _q_arg0 = qscriptvalue_cast<QGraphicsItem*>(context->argument(0));
        QtScriptShell_QGraphicsWidget* _q_cpp_result = new QtScriptShell_QGraphicsWidget(_q_arg0);
        QScriptValue _q_result = context->engine()->newQObject(context->thisObject(), (QGraphicsWidget*)_q_cpp_result, QScriptEngine::AutoOwnership);
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    } else if (context->argumentCount() == 2) {
        QGraphicsItem* _q_arg0 = qscriptvalue_cast<QGraphicsItem*>(context->argument(0));
        QFlags<Qt::WindowType> _q_arg1 = qscriptvalue_cast<QFlags<Qt::WindowType> >(context->argument(1));
        QtScriptShell_QGraphicsWidget* _q_cpp_result = new QtScriptShell_QGraphicsWidget(_q_arg0, _q_arg1);
        QScriptValue _q_result = context->engine()->newQObject(context->thisObject(), (QGraphicsWidget*)_q_cpp_result, QScriptEngine::AutoOwnership);
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    }
    break;

    case 1:
    if (context->argumentCount() == 2) {
        QGraphicsWidget* _q_arg0 = qscriptvalue_cast<QGraphicsWidget*>(context->argument(0));
        QGraphicsWidget* _q_arg1 = qscriptvalue_cast<QGraphicsWidget*>(context->argument(1));
        QGraphicsWidget::setTabOrder(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QGraphicsWidget_throw_ambiguity_error_helper(context,
        qtscript_QGraphicsWidget_function_names[_id],
        qtscript_QGraphicsWidget_function_signatures[_id]);
}

static QScriptValue qtscript_QGraphicsWidget_toScriptValue(QScriptEngine *engine, QGraphicsWidget* const &in)
{
    return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

static void qtscript_QGraphicsWidget_fromScriptValue(const QScriptValue &value, QGraphicsWidget* &out)
{
    out = qobject_cast<QGraphicsWidget*>(value.toQObject());
}

QScriptValue qtscript_create_QGraphicsWidget_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        2
        // static
        , 2
        // prototype
        , 0
        , 0
        , 4
        , 0
        , 0
        , 3
        , 0
        , 2
        , 2
        , 4
        , 4
        , 1
        , 1
        , 4
        , 0
        , 1
        , 0
        , 0
        , 0
        , 0
        , 0
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QGraphicsWidget*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QGraphicsWidget*)0));
    proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QObject*>()));
    proto.setProperty(QString::fromLatin1("__QGraphicsItem__"),
        engine->defaultPrototype(qMetaTypeId<QGraphicsItem*>()),
        QScriptValue::SkipInEnumeration);
    proto.setProperty(QString::fromLatin1("__QGraphicsLayoutItem__"),
        engine->defaultPrototype(qMetaTypeId<QGraphicsLayoutItem*>()),
        QScriptValue::SkipInEnumeration);
    for (int i = 0; i < 22; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QGraphicsWidget_prototype_call, function_lengths[i+2]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QGraphicsWidget_function_names[i+2]),
            fun, QScriptValue::SkipInEnumeration);
    }

    qScriptRegisterMetaType<QGraphicsWidget*>(engine, qtscript_QGraphicsWidget_toScriptValue, 
        qtscript_QGraphicsWidget_fromScriptValue, proto);

    QScriptValue ctor = engine->newFunction(qtscript_QGraphicsWidget_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));
    for (int i = 0; i < 1; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QGraphicsWidget_static_call,
            function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i+1)));
        ctor.setProperty(QString::fromLatin1(qtscript_QGraphicsWidget_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    return ctor;
}
