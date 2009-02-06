#include "qtscriptshell_QHttpResponseHeader.h"

#include <QtScript/QScriptEngine>
#include <QVariant>
#include <qhttp.h>

#define QTSCRIPT_IS_GENERATED_FUNCTION(fun) ((fun.data().toUInt32() & 0xFFFF0000) == 0xBABE0000)


QtScriptShell_QHttpResponseHeader::QtScriptShell_QHttpResponseHeader()
    : QHttpResponseHeader() {}

QtScriptShell_QHttpResponseHeader::QtScriptShell_QHttpResponseHeader(const QHttpResponseHeader&  header)
    : QHttpResponseHeader(header) {}

QtScriptShell_QHttpResponseHeader::QtScriptShell_QHttpResponseHeader(const QString&  str)
    : QHttpResponseHeader(str) {}

QtScriptShell_QHttpResponseHeader::QtScriptShell_QHttpResponseHeader(int  code, const QString&  text, int  majorVer, int  minorVer)
    : QHttpResponseHeader(code, text, majorVer, minorVer) {}

QtScriptShell_QHttpResponseHeader::~QtScriptShell_QHttpResponseHeader() {}

