#ifndef QTSCRIPTSHELL_QTEXTOBJECTINTERFACE_H
#define QTSCRIPTSHELL_QTEXTOBJECTINTERFACE_H

#include <qabstracttextdocumentlayout.h>

#include <QtScript/qscriptvalue.h>

class QtScriptShell_QTextObjectInterface : public QTextObjectInterface
{
public:
    QtScriptShell_QTextObjectInterface();
    ~QtScriptShell_QTextObjectInterface();

    void drawObject(QPainter*  painter, const QRectF&  rect, QTextDocument*  doc, int  posInDocument, const QTextFormat&  format);
    QSizeF  intrinsicSize(QTextDocument*  doc, int  posInDocument, const QTextFormat&  format);

    QScriptValue __qtscript_self;
};

#endif // QTSCRIPTSHELL_QTEXTOBJECTINTERFACE_H
