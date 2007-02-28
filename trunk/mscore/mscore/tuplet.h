//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: tuplet.h,v 1.9 2006/03/13 21:35:59 wschweer Exp $
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

#ifndef __TUPLET_H__
#define __TUPLET_H__

#include "chordlist.h"
#include "element.h"

class Text;

//------------------------------------------------------------------------
//   Tuplet
//     Example of 1/8 triplet:
//       _baseLen     = 192   (division/2 == 1/8 duration)
//       _actualNotes = 3
//       _normalNotes = 2     (3 notes played in the time of 2/8)
//
//    the tuplet has a len of _baseLen * _normalNotes
//    a tuplet note has len of _baseLen * _normalNotes / _actualNotes
//------------------------------------------------------------------------

class Tuplet : public Element {
      ChordRestList _elements;
      bool _hasNumber;
      bool _hasLine;
      int _baseLen;     // tick len of a "normal note"
      int _normalNotes;
      int _actualNotes;

      Text* _number;
      QPolygonF bracketL;
      QPolygonF bracketR;

      virtual bool genPropertyMenu(QMenu* menu) const;
      virtual void propertyAction(const QString&);
      virtual void setSelected(bool f);

   public:
      Tuplet(Score*);
      ~Tuplet();
      virtual Tuplet* clone() const { return new Tuplet(*this); }
      virtual ElementType type() const { return TUPLET; }
      virtual QRectF bbox() const;

      virtual void add(Element*);
      virtual void remove(Element*);

      bool hasNumber() const       { return _hasNumber;   }
      bool hasLine() const         { return _hasLine;     }
      void setHasNumber(bool val)  { _hasNumber = val;    }
      void setHasLine(bool val)    { _hasLine = val;      }
      void setBaseLen(int val)     { _baseLen = val;      }
      void setNormalNotes(int val) { _normalNotes = val;  }
      void setActualNotes(int val) { _actualNotes = val;  }
      int baseLen() const          { return _baseLen;     }
      int normalNotes() const      { return _normalNotes; }
      int actualNotes() const      { return _actualNotes; }
      int noteLen() const { return _baseLen * _normalNotes / _actualNotes; }
      ChordRestList* elements()    { return &_elements; }

      virtual void layout(ScoreLayout*);
      Text* number() const { return _number; }

      virtual void read(QDomNode);
      void write(Xml&, int) const;

      virtual void move(double, double);
      virtual void draw(QPainter&);
      };

#endif

