/*
 * Example low-level D-Bus code.
 * Written by Matthew Johnson <dbus@matthew.ath.cx>
 *
 * This code has been released into the Public Domain.
 * You may do whatever you like with it.
 *
 * Subsequent tweaks by Will Ware <wware@alum.mit.edu>
 * Still in the public domain.
 */
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Qt/qlist.h>
#include <Qt/qvariant.h>
#include <Qt/QtGui>
/**
 * Connect to the DBUS bus and send a broadcast signal
 */
void sendsignal(char* sigvalue)
{
    DBusMessage* msg;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    int ret;
    dbus_uint32_t serial = 0;
    
    printf("Sending signal with value %s\n", sigvalue);
    
    // initialise the error value
    dbus_error_init(&err);
    
    // connect to the DBUS system bus, and check for errors
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        exit(1);
    }
    
    // register our name on the bus, and check for errors
    ret = dbus_bus_request_name(conn, "test.signal.source", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
        exit(1);
    }
    
    // create a signal & check for errors
    msg = dbus_message_new_signal("/test/signal/Object", // object name of the signal
                                  "test.signal.Type", // interface name of the signal
                                  "Test"); // name of the signal
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
    
    // append arguments onto signal
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &sigvalue)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    
    // send the message and flush the connection
    if (!dbus_connection_send(conn, msg, &serial)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    dbus_connection_flush(conn);
    
    printf("Signal Sent\n");
    
    // free the message
    dbus_message_unref(msg);
}

/**
 * Call a method on a remote object
 */
bool queryDbus(const char* method, const char* param, char* replyStr)
{
    DBusMessage* msg;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    DBusPendingCall* pending;
    int ret;
    char* reply = NULL;
    
    
    printf("Calling remote method with %s\n", param);
    
    // initialiset the errors
    dbus_error_init(&err);
    
    // connect to the system bus and check for errors
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        return false;
    }
    
    // request our name on the bus
    ret = dbus_bus_request_name(conn, "test.xt.daemon", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    
    // create a new method call and check for errors
    msg = dbus_message_new_method_call("org.fcitx.Fcitx", // target for the method call
                                       "/inputmethod", // object to call on
                                       "org.fcitx.Fcitx.InputMethod", // interface to call on
                                       method); // method name
    if (NULL == msg) {
        fprintf(stderr, "Message Null\n");
        return false;
    }
    // append arguments
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param)) {
        fprintf(stderr, "Out Of Memory!\n");
        return false;
    }
    
    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
        fprintf(stderr, "Out Of Memory!\n");
        return false;
    }
    if (NULL == pending) {
        fprintf(stderr, "Pending Call Null\n");
        return false;
    }
    dbus_connection_flush(conn);
    
    printf("Request Sent\n");
    
    // free message
    dbus_message_unref(msg);
    
    // block until we recieve a reply
    dbus_pending_call_block(pending);
    
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
        fprintf(stderr, "Reply Null\n");
        return false;
    }
    // free the pending message handle
    dbus_pending_call_unref(pending);
    
    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
        fprintf(stderr, "Argument is not string!\n");
    else
        dbus_message_iter_get_basic(&args, &reply);
    
    //printf("Got Reply: %s\n", zz_string);
    if ((replyStr != NULL) && (reply != NULL))
    {
        strcpy(replyStr, reply);
    }
    // free reply
    dbus_message_unref(msg);
    return true;
}

