#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptValue>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
#include <qmetaobject.h>

#include <qstandarditemmodel.h>
#include <QSize>
#include <QStringList>
#include <QVariant>
#include <qlist.h>
#include <qobject.h>
#include <qstandarditemmodel.h>
#include <qstringlist.h>

#include "qtscriptshell_QStandardItemModel.h"

static const char * const qtscript_QStandardItemModel_function_names[] = {
    "QStandardItemModel"
    // static
    // prototype
    , "appendColumn"
    , "appendRow"
    , "clear"
    , "columnCount"
    , "data"
    , "findItems"
    , "flags"
    , "hasChildren"
    , "headerData"
    , "horizontalHeaderItem"
    , "index"
    , "indexFromItem"
    , "insertColumn"
    , "insertColumns"
    , "insertRow"
    , "insertRows"
    , "invisibleRootItem"
    , "item"
    , "itemData"
    , "itemFromIndex"
    , "itemPrototype"
    , "parent"
    , "removeColumns"
    , "removeRows"
    , "rowCount"
    , "setColumnCount"
    , "setData"
    , "setHeaderData"
    , "setHorizontalHeaderItem"
    , "setHorizontalHeaderLabels"
    , "setItem"
    , "setItemData"
    , "setItemPrototype"
    , "setRowCount"
    , "setVerticalHeaderItem"
    , "setVerticalHeaderLabels"
    , "sort"
    , "supportedDropActions"
    , "takeColumn"
    , "takeHorizontalHeaderItem"
    , "takeItem"
    , "takeRow"
    , "takeVerticalHeaderItem"
    , "verticalHeaderItem"
    , "toString"
};

static const char * const qtscript_QStandardItemModel_function_signatures[] = {
    "QObject parent\nint rows, int columns, QObject parent"
    // static
    // prototype
    , "List items"
    , "QStandardItem item\nList items"
    , ""
    , "QModelIndex parent"
    , "QModelIndex index, int role"
    , "String text, MatchFlags flags, int column"
    , "QModelIndex index"
    , "QModelIndex parent"
    , "int section, Orientation orientation, int role"
    , "int column"
    , "int row, int column, QModelIndex parent"
    , "QStandardItem item"
    , "int column, List items"
    , "int column, int count, QModelIndex parent"
    , "int row, QStandardItem item\nint row, List items"
    , "int row, int count, QModelIndex parent"
    , ""
    , "int row, int column"
    , "QModelIndex index"
    , "QModelIndex index"
    , ""
    , "QModelIndex child"
    , "int column, int count, QModelIndex parent"
    , "int row, int count, QModelIndex parent"
    , "QModelIndex parent"
    , "int columns"
    , "QModelIndex index, Object value, int role"
    , "int section, Orientation orientation, Object value, int role"
    , "int column, QStandardItem item"
    , "List labels"
    , "int row, QStandardItem item\nint row, int column, QStandardItem item"
    , "QModelIndex index, SortedMap roles"
    , "QStandardItem item"
    , "int rows"
    , "int row, QStandardItem item"
    , "List labels"
    , "int column, SortOrder order"
    , ""
    , "int column"
    , "int column"
    , "int row, int column"
    , "int row"
    , "int row"
    , "int row"
""
};

static QScriptValue qtscript_QStandardItemModel_throw_ambiguity_error_helper(
    QScriptContext *context, const char *functionName, const char *signatures)
{
    QStringList lines = QString::fromLatin1(signatures).split(QLatin1Char('\n'));
    QStringList fullSignatures;
    for (int i = 0; i < lines.size(); ++i)
        fullSignatures.append(QString::fromLatin1("%0(%1)").arg(functionName).arg(lines.at(i)));
    return context->throwError(QString::fromLatin1("QFile::%0(): could not find a function match; candidates are:\n%1")
        .arg(functionName).arg(fullSignatures.join(QLatin1String("\n"))));
}

