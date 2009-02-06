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

#include "qscriptbreakpointinfo.h"

#include <QtCore/qatomic.h>
#include <QtCore/qdatastream.h>

/*!
  \class QScriptBreakpointInfo

  \brief The QScriptBreakpointInfo class contains information about a script breakpoint.

  QScriptBreakpointInfo is used to store information about a breakpoint, such
  as the filename and line number it applies to, and whether it is enabled.
*/

class QScriptBreakpointInfoPrivate
{
public:
    QScriptBreakpointInfoPrivate();
    ~QScriptBreakpointInfoPrivate();

    qint64 scriptId;
    QString fileName;
    int lineNumber;
    bool enabled;
    bool singleShot;
    int ignoreCount; 
    QString condition;
    QVariant data;
    int hitCount;
};

QScriptBreakpointInfoPrivate::QScriptBreakpointInfoPrivate()
{
    scriptId = -1;
    lineNumber = -1;
    enabled = true;
    singleShot = false;
    ignoreCount = 0;
    hitCount = 0;
}

QScriptBreakpointInfoPrivate::~QScriptBreakpointInfoPrivate()
{
}

/*!
  Constructs a QScriptBreakpointInfo with the given \a fileName and \a
  lineNumber.
*/
QScriptBreakpointInfo::QScriptBreakpointInfo(const QString &fileName, int lineNumber)
    : d_ptr(new QScriptBreakpointInfoPrivate)
{
    d_ptr->fileName = fileName;
    d_ptr->lineNumber = lineNumber;
}

/*!
  Constructs a QScriptBreakpointInfo with the given \a scriptId and \a
  lineNumber.
*/
QScriptBreakpointInfo::QScriptBreakpointInfo(qint64 scriptId, int lineNumber)
    : d_ptr(new QScriptBreakpointInfoPrivate)
{
    d_ptr->scriptId = scriptId;
    d_ptr->lineNumber = lineNumber;
}

/*!
  Constructs a QScriptBreakpointInfo with no information.
*/
QScriptBreakpointInfo::QScriptBreakpointInfo()
    : d_ptr(new QScriptBreakpointInfoPrivate)
{
}

/*!
  Constructs a QScriptBreakpointInfo that is a copy of the \a other value.
*/
QScriptBreakpointInfo::QScriptBreakpointInfo(const QScriptBreakpointInfo &other)
    : d_ptr(new QScriptBreakpointInfoPrivate)
{
    d_ptr->scriptId = other.d_ptr->scriptId;
    d_ptr->fileName = other.d_ptr->fileName;
    d_ptr->lineNumber = other.d_ptr->lineNumber;
    d_ptr->enabled = other.d_ptr->enabled;
    d_ptr->singleShot = other.d_ptr->singleShot;
    d_ptr->ignoreCount = other.d_ptr->ignoreCount;
    d_ptr->condition = other.d_ptr->condition;
    d_ptr->hitCount = other.d_ptr->hitCount;
    d_ptr->data = other.d_ptr->data;    
}

/*!
  Destroys this QScriptBreakpointInfo.
*/
QScriptBreakpointInfo::~QScriptBreakpointInfo()
{
    if (d_ptr) {
        delete d_ptr;
        d_ptr = 0;
    }
}

/*!
  Assigns the \a other value to this QScriptBreakpointInfo.
*/
QScriptBreakpointInfo &QScriptBreakpointInfo::operator=(const QScriptBreakpointInfo &other)
{
    if (!d_ptr)
        d_ptr = new QScriptBreakpointInfoPrivate;
    d_ptr->scriptId = other.d_ptr->scriptId;
    d_ptr->fileName = other.d_ptr->fileName;
    d_ptr->lineNumber = other.d_ptr->lineNumber;
    d_ptr->enabled = other.d_ptr->enabled;
    d_ptr->singleShot = other.d_ptr->singleShot;
    d_ptr->ignoreCount = other.d_ptr->ignoreCount;
    d_ptr->condition = other.d_ptr->condition;
    d_ptr->hitCount = other.d_ptr->hitCount;
    d_ptr->data = other.d_ptr->data;
    return *this;
}

/*!
  Returns the script ID associated with the breakpoint, or -1 if no ID
  is available.
*/
qint64 QScriptBreakpointInfo::scriptId() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->scriptId;
}

/*!
  Sets the script ID of this QScriptBreakpointInfo to \a scriptId.
*/
void QScriptBreakpointInfo::setScriptId(qint64 scriptId)
{
    Q_D(QScriptBreakpointInfo);
    d->scriptId = scriptId;
}

/*!
  Returns the filename associated with the breakpoint, or an empty
  string if no file name is available.
*/
QString QScriptBreakpointInfo::fileName() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->fileName;
}

