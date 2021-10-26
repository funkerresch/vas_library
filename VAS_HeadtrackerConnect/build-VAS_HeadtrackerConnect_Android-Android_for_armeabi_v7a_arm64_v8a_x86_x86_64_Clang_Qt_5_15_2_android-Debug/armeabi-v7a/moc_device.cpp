/****************************************************************************
** Meta object code from reading C++ file 'device.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../rwacreator/bluetooth/device.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'device.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Device_t {
    QByteArrayData data[36];
    char stringdata0[585];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Device_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Device_t qt_meta_stringdata_Device = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Device"
QT_MOC_LITERAL(1, 7, 14), // "devicesUpdated"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 15), // "servicesUpdated"
QT_MOC_LITERAL(4, 39, 22), // "characteristicsUpdated"
QT_MOC_LITERAL(5, 62, 13), // "updateChanged"
QT_MOC_LITERAL(6, 76, 12), // "stateChanged"
QT_MOC_LITERAL(7, 89, 12), // "disconnected"
QT_MOC_LITERAL(8, 102, 20), // "randomAddressChanged"
QT_MOC_LITERAL(9, 123, 19), // "sendHeadtrackerData"
QT_MOC_LITERAL(10, 143, 20), // "startDeviceDiscovery"
QT_MOC_LITERAL(11, 164, 4), // "name"
QT_MOC_LITERAL(12, 169, 12), // "scanServices"
QT_MOC_LITERAL(13, 182, 7), // "address"
QT_MOC_LITERAL(14, 190, 16), // "connectToService"
QT_MOC_LITERAL(15, 207, 4), // "uuid"
QT_MOC_LITERAL(16, 212, 20), // "disconnectFromDevice"
QT_MOC_LITERAL(17, 233, 24), // "on_characteristicChanged"
QT_MOC_LITERAL(18, 258, 24), // "QLowEnergyCharacteristic"
QT_MOC_LITERAL(19, 283, 1), // "c"
QT_MOC_LITERAL(20, 285, 1), // "a"
QT_MOC_LITERAL(21, 287, 9), // "addDevice"
QT_MOC_LITERAL(22, 297, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(23, 318, 18), // "deviceScanFinished"
QT_MOC_LITERAL(24, 337, 15), // "deviceScanError"
QT_MOC_LITERAL(25, 353, 37), // "QBluetoothDeviceDiscoveryAgen..."
QT_MOC_LITERAL(26, 391, 19), // "addLowEnergyService"
QT_MOC_LITERAL(27, 411, 14), // "QBluetoothUuid"
QT_MOC_LITERAL(28, 426, 15), // "deviceConnected"
QT_MOC_LITERAL(29, 442, 13), // "errorReceived"
QT_MOC_LITERAL(30, 456, 27), // "QLowEnergyController::Error"
QT_MOC_LITERAL(31, 484, 15), // "serviceScanDone"
QT_MOC_LITERAL(32, 500, 18), // "deviceDisconnected"
QT_MOC_LITERAL(33, 519, 24), // "serviceDetailsDiscovered"
QT_MOC_LITERAL(34, 544, 31), // "QLowEnergyService::ServiceState"
QT_MOC_LITERAL(35, 576, 8) // "newState"

    },
    "Device\0devicesUpdated\0\0servicesUpdated\0"
    "characteristicsUpdated\0updateChanged\0"
    "stateChanged\0disconnected\0"
    "randomAddressChanged\0sendHeadtrackerData\0"
    "startDeviceDiscovery\0name\0scanServices\0"
    "address\0connectToService\0uuid\0"
    "disconnectFromDevice\0on_characteristicChanged\0"
    "QLowEnergyCharacteristic\0c\0a\0addDevice\0"
    "QBluetoothDeviceInfo\0deviceScanFinished\0"
    "deviceScanError\0QBluetoothDeviceDiscoveryAgent::Error\0"
    "addLowEnergyService\0QBluetoothUuid\0"
    "deviceConnected\0errorReceived\0"
    "QLowEnergyController::Error\0serviceScanDone\0"
    "deviceDisconnected\0serviceDetailsDiscovered\0"
    "QLowEnergyService::ServiceState\0"
    "newState"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Device[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  124,    2, 0x06 /* Public */,
       3,    0,  125,    2, 0x06 /* Public */,
       4,    0,  126,    2, 0x06 /* Public */,
       5,    0,  127,    2, 0x06 /* Public */,
       6,    0,  128,    2, 0x06 /* Public */,
       7,    0,  129,    2, 0x06 /* Public */,
       8,    0,  130,    2, 0x06 /* Public */,
       9,    1,  131,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    1,  134,    2, 0x0a /* Public */,
      12,    1,  137,    2, 0x0a /* Public */,
      14,    1,  140,    2, 0x0a /* Public */,
      16,    0,  143,    2, 0x0a /* Public */,
      17,    2,  144,    2, 0x0a /* Public */,
      21,    1,  149,    2, 0x08 /* Private */,
      23,    0,  152,    2, 0x08 /* Private */,
      24,    1,  153,    2, 0x08 /* Private */,
      26,    1,  156,    2, 0x08 /* Private */,
      28,    0,  159,    2, 0x08 /* Private */,
      29,    1,  160,    2, 0x08 /* Private */,
      31,    0,  163,    2, 0x08 /* Private */,
      32,    0,  164,    2, 0x08 /* Private */,
      33,    1,  165,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 18, QMetaType::QByteArray,   19,   20,
    QMetaType::Void, 0x80000000 | 22,    2,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 25,    2,
    QMetaType::Void, 0x80000000 | 27,   15,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 30,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 34,   35,

       0        // eod
};

