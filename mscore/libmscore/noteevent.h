//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __NOTEEVENT_H__
#define __NOTEEVENT_H__

class Xml;

//---------------------------------------------------------
//    NoteEvent
//---------------------------------------------------------

class NoteEvent {
      char _pitch;      // relative pitch to note pitch
      int _ontime;      // 1/1000 of nominal note len
      int _len;         // 1/1000 of nominal note len

   public:
      NoteEvent() : _pitch(0), _ontime(0), _len(1000) {}
      NoteEvent(int a, int b, int c) : _pitch(a), _ontime(b), _len(c) {}

      void read(QDomElement);
      void write(Xml& xml) const;

      char pitch() const     { return _pitch; }
      int ontime() const     { return _ontime; }
      int offtime() const    { return _ontime + _len; }
      int len() const        { return _len; }
      void setPitch(int v)   { _pitch = v; }
      void setOntime(int v)  { _ontime = v; }
      void setLen(int v)     { _len = v;    }
      };


#endif
