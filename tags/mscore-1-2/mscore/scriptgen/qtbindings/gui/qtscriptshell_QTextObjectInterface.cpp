#include "qtscriptshell_QTextObjectInterface.h"

#include <QtScript/QScriptEngine>
#include <QVariant>
#include <qpainter.h>
#include <qrect.h>
#include <qsize.h>
#include <qtextdocument.h>
#include <qtextformat.h>

#define QTSCRIPT_IS_GENERATED_FUNCTION(fun) ((fun.data().toUInt32() & 0xFFFF0000) == 0xBABE0000)

Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QTextDocument*)

QtScriptShell_QTextObjectInterface::QtScriptShell_QTextObjectInterface()
    : QTextObjectInterface() {}

QtScriptShell_QTextObjectInterface::~QtScriptShell_QTextObjectInterface() {}

void QtScriptShell_QTextObjectInterface::drawObject(QPainter*  painter, const QRectF&  rect, QTextDocument*  doc, int  posInDocument, const QTextFormat&  format)
{
    QScriptValue _q_function = __qtscript_self.property("drawObject");
    if (!_q_function.isFunction() || QTSCRIPT_IS_GENERATED_FUNCTION(_q_function)
        || (__qtscript_self.propertyFlags("drawObject") & QScriptValue::QObjectMember)) {
        qFatal("QTextObjectInterface::drawObject() is abstract!");
    } else {
        QScriptEngine *_q_engine = __qtscript_self.engine();
        _q_function.call(__qtscript_self,
            QScriptValueList()
            << qScriptValueFromValue(_q_engine, painter)
            << qScriptValueFromValue(_q_engine, rect)
            << qScriptValueFromValue(_q_engine, doc)
            << qScriptValueFromValue(_q_engine, posInDocument)
            << qScriptValueFromValue(_q_engine, format));
    }
}

QSizeF  QtScriptShell_QTextObjectInterface::intrinsicSize(QTextDocument*  doc, int  posInDocument, const QTextFormat&  format)
{
    QScriptValue _q_function = __qtscript_self.property("intrinsicSize");
    if (!_q_function.isFunction() || QTSCRIPT_IS_GENERATED_FUNCTION(_q_function)
        || (__qtscript_self.propertyFlags("intrinsicSize") & QScriptValue::QObjectMember)) {
        qFatal("QTextObjectInterface::intrinsicSize() is abstract!");
    } else {
        QScriptEngine *_q_engine = __qtscript_self.engine();
        return qscriptvalue_cast<QSizeF >(_q_function.call(__qtscript_self,
            QScriptValueList()
            << qScriptValueFromValue(_q_engine, doc)
            << qScriptValueFromValue(_q_engine, posInDocument)
            << qScriptValueFromValue(_q_engine, format)));
    }
}