Q_DECLARE_METATYPE(QStandardItemModel*)
Q_DECLARE_METATYPE(QtScriptShell_QStandardItemModel*)
Q_DECLARE_METATYPE(QStandardItem*)
Q_DECLARE_METATYPE(QList<QStandardItem*>)
Q_DECLARE_METATYPE(QModelIndex)
Q_DECLARE_METATYPE(QVariant)
Q_DECLARE_METATYPE(QFlags<Qt::MatchFlag>)
Q_DECLARE_METATYPE(QFlags<Qt::ItemFlag>)
Q_DECLARE_METATYPE(Qt::Orientation)
template <> \
struct QMetaTypeId< QMap<int,QVariant> > \
{ \
    enum { Defined = 1 }; \
    static int qt_metatype_id() \
    { \
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
        if (!metatype_id) \
            metatype_id = qRegisterMetaType< QMap<int,QVariant> >("QMap<int,QVariant>"); \
        return metatype_id; \
    } \
};
Q_DECLARE_METATYPE(Qt::SortOrder)
Q_DECLARE_METATYPE(QFlags<Qt::DropAction>)

//
// QStandardItemModel
//

static QScriptValue qtscript_QStandardItemModel_prototype_call(QScriptContext *context, QScriptEngine *)
{
#if QT_VERSION > 0x040400
    Q_ASSERT(context->callee().isFunction());
    uint _id = context->callee().data().toUInt32();
#else
    uint _id;
    if (context->callee().isFunction())
        _id = context->callee().data().toUInt32();
    else
        _id = 0xBABE0000 + 44;
#endif
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    QStandardItemModel* _q_self = qscriptvalue_cast<QStandardItemModel*>(context->thisObject());
    if (!_q_self) {
        return context->throwError(QScriptContext::TypeError,
            QString::fromLatin1("QStandardItemModel.%0(): this object is not a QStandardItemModel")
            .arg(qtscript_QStandardItemModel_function_names[_id+1]));
    }

    switch (_id) {
    case 0:
    if (context->argumentCount() == 1) {
        QList<QStandardItem*> _q_arg0;
        qScriptValueToSequence(context->argument(0), _q_arg0);
        _q_self->appendColumn(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 1:
    if (context->argumentCount() == 1) {
        if (qscriptvalue_cast<QStandardItem*>(context->argument(0))) {
            QStandardItem* _q_arg0 = qscriptvalue_cast<QStandardItem*>(context->argument(0));
            _q_self->appendRow(_q_arg0);
            return context->engine()->undefinedValue();
        } else if (context->argument(0).isArray()) {
            QList<QStandardItem*> _q_arg0;
            qScriptValueToSequence(context->argument(0), _q_arg0);
            _q_self->appendRow(_q_arg0);
            return context->engine()->undefinedValue();
        }
    }
    break;

    case 2:
    if (context->argumentCount() == 0) {
        _q_self->clear();
        return context->engine()->undefinedValue();
    }
    break;

    case 3:
    if (context->argumentCount() == 0) {
        int _q_result = _q_self->columnCount();
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        int _q_result = _q_self->columnCount(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 4:
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QVariant _q_result = _q_self->data(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 2) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        int _q_arg1 = context->argument(1).toInt32();
        QVariant _q_result = _q_self->data(_q_arg0, _q_arg1);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 5:
    if (context->argumentCount() == 1) {
        QString _q_arg0 = context->argument(0).toString();
        QList<QStandardItem*> _q_result = _q_self->findItems(_q_arg0);
        return qScriptValueFromSequence(context->engine(), _q_result);
    }
    if (context->argumentCount() == 2) {
        QString _q_arg0 = context->argument(0).toString();
        QFlags<Qt::MatchFlag> _q_arg1 = qscriptvalue_cast<QFlags<Qt::MatchFlag> >(context->argument(1));
        QList<QStandardItem*> _q_result = _q_self->findItems(_q_arg0, _q_arg1);
        return qScriptValueFromSequence(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        QString _q_arg0 = context->argument(0).toString();
        QFlags<Qt::MatchFlag> _q_arg1 = qscriptvalue_cast<QFlags<Qt::MatchFlag> >(context->argument(1));
        int _q_arg2 = context->argument(2).toInt32();
        QList<QStandardItem*> _q_result = _q_self->findItems(_q_arg0, _q_arg1, _q_arg2);
        return qScriptValueFromSequence(context->engine(), _q_result);
    }
    break;

    case 6:
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QFlags<Qt::ItemFlag> _q_result = _q_self->flags(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 7:
    if (context->argumentCount() == 0) {
        bool _q_result = _q_self->hasChildren();
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        bool _q_result = _q_self->hasChildren(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 8:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        Qt::Orientation _q_arg1 = qscriptvalue_cast<Qt::Orientation>(context->argument(1));
        QVariant _q_result = _q_self->headerData(_q_arg0, _q_arg1);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        Qt::Orientation _q_arg1 = qscriptvalue_cast<Qt::Orientation>(context->argument(1));
        int _q_arg2 = context->argument(2).toInt32();
        QVariant _q_result = _q_self->headerData(_q_arg0, _q_arg1, _q_arg2);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 9:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_result = _q_self->horizontalHeaderItem(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 10:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QModelIndex _q_result = _q_self->index(_q_arg0, _q_arg1);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QModelIndex _q_arg2 = qscriptvalue_cast<QModelIndex>(context->argument(2));
        QModelIndex _q_result = _q_self->index(_q_arg0, _q_arg1, _q_arg2);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 11:
    if (context->argumentCount() == 1) {
        QStandardItem* _q_arg0 = qscriptvalue_cast<QStandardItem*>(context->argument(0));
        QModelIndex _q_result = _q_self->indexFromItem(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 12:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        QList<QStandardItem*> _q_arg1;
        qScriptValueToSequence(context->argument(1), _q_arg1);
        _q_self->insertColumn(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 13:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        bool _q_result = _q_self->insertColumns(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QModelIndex _q_arg2 = qscriptvalue_cast<QModelIndex>(context->argument(2));
        bool _q_result = _q_self->insertColumns(_q_arg0, _q_arg1, _q_arg2);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 14:
    if (context->argumentCount() == 2) {
        if (context->argument(0).isNumber()
            && qscriptvalue_cast<QStandardItem*>(context->argument(1))) {
            int _q_arg0 = context->argument(0).toInt32();
            QStandardItem* _q_arg1 = qscriptvalue_cast<QStandardItem*>(context->argument(1));
            _q_self->insertRow(_q_arg0, _q_arg1);
            return context->engine()->undefinedValue();
        } else if (context->argument(0).isNumber()
            && context->argument(1).isArray()) {
            int _q_arg0 = context->argument(0).toInt32();
            QList<QStandardItem*> _q_arg1;
            qScriptValueToSequence(context->argument(1), _q_arg1);
            _q_self->insertRow(_q_arg0, _q_arg1);
            return context->engine()->undefinedValue();
        }
    }
    break;

    case 15:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        bool _q_result = _q_self->insertRows(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QModelIndex _q_arg2 = qscriptvalue_cast<QModelIndex>(context->argument(2));
        bool _q_result = _q_self->insertRows(_q_arg0, _q_arg1, _q_arg2);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 16:
    if (context->argumentCount() == 0) {
        QStandardItem* _q_result = _q_self->invisibleRootItem();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 17:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_result = _q_self->item(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QStandardItem* _q_result = _q_self->item(_q_arg0, _q_arg1);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 18:
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QMap<int,QVariant> _q_result = _q_self->itemData(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 19:
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QStandardItem* _q_result = _q_self->itemFromIndex(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 20:
    if (context->argumentCount() == 0) {
        QStandardItem* _q_result = const_cast<QStandardItem*>(_q_self->itemPrototype());
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 21:
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QModelIndex _q_result = _q_self->parent(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 22:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        bool _q_result = _q_self->removeColumns(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QModelIndex _q_arg2 = qscriptvalue_cast<QModelIndex>(context->argument(2));
        bool _q_result = _q_self->removeColumns(_q_arg0, _q_arg1, _q_arg2);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 23:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        bool _q_result = _q_self->removeRows(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QModelIndex _q_arg2 = qscriptvalue_cast<QModelIndex>(context->argument(2));
        bool _q_result = _q_self->removeRows(_q_arg0, _q_arg1, _q_arg2);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 24:
    if (context->argumentCount() == 0) {
        int _q_result = _q_self->rowCount();
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 1) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        int _q_result = _q_self->rowCount(_q_arg0);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 25:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        _q_self->setColumnCount(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 26:
    if (context->argumentCount() == 2) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QVariant _q_arg1 = context->argument(1).toVariant();
        bool _q_result = _q_self->setData(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 3) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QVariant _q_arg1 = context->argument(1).toVariant();
        int _q_arg2 = context->argument(2).toInt32();
        bool _q_result = _q_self->setData(_q_arg0, _q_arg1, _q_arg2);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 27:
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        Qt::Orientation _q_arg1 = qscriptvalue_cast<Qt::Orientation>(context->argument(1));
        QVariant _q_arg2 = context->argument(2).toVariant();
        bool _q_result = _q_self->setHeaderData(_q_arg0, _q_arg1, _q_arg2);
        return QScriptValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 4) {
        int _q_arg0 = context->argument(0).toInt32();
        Qt::Orientation _q_arg1 = qscriptvalue_cast<Qt::Orientation>(context->argument(1));
        QVariant _q_arg2 = context->argument(2).toVariant();
        int _q_arg3 = context->argument(3).toInt32();
        bool _q_result = _q_self->setHeaderData(_q_arg0, _q_arg1, _q_arg2, _q_arg3);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 28:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_arg1 = qscriptvalue_cast<QStandardItem*>(context->argument(1));
        _q_self->setHorizontalHeaderItem(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 29:
    if (context->argumentCount() == 1) {
        QStringList _q_arg0;
        qScriptValueToSequence(context->argument(0), _q_arg0);
        _q_self->setHorizontalHeaderLabels(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 30:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_arg1 = qscriptvalue_cast<QStandardItem*>(context->argument(1));
        _q_self->setItem(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QStandardItem* _q_arg2 = qscriptvalue_cast<QStandardItem*>(context->argument(2));
        _q_self->setItem(_q_arg0, _q_arg1, _q_arg2);
        return context->engine()->undefinedValue();
    }
    break;

    case 31:
    if (context->argumentCount() == 2) {
        QModelIndex _q_arg0 = qscriptvalue_cast<QModelIndex>(context->argument(0));
        QMap<int,QVariant> _q_arg1 = qscriptvalue_cast<QMap<int,QVariant> >(context->argument(1));
        bool _q_result = _q_self->setItemData(_q_arg0, _q_arg1);
        return QScriptValue(context->engine(), _q_result);
    }
    break;

    case 32:
    if (context->argumentCount() == 1) {
        QStandardItem* _q_arg0 = qscriptvalue_cast<QStandardItem*>(context->argument(0));
        _q_self->setItemPrototype(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 33:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        _q_self->setRowCount(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 34:
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_arg1 = qscriptvalue_cast<QStandardItem*>(context->argument(1));
        _q_self->setVerticalHeaderItem(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 35:
    if (context->argumentCount() == 1) {
        QStringList _q_arg0;
        qScriptValueToSequence(context->argument(0), _q_arg0);
        _q_self->setVerticalHeaderLabels(_q_arg0);
        return context->engine()->undefinedValue();
    }
    break;

    case 36:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        _q_self->sort(_q_arg0);
        return context->engine()->undefinedValue();
    }
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        Qt::SortOrder _q_arg1 = qscriptvalue_cast<Qt::SortOrder>(context->argument(1));
        _q_self->sort(_q_arg0, _q_arg1);
        return context->engine()->undefinedValue();
    }
    break;

    case 37:
    if (context->argumentCount() == 0) {
        QFlags<Qt::DropAction> _q_result = _q_self->supportedDropActions();
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 38:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QList<QStandardItem*> _q_result = _q_self->takeColumn(_q_arg0);
        return qScriptValueFromSequence(context->engine(), _q_result);
    }
    break;

    case 39:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_result = _q_self->takeHorizontalHeaderItem(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 40:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_result = _q_self->takeItem(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QStandardItem* _q_result = _q_self->takeItem(_q_arg0, _q_arg1);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 41:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QList<QStandardItem*> _q_result = _q_self->takeRow(_q_arg0);
        return qScriptValueFromSequence(context->engine(), _q_result);
    }
    break;

    case 42:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_result = _q_self->takeVerticalHeaderItem(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 43:
    if (context->argumentCount() == 1) {
        int _q_arg0 = context->argument(0).toInt32();
        QStandardItem* _q_result = _q_self->verticalHeaderItem(_q_arg0);
        return qScriptValueFromValue(context->engine(), _q_result);
    }
    break;

    case 44: {
    QString result = QString::fromLatin1("QStandardItemModel");
    return QScriptValue(context->engine(), result);
    }

    default:
    Q_ASSERT(false);
    }
    return qtscript_QStandardItemModel_throw_ambiguity_error_helper(context,
        qtscript_QStandardItemModel_function_names[_id+1],
        qtscript_QStandardItemModel_function_signatures[_id+1]);
}

static QScriptValue qtscript_QStandardItemModel_static_call(QScriptContext *context, QScriptEngine *)
{
    uint _id = context->callee().data().toUInt32();
    Q_ASSERT((_id & 0xFFFF0000) == 0xBABE0000);
    _id &= 0x0000FFFF;
    switch (_id) {
    case 0:
    if (context->thisObject().strictlyEquals(context->engine()->globalObject())) {
        return context->throwError(QString::fromLatin1("QStandardItemModel(): Did you forget to construct with 'new'?"));
    }
    if (context->argumentCount() == 0) {
        QtScriptShell_QStandardItemModel* _q_cpp_result = new QtScriptShell_QStandardItemModel();
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QStandardItemModel*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    } else if (context->argumentCount() == 1) {
        QObject* _q_arg0 = context->argument(0).toQObject();
        QtScriptShell_QStandardItemModel* _q_cpp_result = new QtScriptShell_QStandardItemModel(_q_arg0);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QStandardItemModel*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    } else if (context->argumentCount() == 2) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QtScriptShell_QStandardItemModel* _q_cpp_result = new QtScriptShell_QStandardItemModel(_q_arg0, _q_arg1);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QStandardItemModel*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    } else if (context->argumentCount() == 3) {
        int _q_arg0 = context->argument(0).toInt32();
        int _q_arg1 = context->argument(1).toInt32();
        QObject* _q_arg2 = context->argument(2).toQObject();
        QtScriptShell_QStandardItemModel* _q_cpp_result = new QtScriptShell_QStandardItemModel(_q_arg0, _q_arg1, _q_arg2);
        QScriptValue _q_result = context->engine()->newVariant(context->thisObject(), qVariantFromValue((QStandardItemModel*)_q_cpp_result));
        _q_cpp_result->__qtscript_self = _q_result;
        return _q_result;
    }
    break;

    default:
    Q_ASSERT(false);
    }
    return qtscript_QStandardItemModel_throw_ambiguity_error_helper(context,
        qtscript_QStandardItemModel_function_names[_id],
        qtscript_QStandardItemModel_function_signatures[_id]);
}

QScriptValue qtscript_create_QStandardItemModel_class(QScriptEngine *engine)
{
    static const int function_lengths[] = {
        3
        // static
        // prototype
        , 1
        , 1
        , 0
        , 1
        , 2
        , 3
        , 1
        , 1
        , 3
        , 1
        , 3
        , 1
        , 2
        , 3
        , 2
        , 3
        , 0
        , 2
        , 1
        , 1
        , 0
        , 1
        , 3
        , 3
        , 1
        , 1
        , 3
        , 4
        , 2
        , 1
        , 3
        , 2
        , 1
        , 1
        , 2
        , 1
        , 2
        , 0
        , 1
        , 1
        , 2
        , 1
        , 1
        , 1
        , 0
    };
    engine->setDefaultPrototype(qMetaTypeId<QStandardItemModel*>(), QScriptValue());
    QScriptValue proto = engine->newVariant(qVariantFromValue((QStandardItemModel*)0));
    for (int i = 0; i < 45; ++i) {
        QScriptValue fun = engine->newFunction(qtscript_QStandardItemModel_prototype_call, function_lengths[i+1]);
        fun.setData(QScriptValue(engine, uint(0xBABE0000 + i)));
        proto.setProperty(QString::fromLatin1(qtscript_QStandardItemModel_function_names[i+1]),
            fun, QScriptValue::SkipInEnumeration);
    }

    engine->setDefaultPrototype(qMetaTypeId<QStandardItemModel*>(), proto);

    QScriptValue ctor = engine->newFunction(qtscript_QStandardItemModel_static_call, proto, function_lengths[0]);
    ctor.setData(QScriptValue(engine, uint(0xBABE0000 + 0)));

    return ctor;
}
