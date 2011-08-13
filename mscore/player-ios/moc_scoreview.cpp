/****************************************************************************
** Meta object code from reading C++ file 'scoreview.h'
**
** Created: Sat Aug 13 14:54:30 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../player-qt/scoreview.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'scoreview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ScoreView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   11,   10,   10, 0x0a,
      31,   10,   10,   10, 0x0a,
      40,   38,   10,   10, 0x0a,
      60,   10,   10,   10, 0x0a,
      71,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ScoreView[] = {
    "ScoreView\0\0s\0setScore(QString)\0play()\0"
    "n\0setCurrentPage(int)\0nextPage()\0"
    "prevPage()\0"
};

void ScoreView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ScoreView *_t = static_cast<ScoreView *>(_o);
        switch (_id) {
        case 0: _t->setScore((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->play(); break;
        case 2: _t->setCurrentPage((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->nextPage(); break;
        case 4: _t->prevPage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ScoreView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ScoreView::staticMetaObject = {
    { &QDeclarativeItem::staticMetaObject, qt_meta_stringdata_ScoreView,
      qt_meta_data_ScoreView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ScoreView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ScoreView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ScoreView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ScoreView))
        return static_cast<void*>(const_cast< ScoreView*>(this));
    if (!strcmp(_clname, "MuseScoreView"))
        return static_cast< MuseScoreView*>(const_cast< ScoreView*>(this));
    return QDeclarativeItem::qt_metacast(_clname);
}

int ScoreView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDeclarativeItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
