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

#ifndef QSCRIPTDEBUGGERCLIENT_P_H
#define QSCRIPTDEBUGGERCLIENT_P_H

#include <QtCore/qobject.h>

class QScriptDebuggerFrontend;
class QScriptDebuggerEvent;
class QScriptDebuggerResponse;

class QScriptDebuggerClient;
class QScriptDebuggerClientPrivate : public QObject
{
    Q_DECLARE_PUBLIC(QScriptDebuggerClient)
public:
    QScriptDebuggerClientPrivate();
    ~QScriptDebuggerClientPrivate();

    static QScriptDebuggerClientPrivate *get(QScriptDebuggerClient *q);

    bool event(QEvent *e);

    void postEvent(const QScriptDebuggerEvent &event);
    void postCommandFinished(int id, const QScriptDebuggerResponse &response);

    QScriptDebuggerFrontend *frontend;

    QScriptDebuggerClient *q_ptr;
};

#endif
