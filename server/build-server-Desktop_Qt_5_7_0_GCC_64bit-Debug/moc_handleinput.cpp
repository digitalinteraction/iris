/****************************************************************************
** Meta object code from reading C++ file 'handleinput.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../server/handleinput.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'handleinput.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_HandleInput_t {
    QByteArrayData data[13];
    char stringdata0[83];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_HandleInput_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_HandleInput_t qt_meta_stringdata_HandleInput = {
    {
QT_MOC_LITERAL(0, 0, 11), // "HandleInput"
QT_MOC_LITERAL(1, 12, 10), // "MACChanged"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 1), // "x"
QT_MOC_LITERAL(4, 26, 1), // "y"
QT_MOC_LITERAL(5, 28, 3), // "mac"
QT_MOC_LITERAL(6, 32, 12), // "NewImageData"
QT_MOC_LITERAL(7, 45, 4), // "posx"
QT_MOC_LITERAL(8, 50, 4), // "posy"
QT_MOC_LITERAL(9, 55, 14), // "unsigned char*"
QT_MOC_LITERAL(10, 70, 3), // "buf"
QT_MOC_LITERAL(11, 74, 4), // "size"
QT_MOC_LITERAL(12, 79, 3) // "pos"

    },
    "HandleInput\0MACChanged\0\0x\0y\0mac\0"
    "NewImageData\0posx\0posy\0unsigned char*\0"
    "buf\0size\0pos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_HandleInput[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   24,    2, 0x06 /* Public */,
       6,    5,   31,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Long,    3,    4,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, 0x80000000 | 9, QMetaType::Int, QMetaType::Int,    7,    8,   10,   11,   12,

       0        // eod
};

void HandleInput::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HandleInput *_t = static_cast<HandleInput *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->MACChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< long(*)>(_a[3]))); break;
        case 1: _t->NewImageData((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< unsigned char*(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (HandleInput::*_t)(int , int , long );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HandleInput::MACChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (HandleInput::*_t)(int , int , unsigned char * , int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HandleInput::NewImageData)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject HandleInput::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_HandleInput.data,
      qt_meta_data_HandleInput,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *HandleInput::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HandleInput::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_HandleInput.stringdata0))
        return static_cast<void*>(const_cast< HandleInput*>(this));
    return QWidget::qt_metacast(_clname);
}

int HandleInput::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void HandleInput::MACChanged(int _t1, int _t2, long _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void HandleInput::NewImageData(int _t1, int _t2, unsigned char * _t3, int _t4, int _t5)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
