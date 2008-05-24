#include "qtscriptshell_QStandardItemModel.h"

#include <QtScript/QScriptEngine>
#include <QSize>
#include <QStringList>
#include <QVariant>
#include <qlist.h>
#include <qobject.h>
#include <qstandarditemmodel.h>
#include <qstringlist.h>

#define QTSCRIPT_IS_GENERATED_FUNCTION(fun) ((fun.data().toUInt32() & 0xFFFF0000) == 0xBABE0000)


QtScriptShell_QStandardItemModel::QtScriptShell_QStandardItemModel(QObject*  parent)
    : QStandardItemModel(parent) {}

QtScriptShell_QStandardItemModel::QtScriptShell_QStandardItemModel(int  rows, int  columns, QObject*  parent)
    : QStandardItemModel(rows, columns, parent) {}

QtScriptShell_QStandardItemModel::~QtScriptShell_QStandardItemModel() {}

