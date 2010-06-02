//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __PITCHSPELLING_H__
#define __PITCHSPELLING_H__

class MidiNote;
class Note;
class Event;

//---------------------------------------------------------
//   pitch2tpc
//    Returns a default tpc for a given midi pitch.
//    Midi pitch 60 is middle C.
//---------------------------------------------------------

inline static int pitch2tpc(int pitch)
      {
      return (((((pitch % 12) * 7) % 12) + 5) % 12) + 9;
      }

int pitch2tpc2(int pitch, bool preferSharp);

const int INVALID_TPC = -2;

extern int pitch2tpc(int pitch, int key);

extern void spell(QList<Event*>& notes, int);
extern void spell(QList<Note*>& notes);
extern int computeWindow(const QList<Note*>& notes, int start, int end);
extern int tpc(int idx, int pitch, int opt);
extern int pitch2line(int pitch);
extern QString tpc2name(int tpc, bool germanNames);
extern void tpc2name(int tpc, bool germanNames, QChar* name, int* acc);
extern int step2tpc(const QString& stepName, int alter);
extern int step2tpc(int step, int alter);
extern int tpc2pitch(int tpc);
extern int tpc2step(int tpc);
extern int tpc2alter(int tpc);
extern QString tpc2stepName(int tpc);
extern int checkTpc(int);

#endif

