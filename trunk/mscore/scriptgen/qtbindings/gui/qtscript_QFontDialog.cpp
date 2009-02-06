#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qfontdialog.h>
#include <QVariant>
#include <qaction.h>
#include <qbitmap.h>
#include <qbytearray.h>
#include <qcoreevent.h>
#include <qcursor.h>
#include <qevent.h>
#include <qfont.h>
#include <qfontdialog.h>
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

static const char * const qtscript_QFontDialog_function_names[] = {
    "QFontDialog"
    // static
    , "getFont"
    // prototype
    , "toString"
};

static const char * const qtscript_QFontDialog_function_signatures[] = {
    ""
    // static
    , "QWidget parent\nQFont def, QWidget parent\nQFont def, QWidget parent, String caption"
    // prototype
""
};

static QScriptValue qtscript_QFontDialog_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QFontDialog*)
Q_DECLARE_METATYPE(QDialog*)


    Q_DECLARE_METATYPE(QScriptValue)
    
//
// QFontDialog
//

static QScriptValue qtscript_QFontDialog_prototype_call(QScriptContext *context, QScriptEngine *)
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
    QFontDialog* _q_self = qscriptvalue_cast<QFontDialog*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QFontDialog.%0(): this object is not a QFontDialog")
            .arg(qtscript_QFontDialog_function_names[_id+1]));
    }

    switch (_id) {
    case 0: {
    QString result = QString::fromLatin1("QFontDialog");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QFontDialog_throw_ambiguity_error_helper(context,
        qtscript_QFontDialog_function_names[_id+2],
        qtscript_QFontDialog_function_signatures[_id+2]);
}

static QScriptValue qtscript_QFontDialog_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    return context->throwError(QScriptContext::TypeError,
        QString::fromLatin1("QFontDialog cannot be constructed"));
    break;

    case 1:
    if (context->argumentCount() == 0) {

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg0 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                  QFont _q_result = QFontDialog::getFont(_q_arg0);
        
          // TEMPLATE - core.convert_to_null_or_wrap - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = qScriptValueFromValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_wrap - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
    }
    if (context->argumentCount() == 1) {
        if (qscriptvalue_cast<QWidget*>(context->argument(0))) {

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg0 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                      QWidget* _q_arg1 = qscriptvalue_cast<QWidget*>(context->argument(0));
            QFont _q_result = QFontDialog::getFont(_q_arg0, _q_arg1);
            
          // TEMPLATE - core.convert_to_null_or_wrap - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = qScriptValueFromValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_wrap - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
        } else if ((qMetaTypeId<QFont>() == context->argument(0).toVariant().userType())) {

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg0 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                      QFont _q_arg1 = qscriptvalue_cast<QFont>(context->argument(0));
            QFont _q_result = QFontDialog::getFont(_q_arg0, _q_arg1);
            
          // TEMPLATE - core.convert_to_null_or_wrap - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = qScriptValueFromValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_wrap - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
        }
    }
    if (context->argumentCount() == 2) {

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg0 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                  QFont _q_arg1 = qscriptvalue_cast<QFont>(context->argument(0));
        QWidget* _q_arg2 = qscriptvalue_cast<QWidget*>(context->argument(1));
        QFont _q_result = QFontDialog::getFont(_q_arg0, _q_arg1, _q_arg2);
        
          // TEMPLATE - core.convert_to_null_or_wrap - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = qScriptValueFromValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_wrap - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
    }
    if (context->argumentCount() == 3) {

          // TEMPLATE - core.prepare_removed_bool*_argument - START
          bool __ok;
          bool *_q_arg0 = &__ok;
    // TEMPLATE - core.prepare_removed_bool*_argument - END
                  QFont _q_arg1 = qscriptvalue_cast<QFont>(context->argument(0));
        QWidget* _q_arg2 = qscriptvalue_cast<QWidget*>(context->argument(1));
        QString _q_arg3 = context->argument(2).toString();
        QFont _q_result = QFontDialog::getFont(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        
          // TEMPLATE - core.convert_to_null_or_wrap - START
          QScriptValue _q_convertedResult;
          if (!__ok)
              _q_convertedResult = context->engine()->nullValue();
          else
              _q_convertedResult = qScriptValueFromValue(context->engine(), _q_result);
    // TEMPLATE - core.convert_to_null_or_wrap - END
          return qScriptValueFromValue(context->engine(), _q_convertedResult);
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QFontDialog_throw_ambiguity_error_helper(context,
        qtscript_QFontDialog_function_names[_id],
        qtscript_QFontDialog_function_signatures[_id]);
}

static QScriptValue qtscript_QFontDialog_toScriptValue(QScriptEngine *engine, QFontDialog* const &in)
{
    return engine->newQObject(in, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
}

static void qtscript_QFontDialog_fromScriptValue(const QScriptValue &value, QFontDialog* &out)
{
    out = qobject_cast<QFontDialog*>(value.toQObject());
}

QScriptValue qtscript_create_QFontDialog_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        0
        // static
        , 4
        // prototype
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QFontDialog*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QFontDialog*)0));
    proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QDialog*>()));

    qScriptRegisterMetaType<QFontDialog*>(engine, qtscript_QFontDialog_toScriptValue, 
        qtscript_QFontDialog_fromScriptValue, proto);

    QScriptValue ctor = engine->newFunction(qtscript_QFontDialog_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));
    for (int i = 0; i < 1; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QFontDialog_static_call,
            function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i+1)));
        ctor.setProperty(QString::fromLatin1(qtscript_QFontDialog_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    return ctor;
}
