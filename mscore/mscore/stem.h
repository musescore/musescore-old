//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chord.h 3601 2010-10-22 12:46:05Z wschweer $
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

#ifndef __STEM_H__
#define __STEM_H__

#include "element.h"

class Chord;
class Painter;

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

/**
 Graphic representation of a note stem.
*/

class Stem : public Element {
      double _len;
      Spatium _userLen;

   public:
      Stem(Score*);
      Stem &operator=(const Stem&);

      virtual Stem* clone() const      { return new Stem(*this); }
      virtual ElementType type() const { return STEM; }
      virtual void draw(Painter*) const;
      void setLen(double v)            { _len = v; }
      double stemLen() const           { return _len + point(_userLen); }
      virtual QRectF bbox() const;
      virtual bool isEditable() const { return true; }

      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement e);
      virtual void toDefault();
      Spatium userLen() const         { return _userLen; }
      virtual void setVisible(bool f);
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      Chord* chord() const            { return (Chord*)parent(); }
      };

#endif

