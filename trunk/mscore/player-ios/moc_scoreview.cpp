/****************************************************************************
** Meta object code from reading C++ file 'scoreview.h'
**
** Created: Tue Jul 19 22:32:11 2011
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "scoreview.h"
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
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   11,   10,   10, 0x0a,
      33,   11,   10,   10, 0x0a,
      58,   56,   10,   10, 0x0a,
      76,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_ScoreView[] = {
    "ScoreView\0\0x,y\0drag(qreal,qreal)\0"
    "startDrag(qreal,qreal)\0s\0setScore(QString)\0"
    "play()\0"
};

void ScoreView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ScoreView *_t = static_cast<ScoreView *>(_o);
        switch (_id) {
        case 0: _t->drag((*reinterpret_cast< qreal(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2]))); break;
        case 1: _t->startDrag((*reinterpret_cast< qreal(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2]))); break;
        case 2: _t->setScore((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->play(); break;
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
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