void query(char* method, char* param)
{

    DBusMessage* msg;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    DBusPendingCall* pending;
    int ret;
    bool stat;
    char *zz_string = NULL;
    dbus_uint32_t level;
    
    printf("Calling remote method with %s\n", param);
    
    // initialiset the errors
    dbus_error_init(&err);
    
    // connect to the system bus and check for errors
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        printf("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        exit(1);
    }
    
    // request our name on the bus
    ret = dbus_bus_request_name(conn, "test.xt.daemon", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        printf("Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
//    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
//        exit(1);
//    }
    
//    org.freedesktop.DBus.Properties.Get org.fcitx.Fcitx.InputMethod IMList
    
    // create a new method call and check for errors
    msg = dbus_message_new_method_call("org.fcitx.Fcitx", // target for the method call
                                       "/inputmethod", // object to call on
                                       "org.freedesktop.DBus.Properties", // interface to call on
                                       "Get"); // method name
    if (NULL == msg) {
        printf("Message Null\n");
        exit(1);
    }
    // append arguments
    dbus_message_iter_init_append(msg, &args);
    
 #if 0
    const char* array[] = {"org.fcitx.Fcitx.InputMethod", "IMList"};
    const char* *v_Array = array;
    if (!dbus_message_iter_append_fixed_array (&args, DBUS_TYPE_STRING, &v_Array, 2)) {
        printf("Out Of Memory!\n");
        exit(1);
    }
#else
    char *param_ = "org.fcitx.Fcitx.InputMethod IMList";
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param_)) {
        printf("Out Of Memory!\n");
        exit(1);
    }
//    param_ = "IMList";
//    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param_)) {
//        printf("Out Of Memory!\n");
//        exit(1);
//    }
#endif
    
    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1)) { // -1 is default timeout
        printf("Out Of Memory!\n");
        exit(1);
    }
    if (NULL == pending) {
        printf("Pending Call Null\n");
        exit(1);
    }
    dbus_connection_flush(conn);
    
    printf("Request Sent\n");
    
    // free message
    dbus_message_unref(msg);
    
    // block until we recieve a reply
    dbus_pending_call_block(pending);
    
    // get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg) {
        fprintf(stderr, "Reply Null\n");
        exit(1);
    }
    // free the pending message handle
    dbus_pending_call_unref(pending);
#if 1
#if 0
//    dbus_message_iter_init(msg, &args);
//    QList<QVariant> outArgs = msg->arguments();
//    QString str="welcome";
//    qDebug()<<str;
//    qDebug() << *msg;
    
#else
    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
        fprintf(stderr, "Argument is not string!\n");
    else
        dbus_message_iter_get_basic(&args, &zz_string);
#endif
    printf("Got Reply: %s\n", zz_string);
#else
    // read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    else if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args))
        fprintf(stderr, "Argument is not boolean!\n");
    else
        dbus_message_iter_get_basic(&args, &stat);
    
    if (!dbus_message_iter_next(&args))
        fprintf(stderr, "Message has too few arguments!\n");
    else if (DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&args))
        fprintf(stderr, "Argument is not int!\n");
    else
        dbus_message_iter_get_basic(&args, &level);
    
    printf("Got Reply: %d, %d\n", stat, level);
#endif
    // free reply
    dbus_message_unref(msg);
}

void reply_to_method_call(DBusMessage* msg, DBusConnection* conn)
{
    DBusMessage* reply;
    DBusMessageIter args;
    bool stat = true;
    dbus_uint32_t level = 21614;
    dbus_uint32_t serial = 0;
    char* param = "";
    
    // read the arguments
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "Message has no arguments!\n");
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
        fprintf(stderr, "Argument is not string!\n");
    else
        dbus_message_iter_get_basic(&args, &param);
    
    printf("Method called with %s\n", param);
    
    // create a reply from the message
    reply = dbus_message_new_method_return(msg);
    
    // add the arguments to the reply
    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &stat)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &level)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    
    // send the reply && flush the connection
    if (!dbus_connection_send(conn, reply, &serial)) {
        fprintf(stderr, "Out Of Memory!\n");
        exit(1);
    }
    dbus_connection_flush(conn);
    
    // free the reply
    dbus_message_unref(reply);
}

/**
 * Server that exposes a method call and waits for it to be called
 */
void listen()
{
    DBusMessage* msg;
    DBusMessage* reply;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    int ret;
    char* param;
    
    printf("Listening for method calls\n");
    // initialise the error
    dbus_error_init(&err);
    
    // connect to the bus and check for errors
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        fprintf(stderr, "Connection Null\n");
        exit(1);
    }
    
    // request our name on the bus and check for errors
    ret = dbus_bus_request_name(conn, "test.method.server", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
        fprintf(stderr, "Not Primary Owner (%d)\n", ret);
        exit(1);
    }
    
    // loop, testing for new messages
    while (true) {
        // non blocking read of the next available message
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);
        
        // loop again if we haven't got a message
        if (NULL == msg) {
            usleep(10000);
            continue;
        }
        
        // check this is a method call for the right interface & method
        if (dbus_message_is_method_call(msg, "test.method.Type", "Method"))
            reply_to_method_call(msg, conn);
        
        // free the message
        dbus_message_unref(msg);
    }
    
}

/**
 * Listens for signals on the bus
 */
