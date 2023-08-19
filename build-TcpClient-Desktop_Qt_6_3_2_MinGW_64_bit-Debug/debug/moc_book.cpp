/****************************************************************************
** Meta object code from reading C++ file 'book.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.3.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../TcpClient/book.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'book.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.3.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Book_t {
    uint offsetsAndSizes[34];
    char stringdata0[5];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[12];
    char stringdata5[12];
    char stringdata6[5];
    char stringdata7[7];
    char stringdata8[14];
    char stringdata9[11];
    char stringdata10[11];
    char stringdata11[15];
    char stringdata12[11];
    char stringdata13[13];
    char stringdata14[10];
    char stringdata15[9];
    char stringdata16[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Book_t::offsetsAndSizes) + ofs), len 
static const qt_meta_stringdata_Book_t qt_meta_stringdata_Book = {
    {
        QT_MOC_LITERAL(0, 4),  // "Book"
        QT_MOC_LITERAL(5, 9),  // "createDir"
        QT_MOC_LITERAL(15, 0),  // ""
        QT_MOC_LITERAL(16, 9),  // "flushFile"
        QT_MOC_LITERAL(26, 11),  // "showDirFile"
        QT_MOC_LITERAL(38, 11),  // "QModelIndex"
        QT_MOC_LITERAL(50, 4),  // "item"
        QT_MOC_LITERAL(55, 6),  // "delDir"
        QT_MOC_LITERAL(62, 13),  // "returnDirFile"
        QT_MOC_LITERAL(76, 10),  // "renameFile"
        QT_MOC_LITERAL(87, 10),  // "uploadFile"
        QT_MOC_LITERAL(98, 14),  // "uploadFileData"
        QT_MOC_LITERAL(113, 10),  // "delRegFile"
        QT_MOC_LITERAL(124, 12),  // "downloadFile"
        QT_MOC_LITERAL(137, 9),  // "shareFile"
        QT_MOC_LITERAL(147, 8),  // "moveFile"
        QT_MOC_LITERAL(156, 13)   // "selectDestDir"
    },
    "Book",
    "createDir",
    "",
    "flushFile",
    "showDirFile",
    "QModelIndex",
    "item",
    "delDir",
    "returnDirFile",
    "renameFile",
    "uploadFile",
    "uploadFileData",
    "delRegFile",
    "downloadFile",
    "shareFile",
    "moveFile",
    "selectDestDir"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Book[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x08,    1 /* Private */,
       3,    0,   93,    2, 0x08,    2 /* Private */,
       4,    1,   94,    2, 0x08,    3 /* Private */,
       7,    0,   97,    2, 0x08,    5 /* Private */,
       8,    0,   98,    2, 0x08,    6 /* Private */,
       9,    0,   99,    2, 0x08,    7 /* Private */,
      10,    0,  100,    2, 0x08,    8 /* Private */,
      11,    0,  101,    2, 0x08,    9 /* Private */,
      12,    0,  102,    2, 0x08,   10 /* Private */,
      13,    0,  103,    2, 0x08,   11 /* Private */,
      14,    0,  104,    2, 0x08,   12 /* Private */,
      15,    0,  105,    2, 0x08,   13 /* Private */,
      16,    0,  106,    2, 0x08,   14 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Book::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Book *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->createDir(); break;
        case 1: _t->flushFile(); break;
        case 2: _t->showDirFile((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1]))); break;
        case 3: _t->delDir(); break;
        case 4: _t->returnDirFile(); break;
        case 5: _t->renameFile(); break;
        case 6: _t->uploadFile(); break;
        case 7: _t->uploadFileData(); break;
        case 8: _t->delRegFile(); break;
        case 9: _t->downloadFile(); break;
        case 10: _t->shareFile(); break;
        case 11: _t->moveFile(); break;
        case 12: _t->selectDestDir(); break;
        default: ;
        }
    }
}

const QMetaObject Book::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Book.offsetsAndSizes,
    qt_meta_data_Book,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Book_t
, QtPrivate::TypeAndForceComplete<Book, std::true_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<QModelIndex, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *Book::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Book::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Book.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Book::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
