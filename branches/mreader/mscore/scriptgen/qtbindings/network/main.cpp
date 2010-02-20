#include <QtScript/QScriptExtensionPlugin>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtCore/QDebug>

#include <qhttp.h>
#include <qauthenticator.h>
#include <qhostaddress.h>
#include <qnetworkcookie.h>
#include <qnetworkinterface.h>
#include <qssl.h>
#include <qabstractnetworkcache.h>
#include <qhostinfo.h>
#include <qnetworkproxy.h>
#include <qnetworkproxy.h>
#include <qnetworkproxy.h>
#include <qnetworkrequest.h>
#include <qhostaddress.h>
#include <qnetworkinterface.h>
#include <qurlinfo.h>
#include <qnetworkcookie.h>
#include <qnetworkaccessmanager.h>
#include <qftp.h>
#include <qlocalserver.h>
#include <qhttp.h>
#include <qhttp.h>
#include <qabstractnetworkcache.h>
#include <qtcpserver.h>
#include <qhttp.h>
#include <qabstractsocket.h>
#include <qnetworkreply.h>
#include <qlocalsocket.h>
#include <qtcpsocket.h>
#include <qudpsocket.h>

QScriptValue qtscript_create_QHttpHeader_class(QScriptEngine *engine);
QScriptValue qtscript_create_QAuthenticator_class(QScriptEngine *engine);
QScriptValue qtscript_create_QIPv6Address_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkCookie_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkAddressEntry_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSsl_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkCacheMetaData_class(QScriptEngine *engine);
QScriptValue qtscript_create_QHostInfo_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkProxyQuery_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkProxy_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkProxyFactory_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkRequest_class(QScriptEngine *engine);
QScriptValue qtscript_create_QHostAddress_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkInterface_class(QScriptEngine *engine);
QScriptValue qtscript_create_QUrlInfo_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkCookieJar_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkAccessManager_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFtp_class(QScriptEngine *engine);
QScriptValue qtscript_create_QLocalServer_class(QScriptEngine *engine);
QScriptValue qtscript_create_QHttpResponseHeader_class(QScriptEngine *engine);
QScriptValue qtscript_create_QHttpRequestHeader_class(QScriptEngine *engine);
QScriptValue qtscript_create_QAbstractNetworkCache_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTcpServer_class(QScriptEngine *engine);
QScriptValue qtscript_create_QHttp_class(QScriptEngine *engine);
QScriptValue qtscript_create_QAbstractSocket_class(QScriptEngine *engine);
QScriptValue qtscript_create_QNetworkReply_class(QScriptEngine *engine);
QScriptValue qtscript_create_QLocalSocket_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTcpSocket_class(QScriptEngine *engine);
QScriptValue qtscript_create_QUdpSocket_class(QScriptEngine *engine);

static const char * const qtscript_com_trolltech_qt_network_class_names[] = {
    "QHttpHeader"
    , "QAuthenticator"
    , "QIPv6Address"
    , "QNetworkCookie"
    , "QNetworkAddressEntry"
    , "QSsl"
    , "QNetworkCacheMetaData"
    , "QHostInfo"
    , "QNetworkProxyQuery"
    , "QNetworkProxy"
    , "QNetworkProxyFactory"
    , "QNetworkRequest"
    , "QHostAddress"
    , "QNetworkInterface"
    , "QUrlInfo"
    , "QNetworkCookieJar"
    , "QNetworkAccessManager"
    , "QFtp"
    , "QLocalServer"
    , "QHttpResponseHeader"
    , "QHttpRequestHeader"
    , "QAbstractNetworkCache"
    , "QTcpServer"
    , "QHttp"
    , "QAbstractSocket"
    , "QNetworkReply"
    , "QLocalSocket"
    , "QTcpSocket"
    , "QUdpSocket"
};

typedef QScriptValue (*QtBindingCreator)(QScriptEngine *engine);
static const QtBindingCreator qtscript_com_trolltech_qt_network_class_functions[] = {
    qtscript_create_QHttpHeader_class
    , qtscript_create_QAuthenticator_class
    , qtscript_create_QIPv6Address_class
    , qtscript_create_QNetworkCookie_class
    , qtscript_create_QNetworkAddressEntry_class
    , qtscript_create_QSsl_class
    , qtscript_create_QNetworkCacheMetaData_class
    , qtscript_create_QHostInfo_class
    , qtscript_create_QNetworkProxyQuery_class
    , qtscript_create_QNetworkProxy_class
    , qtscript_create_QNetworkProxyFactory_class
    , qtscript_create_QNetworkRequest_class
    , qtscript_create_QHostAddress_class
    , qtscript_create_QNetworkInterface_class
    , qtscript_create_QUrlInfo_class
    , qtscript_create_QNetworkCookieJar_class
    , qtscript_create_QNetworkAccessManager_class
    , qtscript_create_QFtp_class
    , qtscript_create_QLocalServer_class
    , qtscript_create_QHttpResponseHeader_class
    , qtscript_create_QHttpRequestHeader_class
    , qtscript_create_QAbstractNetworkCache_class
    , qtscript_create_QTcpServer_class
    , qtscript_create_QHttp_class
    , qtscript_create_QAbstractSocket_class
    , qtscript_create_QNetworkReply_class
    , qtscript_create_QLocalSocket_class
    , qtscript_create_QTcpSocket_class
    , qtscript_create_QUdpSocket_class
};

class com_trolltech_qt_network_ScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList com_trolltech_qt_network_ScriptPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("qt");
    list << QLatin1String("qt.network");
    return list;
}

void com_trolltech_qt_network_ScriptPlugin::initialize(const QString &key, QScriptEngine *engine)
{
    if (key == QLatin1String("qt")) {
    } else if (key == QLatin1String("qt.network")) {
        QScriptValue extensionObject = engine->globalObject();
        for (int i = 0; i < 29; ++i) {
            extensionObject.setProperty(qtscript_com_trolltech_qt_network_class_names[i],
                qtscript_com_trolltech_qt_network_class_functions[i](engine),
                QScriptValue::SkipInEnumeration);
        }
    } else {
        Q_ASSERT_X(false, "com_trolltech_qt_network::initialize", qPrintable(key));
    }
}
Q_EXPORT_STATIC_PLUGIN(com_trolltech_qt_network_ScriptPlugin)
Q_EXPORT_PLUGIN2(qtscript_com_trolltech_qt_network, com_trolltech_qt_network_ScriptPlugin)

