#include <QtScript/QScriptExtensionPlugin>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtCore/QDebug>

#include <qtscriptconcurrent.h>
#include <qurl.h>
#include <qrect.h>
#include <qsize.h>
#include <qsize.h>
#include <qxmlstream.h>
#include <qtconcurrentexception.h>
#include <qsystemsemaphore.h>
#include <qdiriterator.h>
#include <qtscriptconcurrent.h>
#include <qtextstream.h>
#include <qtscriptconcurrent.h>
#include <qtextcodec.h>
#include <qsemaphore.h>
#include <qtscriptconcurrent.h>
#include <qxmlstream.h>
#include <qdatetime.h>
#include <qxmlstream.h>
#include <qxmlstream.h>
#include <qmutex.h>
#include <quuid.h>
#include <qdatastream.h>
#include <qxmlstream.h>
#include <qbytearraymatcher.h>
#include <qrunnable.h>
#include <qbytearray.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qxmlstream.h>
#include <qnamespace.h>
#include <qcoreevent.h>
#include <qlibraryinfo.h>
#include <qpoint.h>
#include <qobject.h>
#include <qpoint.h>
#include <qstringmatcher.h>
#include <qwaitcondition.h>
#include <qxmlstream.h>
#include <qbitarray.h>
#include <qdir.h>
#include <qrect.h>
#include <qtscriptconcurrent.h>
#include <qtscriptconcurrent.h>
#include <qtextboundaryfinder.h>
#include <qtscriptconcurrent.h>
#include <qbasictimer.h>
#include <qtextcodec.h>
#include <qxmlstream.h>
#include <qlocale.h>
#include <qcryptographichash.h>
#include <qsignalmapper.h>
#include <qfilesystemwatcher.h>
#include <qtimeline.h>
#include <qcoreevent.h>
#include <qiodevice.h>
#include <qcoreapplication.h>
#include <qthreadpool.h>
#include <qeventloop.h>
#include <qtranslator.h>
#include <qsettings.h>
#include <qcoreevent.h>
#include <qcoreevent.h>
#include <qtimer.h>
#include <qsocketnotifier.h>
#include <qbuffer.h>
#include <qfile.h>
#include <qprocess.h>
#include <qtemporaryfile.h>

QScriptValue qtscript_create_QFutureWatcherVoid_class(QScriptEngine *engine);
QScriptValue qtscript_create_QUrl_class(QScriptEngine *engine);
QScriptValue qtscript_create_QRectF_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSize_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSizeF_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamNotationDeclaration_class(QScriptEngine *engine);
QScriptValue qtscript_create_QtConcurrent_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSystemSemaphore_class(QScriptEngine *engine);
QScriptValue qtscript_create_QDirIterator_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFutureIterator_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTextStream_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFutureVoid_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTextCodec_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSemaphore_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFuture_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamAttributes_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTime_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamNamespaceDeclaration_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamReader_class(QScriptEngine *engine);
QScriptValue qtscript_create_QMutex_class(QScriptEngine *engine);
QScriptValue qtscript_create_QUuid_class(QScriptEngine *engine);
QScriptValue qtscript_create_QDataStream_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamWriter_class(QScriptEngine *engine);
QScriptValue qtscript_create_QByteArrayMatcher_class(QScriptEngine *engine);
QScriptValue qtscript_create_QRunnable_class(QScriptEngine *engine);
QScriptValue qtscript_create_QByteArray_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFileInfo_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTextDecoder_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamEntityResolver_class(QScriptEngine *engine);
QScriptValue qtscript_create_Qt_class(QScriptEngine *engine);
QScriptValue qtscript_create_QEvent_class(QScriptEngine *engine);
QScriptValue qtscript_create_QLibraryInfo_class(QScriptEngine *engine);
QScriptValue qtscript_create_QPoint_class(QScriptEngine *engine);
QScriptValue qtscript_create_QObject_class(QScriptEngine *engine);
QScriptValue qtscript_create_QPointF_class(QScriptEngine *engine);
QScriptValue qtscript_create_QStringMatcher_class(QScriptEngine *engine);
QScriptValue qtscript_create_QWaitCondition_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamEntityDeclaration_class(QScriptEngine *engine);
QScriptValue qtscript_create_QBitArray_class(QScriptEngine *engine);
QScriptValue qtscript_create_QDir_class(QScriptEngine *engine);
QScriptValue qtscript_create_QRect_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFutureSynchronizer_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFutureSynchronizerVoid_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTextBoundaryFinder_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFutureWatcher_class(QScriptEngine *engine);
QScriptValue qtscript_create_QBasicTimer_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTextEncoder_class(QScriptEngine *engine);
QScriptValue qtscript_create_QXmlStreamAttribute_class(QScriptEngine *engine);
QScriptValue qtscript_create_QLocale_class(QScriptEngine *engine);
QScriptValue qtscript_create_QCryptographicHash_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSignalMapper_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFileSystemWatcher_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTimeLine_class(QScriptEngine *engine);
QScriptValue qtscript_create_QChildEvent_class(QScriptEngine *engine);
QScriptValue qtscript_create_QIODevice_class(QScriptEngine *engine);
QScriptValue qtscript_create_QCoreApplication_class(QScriptEngine *engine);
QScriptValue qtscript_create_QThreadPool_class(QScriptEngine *engine);
QScriptValue qtscript_create_QEventLoop_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTranslator_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSettings_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTimerEvent_class(QScriptEngine *engine);
QScriptValue qtscript_create_QDynamicPropertyChangeEvent_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTimer_class(QScriptEngine *engine);
QScriptValue qtscript_create_QSocketNotifier_class(QScriptEngine *engine);
QScriptValue qtscript_create_QBuffer_class(QScriptEngine *engine);
QScriptValue qtscript_create_QFile_class(QScriptEngine *engine);
QScriptValue qtscript_create_QProcess_class(QScriptEngine *engine);
QScriptValue qtscript_create_QTemporaryFile_class(QScriptEngine *engine);

