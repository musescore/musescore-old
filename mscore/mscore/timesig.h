//=============================================================================
//  MusE Score
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

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "element.h"
#include "al/sig.h"

class ScoreView;
class Segment;

enum {
      TSIG_FOUR_FOUR  = 0x40000104,
      TSIG_ALLA_BREVE = 0x40002084
      };

//---------------------------------------------------------
//   TimeSig
//    Time Signature
//---------------------------------------------------------

/**
 \a subtype() is coded as:
      bit 0-5     denominator (n)
      bit 6-11    nominator1 (z1)
      bit 12-17   nominator2
      bit 18-23   nominator3
      bit 24-29   nominator4
      bit 30      variation (alla breve etc.)
*/
class TimeSig : public Element {

      QString sz, sn;   // cached values, set in layout()
      QPointF pz, pn;

   public:
      TimeSig(Score*);
      TimeSig(Score*, int st);
      TimeSig(Score*, int n, int z1, int z2=0, int z3=0, int z4=0);

      TimeSig* clone() const   { return new TimeSig(*this); }
      ElementType type() const { return TIMESIG; }
      QPointF canvasPos() const;      ///< position in canvas coordinates
      void setSubtype(int val);
      void draw(QPainter&) const;
      void write(Xml& xml) const;
      void read(QDomElement);
      void layout();
      Space space() const;

      Fraction getSig() const { return getSig(subtype()); }
      static Fraction getSig(int st) {
            return Fraction(
                  ((st>>24)& 0x3f)
                  + ((st>>18)& 0x3f)
                  + ((st>>12)& 0x3f)
                  + ((st>>6) & 0x3f), st & 0x3f);
            }
      void getSig(int* n, int* z1, int* z2, int*z3=0, int*z4=0) const;
      void setSig(int n, int z1, int z2=0, int z3=0, int z4=0);
      void setSig(const Fraction& f)     { setSig(f.denominator(), f.numerator(), 0, 0, 0); }
      void setSig(const AL::SigEvent& e) { setSig(e.fraction()); }

      bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      static int sigtype(int n, int z1, int z2 = 0, int z3 = 0, int z4 = 0) {
            return (z4 << 24) + (z3 << 18) + (z2 << 12) + (z1 << 6) + n;
            }
      Segment* segment() const { return (Segment*)parent(); }
      };

#endif

