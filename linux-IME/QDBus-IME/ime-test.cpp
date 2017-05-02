//http://stackoverflow.com/questions/20206376/how-do-i-extract-the-returned-data-from-qdbusmessage-in-a-qt-dbus-call
//http://doc.qt.io/qt-5/qstring.html
//http://www.cnblogs.com/wwang/archive/2010/10/27/1862552.html

#include <QtCore>
#include <QtDBus>



class signalTest
{
    
public:
    signalTest();
    ~signalTest();
    
    void SlotPropertiesChanged(QString interface_name, QVariantMap changed_properties, QStringList invalidated_properties);
};

signalTest::signalTest()
{
    QDBusConnection::sessionBus().connect("org.fcitx.Fcitx",
                                          "/inputmethod",
                                          "org.freedesktop.DBus.Properties",
                                          "PropertiesChanged",
                                          QOBJECT(this),
                                          SLOT(SlotPropertiesChanged(QString interface_name, QVariantMap changed_properties, QStringList invalidated_properties))
                                          )
}

void signalTest::SlotPropertiesChanged(QString interface_name, QVariantMap changed_properties, QStringList invalidated_properties)
{
    printf("hahahaha!!!!!\n");
}

int main(int argc, char *argv[])
{
#if 1
    signalTest *test;
    
    test = new signalTest();
    
    
    while (1);
    
#else
    
//    int num_room;
    QString property_interface;
    QString property_name;


//    if (argc > 2) {
//        fprintf(stderr, "Usage: %s [num_room]\n", argv[0]);
//        exit(1);
//    }
    
//    if (argc == 2) {
//        num_room = QString(argv[1]).toInt();
//    } else {
//        num_room = 1;
//    }
    
    property_interface = QString::fromUtf8("org.fcitx.Fcitx.InputMethod");
    property_name = QString::fromUtf8("IMList");
    

    // 创建QDBusInterface
    QDBusInterface iface( "org.fcitx.Fcitx",
                         "/inputmethod",
//                         "org.fcitx.Fcitx.InputMethod",
                          "org.freedesktop.DBus.Properties",
                         QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        exit(1);
    }
    
    // 呼叫远程的checkIn，参数为num_room
    //        QDBusReply<int> reply = iface.call("GetCurrentIM", num_room);
//    QDBusReply<QDBusVariant> reply = iface.call("Get", property_interface, property_name);
    QDBusMessage reply = iface.call("Get", property_interface, property_name);
    qDebug() << reply.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>().currentType();
    const QDBusArgument &dbusArgs = reply.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>();//.at(0).value<QVariant>;
#if 1
    QVariant IME_member0;
//    QVariant IME_member1;
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd())
    {
        dbusArgs >> IME_member0;// >> IME_member1;
        //dbusArgs >> path;
        // append path to a vector here if you want to keep it
        qDebug() << IME_member0;//.value<QString>() << IME_member1.value<QString>();
        break;
    }
    dbusArgs.endArray();
#endif
    
//    if (reply.isValid()) {
//        qDebug() << reply.value();
//        arry = reply.value();
////        im_name = reply.value();
////        printf("Got im name:%s\n", im_name.toStdString().c_str());
//    } else {
//        fprintf(stderr, "no reply!\n");
//    }
#endif
    return 0;
}