void Device::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Device *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->devicesUpdated(); break;
        case 1: _t->servicesUpdated(); break;
        case 2: _t->characteristicsUpdated(); break;
        case 3: _t->updateChanged(); break;
        case 4: _t->stateChanged(); break;
        case 5: _t->disconnected(); break;
        case 6: _t->randomAddressChanged(); break;
        case 7: _t->sendHeadtrackerData((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 8: _t->startDeviceDiscovery((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: _t->scanServices((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->connectToService((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->disconnectFromDevice(); break;
        case 12: _t->on_characteristicChanged((*reinterpret_cast< QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        case 13: _t->addDevice((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 14: _t->deviceScanFinished(); break;
        case 15: _t->deviceScanError((*reinterpret_cast< QBluetoothDeviceDiscoveryAgent::Error(*)>(_a[1]))); break;
        case 16: _t->addLowEnergyService((*reinterpret_cast< const QBluetoothUuid(*)>(_a[1]))); break;
        case 17: _t->deviceConnected(); break;
        case 18: _t->errorReceived((*reinterpret_cast< QLowEnergyController::Error(*)>(_a[1]))); break;
        case 19: _t->serviceScanDone(); break;
        case 20: _t->deviceDisconnected(); break;
        case 21: _t->serviceDetailsDiscovered((*reinterpret_cast< QLowEnergyService::ServiceState(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyCharacteristic >(); break;
            }
            break;
        case 13:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothDeviceInfo >(); break;
            }
            break;
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothUuid >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyController::Error >(); break;
            }
            break;
        case 21:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyService::ServiceState >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::devicesUpdated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::servicesUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::characteristicsUpdated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::updateChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::stateChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::disconnected)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (Device::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::randomAddressChanged)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (Device::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Device::sendHeadtrackerData)) {
                *result = 7;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Device::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Device.data,
    qt_meta_data_Device,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Device::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Device::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Device.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Device::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void Device::devicesUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Device::servicesUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Device::characteristicsUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Device::updateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Device::stateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Device::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void Device::randomAddressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void Device::sendHeadtrackerData(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
