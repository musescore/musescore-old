#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qinputdialog.h>
#include <QVariant>
#include <qaction.h>
#include <qbitmap.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qevent.h>
#include <qfont.h>
#include <qicon.h>
#include <qinputcontext.h>
#include <qinputdialog.h>
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
#include <qstringlist.h>
#include <qstyle.h>
#include <qwidget.h>

static const char * const qtscript_QInputDialog_function_names[] = {
    "QInputDialog"
    // static
    , "getDouble"
    , "getInteger"
    , "getItem"
    , "getText"
    // prototype
    , "toString"
};

static const char * const qtscript_QInputDialog_function_signatures[] = {
    ""
    // static
    , "QWidget parent, String title, String label, double value, double minValue, double maxValue, int decimals, WindowFlags f"
    , "QWidget parent, String title, String label, int value, int minValue, int maxValue, int step, WindowFlags f"
    , "QWidget parent, String title, String label, List list, int current, bool editable, WindowFlags f"
    , "QWidget parent, String title, String label, EchoMode echo, String text, WindowFlags f"
    // prototype
""
};

static QScriptValue qtscript_QInputDialog_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QInputDialog*)
Q_DECLARE_METATYPE(QFlags<Qt::WindowType>)
Q_DECLARE_METATYPE(QLineEdit::EchoMode)
Q_DECLARE_METATYPE(QDialog*)


    Q_DECLARE_METATYPE(QScriptValue)
    
//
// QInputDialog
//

static QScriptValue qtscript_QInputDialog_prototype_call(QScriptContext *context, QScriptEngine *)
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
    QInputDialog* _q_self = qscriptvalue_cast<QInputDialog*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QInputDialog.%0(): this object is not a QInputDialog")
            .arg(qtscript_QInputDialog_function_names[_id+1]));
    }

    switch (_id) {
    case 0: {
    QString result = QString::fromLatin1("QInputDialog");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QInputDialog_throw_ambiguity_error_helper(context,
        qtscript_QInputDialog_function_names[_id+5],
        qtscript_QInputDialog_function_signatures[_id+5]);
}

static QScriptValue qtscript_QInputDialog_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    return context->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QInputDialog cannot be constructed"));
    break;

    case 1:
    if (context->argumentCount() == 8) {
        QWidget* _q_arg0 = qscriptvalue_cast<QWidget*>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        QString _q_arg2 = context->argument(2).toString();
        double _q_arg3 = context->argument(3).toNumber();
        double _q_arg4 = context->argument(4).toNumber();
        double _q_arg5 = context->argument(5).toNumber();
        int _q_arg6 = context->argument(6).toInt32();

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg7 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                  QFlags<Qt::WindowType> _q_arg8 = qscriptvalue_cast<QFlags<Qt::WindowType> >(context->argument(7));
        double _q_result = QInputDialog::getDouble(_q_arg0, _q_arg1, _q_arg2, _q_arg3, _q_arg4, _q_arg5, _q_arg6, _q_arg7, _q_arg8);
        
          // TEMPLATE - core.convert_to_null_or_primitive - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = QScriptValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_primitive - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
    }
    break;

    case 2:
    if (context->argumentCount() == 8) {
        QWidget* _q_arg0 = qscriptvalue_cast<QWidget*>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        QString _q_arg2 = context->argument(2).toString();
        int _q_arg3 = context->argument(3).toInt32();
        int _q_arg4 = context->argument(4).toInt32();
        int _q_arg5 = context->argument(5).toInt32();
        int _q_arg6 = context->argument(6).toInt32();

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg7 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                  QFlags<Qt::WindowType> _q_arg8 = qscriptvalue_cast<QFlags<Qt::WindowType> >(context->argument(7));
        int _q_result = QInputDialog::getInteger(_q_arg0, _q_arg1, _q_arg2, _q_arg3, _q_arg4, _q_arg5, _q_arg6, _q_arg7, _q_arg8);
        
          // TEMPLATE - core.convert_to_null_or_primitive - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = QScriptValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_primitive - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
    }
    break;

    case 3:
    if (context->argumentCount() == 7) {
        QWidget* _q_arg0 = qscriptvalue_cast<QWidget*>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        QString _q_arg2 = context->argument(2).toString();
        QStringList _q_arg3;
        qScriptValueToSequence(context->argument(3), _q_arg3);
        int _q_arg4 = context->argument(4).toInt32();
        bool _q_arg5 = context->argument(5).toBoolean();

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg6 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                  QFlags<Qt::WindowType> _q_arg7 = qscriptvalue_cast<QFlags<Qt::WindowType> >(context->argument(6));
        QString _q_result = QInputDialog::getItem(_q_arg0, _q_arg1, _q_arg2, _q_arg3, _q_arg4, _q_arg5, _q_arg6, _q_arg7);
        
          // TEMPLATE - core.convert_to_null_or_primitive - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = QScriptValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_primitive - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
    }
    break;

    case 4:
    if (context->argumentCount() == 6) {
        QWidget* _q_arg0 = qscriptvalue_cast<QWidget*>(context->argument(0));
        QString _q_arg1 = context->argument(1).toString();
        QString _q_arg2 = context->argument(2).toString();
        QLineEdit::EchoMode _q_arg3 = qscriptvalue_cast<QLineEdit::EchoMode>(context->argument(3));
        QString _q_arg4 = context->argument(4).toString();

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg5 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                  QFlags<Qt::WindowType> _q_arg6 = qscriptvalue_cast<QFlags<Qt::WindowType> >(context->argument(5));
        QString _q_result = QInputDialog::getText(_q_arg0, _q_arg1, _q_arg2, _q_arg3, _q_arg4, _q_arg5, _q_arg6);
        
          // TEMPLATE - core.convert_to_null_or_primitive - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = QScriptValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_primitive - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QInputDialog_throw_ambiguity_error_helper(context,
        qtscript_QInputDialog_function_names[_id],
        qtscript_QInputDialog_function_signatures[_id]);
}

static QScriptValue qtscript_QInputDialog_toScriptValue(QScriptEngine *engine, QInputDialog* const &in)
{
    return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

static void qtscript_QInputDialog_fromScriptValue(const QScriptValue &value, QInputDialog* &out)
{
    out = qobject_cast<QInputDialog*>(value.toQObject());
}

QScriptValue qtscript_create_QInputDialog_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        0
        // static
        , 9
        , 9
        , 8
        , 7
        // prototype
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QInputDialog*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QInputDialog*)0));
    proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QDialog*>()));

    qScriptRegisterMetaType<QInputDialog*>(engine, qtscript_QInputDialog_toScriptValue, 
        qtscript_QInputDialog_fromScriptValue, proto);

    QScriptValue ctor = engine->newFunction(qtscript_QInputDialog_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));
    for (int i = 0; i < 4; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QInputDialog_static_call,
            function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i+1)));
        ctor.setProperty(QString::fromLatin1(qtscript_QInputDialog_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    return ctor;
}
