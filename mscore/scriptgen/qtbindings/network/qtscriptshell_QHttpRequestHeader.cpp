#include "qtscriptshell_QHttpRequestHeader.h"

#include <QtScript/QScriptEngine>
#include <QVariant>
#include <qhttp.h>

#define QTSCRIPT_IS_GENERATED_FUNCTION(fun) ((fun.data().toUInt32() & 0xFFFF0000) == 0xBABE0000)


QtScriptShell_QHttpRequestHeader::QtScriptShell_QHttpRequestHeader()
    : QHttpRequestHeader() {}

QtScriptShell_QHttpRequestHeader::QtScriptShell_QHttpRequestHeader(const QHttpRequestHeader&  header)
    : QHttpRequestHeader(header) {}

QtScriptShell_QHttpRequestHeader::QtScriptShell_QHttpRequestHeader(const QString&  method, const QString&  path, int  majorVer, int  minorVer)
    : QHttpRequestHeader(method, path, majorVer, minorVer) {}

QtScriptShell_QHttpRequestHeader::QtScriptShell_QHttpRequestHeader(const QString&  str)
    : QHttpRequestHeader(str) {}

QtScriptShell_QHttpRequestHeader::~QtScriptShell_QHttpRequestHeader() {}