/*!
  Sets the filename of this QScriptBreakpointInfo to \a fileName.
*/
void QScriptBreakpointInfo::setFileName(const QString &fileName)
{
    Q_D(QScriptBreakpointInfo);
    d->fileName = fileName;
}

/*!
  Returns the line number associated with the breakpoint, or -1
  if no line number is available.
*/
int QScriptBreakpointInfo::lineNumber() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->lineNumber;
}

/*!
  Sets the line number of this QScriptBreakpointInfo to \a lineNumber.
*/
void QScriptBreakpointInfo::setLineNumber(int lineNumber)
{
    Q_D(QScriptBreakpointInfo);
    d->lineNumber = lineNumber;
}

/*!
  Returns true if the breakpoint is enabled, otherwise returns false.

  \sa setEnabled()
*/
bool QScriptBreakpointInfo::isEnabled() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->enabled;
}

/*!
  Sets the enabled state of this QScriptBreakpointInfo to \a enabled.
*/
void QScriptBreakpointInfo::setEnabled(bool enabled)
{
    Q_D(QScriptBreakpointInfo);
    d->enabled = enabled;
}

/*!
  Returns true if the breakpoint is singleshot, otherwise returns
  false.

  \sa setSingleShot()
*/
bool QScriptBreakpointInfo::isSingleShot() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->singleShot;
}

/*!
  Sets the single-shot state of this QScriptBreakpointInfo to \a
  singleShot.

  \sa isSingleShot()
*/
void QScriptBreakpointInfo::setSingleShot(bool singleShot)
{
    Q_D(QScriptBreakpointInfo);
    d->singleShot = singleShot;
}

/*!
  Returns the ignore count of the breakpoint.
*/
int QScriptBreakpointInfo::ignoreCount() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->ignoreCount;
}

/*!
  Sets the ignore count of this QScriptBreakpointInfo to \a count.
*/
void QScriptBreakpointInfo::setIgnoreCount(int count)
{
    Q_D(QScriptBreakpointInfo);
    d->ignoreCount = count;
}

/*!
  Returns the condition associated with the breakpoint, or an empty
  string if the breakpoint is unconditional.
*/
QString QScriptBreakpointInfo::condition() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->condition;
}

/*!
  Sets the condition associated with this QScriptBreakpointInfo to \a
  condition.
*/
void QScriptBreakpointInfo::setCondition(const QString &condition)
{
    Q_D(QScriptBreakpointInfo);
    d->condition = condition;
}

/*!
  Returns user data associated with the breakpoint.
*/
QVariant QScriptBreakpointInfo::data() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->data;
}

/*!
  Sets the user data of this QScriptBreakpointInfo to \a data.
*/
void QScriptBreakpointInfo::setData(const QVariant &data)
{
    Q_D(QScriptBreakpointInfo);
    d->data = data;
}

/*!
  Increases the breakpoint's hit count by one.
*/
void QScriptBreakpointInfo::bumpHitCount()
{
    Q_D(QScriptBreakpointInfo);
    ++d->hitCount;
}

/*!
  Returns the number of times this breakpoint has been hit.
*/
int QScriptBreakpointInfo::hitCount() const
{
    Q_D(const QScriptBreakpointInfo);
    return d->hitCount;
}

/*!
  \fn QDataStream &operator<<(QDataStream &stream, const QScriptBreakpointInfo &info)
  \relates QScriptBreakpointInfo

  Writes the given \a info to the specified \a stream.
*/
QDataStream &operator<<(QDataStream &out, const QScriptBreakpointInfo &info)
{
    const QScriptBreakpointInfoPrivate *d = info.d_ptr;
    out << d->scriptId;
    out << d->fileName;
    out << d->lineNumber;
    out << d->enabled;
    out << d->singleShot;
    out << d->ignoreCount;
    out << d->condition;
    out << d->hitCount;
    out << d->data;
    return out;
}

/*!
  \fn QDataStream &operator>>(QDataStream &stream, QScriptBreakpointInfo &info)
  \relates QScriptBreakpointInfo

  Reads a QScriptBreakpointInfo from the specified \a stream into the
  given \a info.
*/
QDataStream &operator>>(QDataStream &in, QScriptBreakpointInfo &info)
{
    QScriptBreakpointInfoPrivate *d = info.d_ptr;
    in >> d->scriptId;
    in >> d->fileName;
    in >> d->lineNumber;
    in >> d->enabled;
    in >> d->singleShot;
    in >> d->ignoreCount;
    in >> d->condition;
    in >> d->hitCount;
    in >> d->data;
    return in;
}
