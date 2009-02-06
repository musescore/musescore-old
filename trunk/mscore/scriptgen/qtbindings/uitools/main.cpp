#include <QtScript/QScriptExtensionPlugin>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtCore/QDebug>

#include <quiloader.h>

QScriptValue qtscript_create_QUiLoader_class(QScriptEngine *engine);

static const char * const qtscript_com_trolltech_qt_uitools_class_names[] = {
    "QUiLoader"
};

typedef QScriptValue (*QtBindingCreator)(QScriptEngine *engine);
static const QtBindingCreator qtscript_com_trolltech_qt_uitools_class_functions[] = {
    qtscript_create_QUiLoader_class
};

class com_trolltech_qt_uitools_ScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList com_trolltech_qt_uitools_ScriptPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("qt");
    list << QLatin1String("qt.uitools");
    return list;
}

void com_trolltech_qt_uitools_ScriptPlugin::initialize(const QString &key, QScriptEngine *engine)
{
    if (key == QLatin1String("qt")) {
    } else if (key == QLatin1String("qt.uitools")) {
        QScriptValue extensionObject = engine->globalObject();
        for (int i = 0; i < 1; ++i) {
            extensionObject.setProperty(qtscript_com_trolltech_qt_uitools_class_names[i],
                qtscript_com_trolltech_qt_uitools_class_functions[i](engine),
                QScriptValue::SkipInEnumeration);
        }
    } else {
        Q_ASSERT_X(false, "com_trolltech_qt_uitools::initialize", qPrintable(key));
    }
}
Q_EXPORT_STATIC_PLUGIN(com_trolltech_qt_uitools_ScriptPlugin)
Q_EXPORT_PLUGIN2(qtscript_com_trolltech_qt_uitools, com_trolltech_qt_uitools_ScriptPlugin)

