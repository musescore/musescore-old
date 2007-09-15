//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: note.h,v 1.45 2006/03/03 16:20:42 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __REPEAT_H__
#define __REPEAT_H__

#include "symbol.h"
#include "measure.h"

//---------------------------------------------------------
//   RepeatMeasure
//---------------------------------------------------------

class RepeatMeasure : public Element {
      QPainterPath path;

   public:
      RepeatMeasure(Score*);
      RepeatMeasure &operator=(const RepeatMeasure&);
      virtual RepeatMeasure* clone() const  { return new RepeatMeasure(*this); }
      virtual ElementType type() const      { return REPEAT_MEASURE; }
      virtual void draw(QPainter&);
      virtual void layout(ScoreLayout*);
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      };

enum RepeatType {
      RepeatNo = 0,
      RepeatSegno = 1,
      RepeatCoda  = 2,
      RepeatVarcoda  = 4,
      RepeatCodetta = 8,
      RepeatDacapo = 16,
      RepeatDacapoAlFine = 32,
      RepeatDacapoAlCoda = 64,
      RepeatDalSegno = 128,
      RepeatDalSegnoAlFine = 256,
      RepeatDalSegnoAlCoda = 512,
      RepeatAlSegno = 1024,
      RepeatFine = 2048
      };

//---------------------------------------------------------
//   Repeat
//---------------------------------------------------------

class Repeat : public Element {
      static QMap<QString, int> mapSI;
      static QMap<int, QString> mapIS;
      static bool initialized;

      virtual bool isMovable() const { return true; }

   public:
      Repeat(Score*);

      virtual Repeat* clone() const    { return new Repeat(*this); }
      virtual ElementType type() const { return REPEAT; }

      virtual void draw(QPainter&);
      virtual QRectF bbox() const;

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      };

#endif

