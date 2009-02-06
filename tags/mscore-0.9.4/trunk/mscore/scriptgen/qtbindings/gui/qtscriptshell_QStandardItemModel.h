#ifndef QTSCRIPTSHELL_QSTANDARDITEMMODEL_H
#define QTSCRIPTSHELL_QSTANDARDITEMMODEL_H

#include <qstandarditemmodel.h>

#include <QtScript/qscriptvalue.h>

class QtScriptShell_QStandardItemModel : public QStandardItemModel
{
public:
    QtScriptShell_QStandardItemModel(QObject*  parent = 0);
    QtScriptShell_QStandardItemModel(int  rows, int  columns, QObject*  parent = 0);
    ~QtScriptShell_QStandardItemModel();


    QScriptValue __qtscript_self;
};

#endif // QTSCRIPTSHELL_QSTANDARDITEMMODEL_H
