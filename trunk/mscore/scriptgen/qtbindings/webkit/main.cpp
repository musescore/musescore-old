#include <QtScript/QScriptExtensionPlugin>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtCore/QDebug>

#include <qwebframe.h>
#include <qwebsettings.h>
#include <qwebpluginfactory.h>
#include <qwebhistoryinterface.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <qwebview.h>

QScriptValue qtscript_create_QWebHitTestResult_class(QScriptEngine *engine);
QScriptValue qtscript_create_QWebSettings_class(QScriptEngine *engine);
QScriptValue qtscript_create_QWebPluginFactory_class(QScriptEngine *engine);
QScriptValue qtscript_create_QWebHistoryInterface_class(QScriptEngine *engine);
QScriptValue qtscript_create_QWebPage_class(QScriptEngine *engine);
QScriptValue qtscript_create_QWebFrame_class(QScriptEngine *engine);
QScriptValue qtscript_create_QWebView_class(QScriptEngine *engine);

static const char * const qtscript_com_trolltech_qt_webkit_class_names[] = {
    "QWebHitTestResult"
    , "QWebSettings"
    , "QWebPluginFactory"
    , "QWebHistoryInterface"
    , "QWebPage"
    , "QWebFrame"
    , "QWebView"
};

typedef QScriptValue (*QtBindingCreator)(QScriptEngine *engine);
static const QtBindingCreator qtscript_com_trolltech_qt_webkit_class_functions[] = {
    qtscript_create_QWebHitTestResult_class
    , qtscript_create_QWebSettings_class
    , qtscript_create_QWebPluginFactory_class
    , qtscript_create_QWebHistoryInterface_class
    , qtscript_create_QWebPage_class
    , qtscript_create_QWebFrame_class
    , qtscript_create_QWebView_class
};

class com_trolltech_qt_webkit_ScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList com_trolltech_qt_webkit_ScriptPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("qt");
    list << QLatin1String("qt.webkit");
    return list;
}

void com_trolltech_qt_webkit_ScriptPlugin::initialize(const QString &key, QScriptEngine *engine)
{
    if (key == QLatin1String("qt")) {
    } else if (key == QLatin1String("qt.webkit")) {
        QScriptValue extensionObject = engine->globalObject();
        for (int i = 0; i < 7; ++i) {
            extensionObject.setProperty(qtscript_com_trolltech_qt_webkit_class_names[i],
                qtscript_com_trolltech_qt_webkit_class_functions[i](engine),
                QScriptValue::SkipInEnumeration);
        }
    } else {
        Q_ASSERT_X(false, "com_trolltech_qt_webkit::initialize", qPrintable(key));
    }
}
Q_EXPORT_STATIC_PLUGIN(com_trolltech_qt_webkit_ScriptPlugin)
Q_EXPORT_PLUGIN2(qtscript_com_trolltech_qt_webkit, com_trolltech_qt_webkit_ScriptPlugin)

