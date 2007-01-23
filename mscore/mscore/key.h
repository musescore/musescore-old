//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: key.h,v 1.3 2006/03/02 17:08:34 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __KEY_H__
#define __KEY_H__

class Xml;
class Score;

//---------------------------------------------------------
//   KeyList
//    this list is instantiated for every Instrument
//    to keep track of key signature changes
//---------------------------------------------------------

typedef std::map<const int, int>::iterator iKeyEvent;
typedef std::map<const int, int>::const_iterator ciKeyEvent;

class KeyList : public std::map<const int, int> {
   public:
      KeyList() {}
      int key(int tick) const;
      void read(QDomNode, Score*);
      void write(Xml&, const char* name) const;

      void removeTime(int start, int len);
      void insertTime(int start, int len);
      };

#endif