static const char * const qtscript_com_trolltech_qt_core_class_names[] = {
    "QFutureWatcherVoid"
    , "QUrl"
    , "QRectF"
    , "QSize"
    , "QSizeF"
    , "QXmlStreamNotationDeclaration"
    , "QtConcurrent"
    , "QSystemSemaphore"
    , "QDirIterator"
    , "QFutureIterator"
    , "QTextStream"
    , "QFutureVoid"
    , "QTextCodec"
    , "QSemaphore"
    , "QFuture"
    , "QXmlStreamAttributes"
    , "QTime"
    , "QXmlStreamNamespaceDeclaration"
    , "QXmlStreamReader"
    , "QMutex"
    , "QUuid"
    , "QDataStream"
    , "QXmlStreamWriter"
    , "QByteArrayMatcher"
    , "QRunnable"
    , "QByteArray"
    , "QFileInfo"
    , "QTextDecoder"
    , "QXmlStreamEntityResolver"
    , "Qt"
    , "QEvent"
    , "QLibraryInfo"
    , "QPoint"
    , "QObject"
    , "QPointF"
    , "QStringMatcher"
    , "QWaitCondition"
    , "QXmlStreamEntityDeclaration"
    , "QBitArray"
    , "QDir"
    , "QRect"
    , "QFutureSynchronizer"
    , "QFutureSynchronizerVoid"
    , "QTextBoundaryFinder"
    , "QFutureWatcher"
    , "QBasicTimer"
    , "QTextEncoder"
    , "QXmlStreamAttribute"
    , "QLocale"
    , "QCryptographicHash"
    , "QSignalMapper"
    , "QFileSystemWatcher"
    , "QTimeLine"
    , "QChildEvent"
    , "QIODevice"
    , "QCoreApplication"
    , "QThreadPool"
    , "QEventLoop"
    , "QTranslator"
    , "QSettings"
    , "QTimerEvent"
    , "QDynamicPropertyChangeEvent"
    , "QTimer"
    , "QSocketNotifier"
    , "QBuffer"
    , "QFile"
    , "QProcess"
    , "QTemporaryFile"
};