void receive()
{
    DBusMessage* msg;
    DBusMessageIter args;
    DBusConnection* conn;
    DBusError err;
    int ret;
    char* sigvalue;
    
    printf("Listening for signals\n");
    
    // initialise the errors
    dbus_error_init(&err);
    
    // connect to the bus and check for errors
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (NULL == conn) {
        exit(1);
    }
    
    // request our name on the bus and check for errors
    ret = dbus_bus_request_name(conn, "test.signal.sink", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Name Error (%s)\n", err.message);
        dbus_error_free(&err);
    }
    if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
        exit(1);
    }
    
    // add a rule for which messages we want to see
    dbus_bus_add_match(conn, "type='signal',interface='test.signal.Type'", &err); // see signals from the given interface
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match Error (%s)\n", err.message);
        exit(1);
    }
    printf("Match rule sent\n");
    
    // loop listening for signals being emmitted
    while (true) {
        
        // non blocking read of the next available message
        dbus_connection_read_write(conn, 0);
        msg = dbus_connection_pop_message(conn);
        
        // loop again if we haven't read a message
        if (NULL == msg) {
            usleep(10000);
            continue;
        }
        
        // check if the message is a signal from the correct interface and with the correct name
        if (dbus_message_is_signal(msg, "test.signal.Type", "Test")) {
            
            // read the parameters
            if (!dbus_message_iter_init(msg, &args))
                fprintf(stderr, "Message Has No Parameters\n");
            else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
                fprintf(stderr, "Argument is not string!\n");
            else
                dbus_message_iter_get_basic(&args, &sigvalue);
            
            printf("Got Signal with value %s\n", sigvalue);
        }
        
        // free the message
        dbus_message_unref(msg);
    }
}

#define IME_SOGOU_PINYIN "sogoupinyin"
#define IME_SOGOU_WUBI   "wubi"
#define IME_US_Keyboard  "fcitx-keyboard-us"

int main(int argc, char** argv)
{
#if 1
    //char buffer[255] = { 0 };
    char *method = "";
    char *params = "org.fcitx.Fcitx.InputMethod IMList";
    if (argc > 1) method = argv[1];
    if (argc > 2) params = argv[2];
    
    query(method, params);
//    queryDbus(method, params, buffer);
    
//    printf("feedback: %s \n", buffer);
    
    
//    char buffer[255] = { 0 };
#if 0
    queryDbus("SetCurrentIM", IME_SOGOU_PINYIN, NULL);
    queryDbus("GetCurrentIM", "", buffer);
    if (strstr(buffer, IME_SOGOU_PINYIN))
    {
        printf("find sogou pinyin IME\n");
//        LOG4CXX_DEBUG(AppLogger, "find sogou pinyin IME");
//        strcpy(ImeInfo.tcName, buffer);
//        IMEList.push_back(ID_SOGOU_PINYIN);
//        IMETable.insert(make_pair(ID_SOGOU_PINYIN, ImeInfo));
    }
    queryDbus("SetCurrentIM", IME_SOGOU_WUBI, NULL);
    queryDbus("GetCurrentIM", "", buffer);
    if (strstr(buffer, IME_SOGOU_WUBI))
    {
        printf("find sogou wubi IME\n");
//        LOG4CXX_DEBUG(AppLogger, "find sogou wubi IME");
//        strcpy(ImeInfo.tcName, buffer);
//        IMEList.push_back(ID_SOGOU_WUBI);
//        IMETable.insert(make_pair(ID_SOGOU_WUBI, ImeInfo));
    }
    queryDbus("SetCurrentIM", IME_US_Keyboard, NULL);
    queryDbus("GetCurrentIM", "", buffer);
    if (strstr(buffer, IME_US_Keyboard))
    {
        printf("find US keyborad IME\n");
//        LOG4CXX_DEBUG(AppLogger, "find US keyborad IME");
//        strcpy(ImeInfo.tcName, buffer);
//        IMEList.push_back(ID_US_Keyboard);
//        IMETable.insert(make_pair(ID_US_Keyboard, ImeInfo));
    }
#endif
    printf("end process!");
#else
    if (2 > argc) {
        printf ("Syntax: dbus-example [send|receive|listen|query] [<param>]\n");
        return 1;
    }
    char* param = "no param";
    if (3 >= argc && NULL != argv[2]) param = argv[2];
    if (0 == strcmp(argv[1], "send"))
        sendsignal(param);
    else if (0 == strcmp(argv[1], "receive"))
        receive();
    else if (0 == strcmp(argv[1], "listen"))
        listen();
    else if (0 == strcmp(argv[1], "query"))
        query(param);
    else {
        printf ("Syntax: dbus-example [send|receive|listen|query] [<param>]\n");
        return 1;
    }
    return 0;
#endif
}
