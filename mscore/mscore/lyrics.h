//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2007 Werner Schweer (ws@seh.de)
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

#ifndef __LYRICS_H__
#define __LYRICS_H__

#include "text.h"

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

class Lyrics : public Text {
   public:
      enum Syllabic { SINGLE, BEGIN, END, MIDDLE };

   private:
      int _no;                ///< row index
      Syllabic _syllabic;
      Line* _separator;

   public:
      Lyrics(Score*);
      virtual Lyrics* clone() const { return new Lyrics(*this); }
      virtual ElementType type() const { return LYRICS; }

      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      void setNo(int n)             { _no = n; }
      int no() const                { return _no; }
      void setSyllabic(Syllabic s)  { _syllabic = s; }
      Syllabic syllabic() const     { return _syllabic; }
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void draw1(Painter&);
      Line* separator() const       { return _separator; }
      void setSeparator(Line* l)    { _separator = l; }
      };

#endif

