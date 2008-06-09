/****************************************************************************
**
** Copyright (C) 2007-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Script Debug project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTBREAKPOINTINFO_H
#define QSCRIPTBREAKPOINTINFO_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobjectdefs.h>

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

class QDataStream;

class QScriptBreakpointInfoPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptBreakpointInfo
{
public:
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptBreakpointInfo &);
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptBreakpointInfo &);

    QScriptBreakpointInfo(qint64 scriptId, int lineNumber);
    QScriptBreakpointInfo(const QString &fileName, int lineNumber);
    QScriptBreakpointInfo();
    QScriptBreakpointInfo(const QScriptBreakpointInfo &other);
    ~QScriptBreakpointInfo();

    qint64 scriptId() const;
    void setScriptId(qint64 id);

    QString fileName() const;
    void setFileName(const QString &fileName);

    int lineNumber() const;
    void setLineNumber(int lineNumber);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    bool isSingleShot() const;
    void setSingleShot(bool singleShot);

    int ignoreCount() const;
    void setIgnoreCount(int count);

    QString condition() const;
    void setCondition(const QString &condition);

    QVariant data() const;
    void setData(const QVariant &data);

    int hitCount() const;
    void bumpHitCount();

    QScriptBreakpointInfo &operator=(const QScriptBreakpointInfo &other);

private:
    QScriptBreakpointInfoPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptBreakpointInfo)
};

typedef QList<QScriptBreakpointInfo> QScriptBreakpointInfoList;

Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptBreakpointInfo &);
Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptBreakpointInfo &);

#endif
