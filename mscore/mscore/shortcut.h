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

#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__

enum ShortcutFlags {
      A_SCORE = 0x1, A_CMD = 0x2
      };

//---------------------------------------------------------
//   Shortcut
//    hold the basic values for configurable shortcuts
//---------------------------------------------------------

class Shortcut {
   public:
      int state;              //! shortcut is valid in this Mscore state
      int flags;
      const char* xml;        //! xml tag name for configuration file
      QString descr;          //! descriptor, shown in editor
      QKeySequence key;       //! shortcut
      QKeySequence::StandardKey standardKey;
      Qt::ShortcutContext context;
      QString text;           //! text as shown on buttons or menus
      QString help;           //! ballon help
      int icon;
      QAction* action;        //! cached action
      bool translated;

      Shortcut();
      Shortcut(int state, int flags, const char* name, const char* d, QKeySequence::StandardKey sk = QKeySequence::UnknownKey,
         Qt::ShortcutContext cont = Qt::ApplicationShortcut,
         const char* txt = 0, const char* h = 0, int i = -1);
      Shortcut(int state, int flags, const char* name, const char* d, const QKeySequence& k = QKeySequence(),
         Qt::ShortcutContext cont = Qt::ApplicationShortcut,
         const char* txt = 0, const char* h = 0, int i = -1);
      Shortcut(const Shortcut& c);
      };

#endif