typedef QScriptValue (*QtBindingCreator)(QScriptEngine *engine);
static const QtBindingCreator qtscript_com_trolltech_qt_core_class_functions[] = {
    qtscript_create_QFutureWatcherVoid_class
    , qtscript_create_QUrl_class
    , qtscript_create_QRectF_class
    , qtscript_create_QSize_class
    , qtscript_create_QSizeF_class
    , qtscript_create_QXmlStreamNotationDeclaration_class
    , qtscript_create_QtConcurrent_class
    , qtscript_create_QSystemSemaphore_class
    , qtscript_create_QDirIterator_class
    , qtscript_create_QFutureIterator_class
    , qtscript_create_QTextStream_class
    , qtscript_create_QFutureVoid_class
    , qtscript_create_QTextCodec_class
    , qtscript_create_QSemaphore_class
    , qtscript_create_QFuture_class
    , qtscript_create_QXmlStreamAttributes_class
    , qtscript_create_QTime_class
    , qtscript_create_QXmlStreamNamespaceDeclaration_class
    , qtscript_create_QXmlStreamReader_class
    , qtscript_create_QMutex_class
    , qtscript_create_QUuid_class
    , qtscript_create_QDataStream_class
    , qtscript_create_QXmlStreamWriter_class
    , qtscript_create_QByteArrayMatcher_class
    , qtscript_create_QRunnable_class
    , qtscript_create_QByteArray_class
    , qtscript_create_QFileInfo_class
    , qtscript_create_QTextDecoder_class
    , qtscript_create_QXmlStreamEntityResolver_class
    , qtscript_create_Qt_class
    , qtscript_create_QEvent_class
    , qtscript_create_QLibraryInfo_class
    , qtscript_create_QPoint_class
    , qtscript_create_QObject_class
    , qtscript_create_QPointF_class
    , qtscript_create_QStringMatcher_class
    , qtscript_create_QWaitCondition_class
    , qtscript_create_QXmlStreamEntityDeclaration_class
    , qtscript_create_QBitArray_class
    , qtscript_create_QDir_class
    , qtscript_create_QRect_class
    , qtscript_create_QFutureSynchronizer_class
    , qtscript_create_QFutureSynchronizerVoid_class
    , qtscript_create_QTextBoundaryFinder_class
    , qtscript_create_QFutureWatcher_class
    , qtscript_create_QBasicTimer_class
    , qtscript_create_QTextEncoder_class
    , qtscript_create_QXmlStreamAttribute_class
    , qtscript_create_QLocale_class
    , qtscript_create_QCryptographicHash_class
    , qtscript_create_QSignalMapper_class
    , qtscript_create_QFileSystemWatcher_class
    , qtscript_create_QTimeLine_class
    , qtscript_create_QChildEvent_class
    , qtscript_create_QIODevice_class
    , qtscript_create_QCoreApplication_class
    , qtscript_create_QThreadPool_class
    , qtscript_create_QEventLoop_class
    , qtscript_create_QTranslator_class
    , qtscript_create_QSettings_class
    , qtscript_create_QTimerEvent_class
    , qtscript_create_QDynamicPropertyChangeEvent_class
    , qtscript_create_QTimer_class
    , qtscript_create_QSocketNotifier_class
    , qtscript_create_QBuffer_class
    , qtscript_create_QFile_class
    , qtscript_create_QProcess_class
    , qtscript_create_QTemporaryFile_class
};

class com_trolltech_qt_core_ScriptPlugin : public QScriptExtensionPlugin
{
public:
    QStringList keys() const;
    void initialize(const QString &key, QScriptEngine *engine);
};

QStringList com_trolltech_qt_core_ScriptPlugin::keys() const
{
    QStringList list;
    list << QLatin1String("qt");
    list << QLatin1String("qt.core");
    return list;
}

void com_trolltech_qt_core_ScriptPlugin::initialize(const QString &key, QScriptEngine *engine)
{
    if (key == QLatin1String("qt")) {
    } else if (key == QLatin1String("qt.core")) {
        QScriptValue extensionObject = engine->globalObject();
        for (int i = 0; i < 68; ++i) {
            extensionObject.setProperty(qtscript_com_trolltech_qt_core_class_names[i],
                qtscript_com_trolltech_qt_core_class_functions[i](engine),
                QScriptValue::SkipInEnumeration);
        }
    } else {
        Q_ASSERT_X(false, "com_trolltech_qt_core::initialize", qPrintable(key));
    }
}
Q_EXPORT_STATIC_PLUGIN(com_trolltech_qt_core_ScriptPlugin)
Q_EXPORT_PLUGIN2(qtscript_com_trolltech_qt_core, com_trolltech_qt_core_ScriptPlugin)

