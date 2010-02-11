//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "score.h"

class ScScore;
class ScSCursor;

//---------------------------------------------------------
//   ScriptInterface
//---------------------------------------------------------

struct ScriptInterface {
      int n;
      const char* const* names;
      const int* lengths;
      const QScriptValue::PropertyFlags* flags;

      QString name(int i)                     { return QString::fromLatin1(names[i]); }
      QScriptValue::PropertyFlags flag(int i) { return flags[i]; }
      };

//---------------------------------------------------------
//   ScriptEngine
//---------------------------------------------------------

class ScriptEngine : public QScriptEngine {
      Q_OBJECT

      ScScore* scoreClass;
      ScSCursor* cursorClass;

   public:
      ScriptEngine();
      ScScore* getScoreClass() const { return scoreClass; }
      ScSCursor* getCursorClass() const { return cursorClass; }
      };

#endif

