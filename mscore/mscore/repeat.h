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
      RepeatNo             = 0,
      RepeatSegno          = 1 << 0,
      RepeatCoda           = 1 << 1,
      RepeatVarcoda        = 1 << 2,
      RepeatCodetta        = 1 << 3,
      RepeatDacapo         = 1 << 4,
      RepeatDacapoAlFine   = 1 << 5,
      RepeatDacapoAlCoda   = 1 << 6,
      RepeatDalSegno       = 1 << 7,
      RepeatDalSegnoAlFine = 1 << 8,
      RepeatDalSegnoAlCoda = 1 << 9,
      RepeatAlSegno        = 1 << 10,
      RepeatFine           = 1 << 11,
      RepeatStart          = 1 << 12,
      RepeatEnd            = 1 << 13,
      RepeatMeasureFlag    = 1 << 14
      };

//---------------------------------------------------------
//   Repeat
//---------------------------------------------------------

class Repeat : public Element {
      Q_DECLARE_TR_FUNCTIONS(Measure)

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

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);
      };

#endif

