#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qcolordialog.h>
#include <QVariant>
#include <qaction.h>
#include <qbitmap.h>
#include <qbytearray.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qevent.h>
#include <qfont.h>
#include <qicon.h>
#include <qinputcontext.h>
#include <qkeysequence.h>
#include <qlayout.h>
#include <qlist.h>
#include <qlocale.h>
#include <qobject.h>
#include <qpaintdevice.h>
#include <qpaintengine.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpoint.h>
#include <qrect.h>
#include <qregion.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qstyle.h>
#include <qwidget.h>

static const char * const qtscript_QColorDialog_function_names[] = {
    "QColorDialog"
    // static
    , "customColor"
    , "customCount"
    , "getColor"
    , "setCustomColor"
    , "setStandardColor"
    // prototype
    , "toString"
};

static const char * const qtscript_QColorDialog_function_signatures[] = {
    ""
    // static
    , "int arg__1"
    , ""
    , "QColor init, QWidget parent"
    , "int arg__1, unsigned int arg__2"
    , "int arg__1, unsigned int arg__2"
    // prototype
""
};

static QScriptValue qtscript_QColorDialog_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QColorDialog*)
Q_DECLARE_METATYPE(QDialog*)

//
// QColorDialog
//

static QScriptValue qtscript_QColorDialog_prototype_call(QScriptContext *context, QScriptEngine *)
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
    QColorDialog* _q_self = qscriptvalue_cast<QColorDialog*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QColorDialog.%0(): this object is not a QColorDialog")
            .arg(qtscript_QColorDialog_function_names[_id+1]));
    }

    switch (_id) {
    case 0: {
    QString result = QString::fromLatin1("QColorDialog");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QColorDialog_throw_ambiguity_error_helper(context,
        qtscript_QColorDialog_function_names[_id+6],
        qtscript_QColorDialog_function_signatures[_id+6]);
}

static QScriptValue qtscript_QColorDialog_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    return context->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QColorDialog cannot be constructed"));
    break;

    case 1:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        uint _q_result = QColorDialog::customColor(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 2:
    if (context->argumentCount() == 0) {
        int _q_result = QColorDialog::customCount();
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 3:
    if (context->argumentCount() == 0) {
        QColor _q_result = QColorDialog::getColor();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 1) {
        QColor _q_arg0 = qscriptvalue_cast<QColor>(context->argument(0));
        QColor _q_result = QColorDialog::getColor(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 2) {
        QColor _q_arg0 = qscriptvalue_cast<QColor>(context->argument(0));
        QWidget* _q_arg1 = qscriptvalue_cast<QWidget*>(context->argument(1));
        QColor _q_result = QColorDialog::getColor(_q_arg0, _q_arg1);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 4:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        uint _q_arg1 = context->argument(1).toUInt32();
        QColorDialog::setCustomColor(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 5:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        uint _q_arg1 = context->argument(1).toUInt32();
        QColorDialog::setStandardColor(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QColorDialog_throw_ambiguity_error_helper(context,
        qtscript_QColorDialog_function_names[_id],
        qtscript_QColorDialog_function_signatures[_id]);
}

static QScriptValue qtscript_QColorDialog_toScriptValue(QScriptEngine *engine, QColorDialog* const &in)
{
    return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

static void qtscript_QColorDialog_fromScriptValue(const QScriptValue &value, QColorDialog* &out)
{
    out = qobject_cast<QColorDialog*>(value.toQObject());
}

QScriptValue qtscript_create_QColorDialog_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        0
        // static
        , 1
        , 0
        , 2
        , 2
        , 2
        // prototype
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QColorDialog*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QColorDialog*)0));
    proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QDialog*>()));

    qScriptRegisterMetaType<QColorDialog*>(engine, qtscript_QColorDialog_toScriptValue, 
        qtscript_QColorDialog_fromScriptValue, proto);

    QScriptValue ctor = engine->newFunction(qtscript_QColorDialog_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));
    for (int i = 0; i < 5; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QColorDialog_static_call,
            function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i+1)));
        ctor.setProperty(QString::fromLatin1(qtscript_QColorDialog_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    return ctor;
}
