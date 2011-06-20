//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "globals.h"
#include "shortcut.h"

//---------------------------------------------------------
//   Shortcut
//---------------------------------------------------------

Shortcut::Shortcut()
      {
      state       = STATE_NORMAL;
      xml         = 0;
      standardKey = QKeySequence::UnknownKey;
      key         = 0;
      context     = Qt::WindowShortcut;
      icon        = -1;
      action      = 0;
      translated  = false;
      }

Shortcut::Shortcut(int s, int f, const char* name, const char* d, const QKeySequence& k,
   Qt::ShortcutContext cont, const char* txt, const char* h, int i)
      {
      state       = s;
      flags       = f;
      xml         = name;
      standardKey = QKeySequence::UnknownKey;
      key         = k;
      context     = cont;
      icon        = i;
      action      = 0;
      descr       = qApp->translate("action", d);
      help        = qApp->translate("action", h);
      text        = qApp->translate("action", txt);
      translated  = false;
      }

Shortcut::Shortcut(int s, int f, const char* name, const char* d, QKeySequence::StandardKey sk,
   Qt::ShortcutContext cont, const char* txt, const char* h, int i)
      {
      state       = s;
      flags       = f;
      xml         = name;
      standardKey = sk;
      key         = 0;
      context     = cont;
      icon        = i;
      action      = 0;
      descr       = qApp->translate("action", d);
      help        = qApp->translate("action", h);
      text        = qApp->translate("action", txt);
      translated  = false;
      }

Shortcut::Shortcut(const Shortcut& c)
      {
      state       = c.state;
      flags       = c.flags;
      xml         = c.xml;
      standardKey = c.standardKey;
      key         = c.key;
      context     = c.context;
      icon        = c.icon;
      action      = c.action;
      if (c.translated) {
            descr   = c.descr;
            help    = c.help;
            text    = c.text;
            }
      else {
            descr   = qApp->translate("action", c.descr.toUtf8().data());
            help    = qApp->translate("action", c.help.toUtf8().data());
            text    = qApp->translate("action", c.text.toUtf8().data());
            translated = true;
            }
      }


