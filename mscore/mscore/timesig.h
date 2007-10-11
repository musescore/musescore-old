//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: timesig.h,v 1.2 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "element.h"

class Viewer;

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

   public:
      TimeSig(Score*);
      TimeSig(Score*, int st);
      TimeSig(Score*, int n, int z1, int z2=0, int z3=0, int z4=0);

      virtual TimeSig* clone() const { return new TimeSig(*this); }
      virtual ElementType type() const { return TIMESIG; }
      virtual void setSubtype(int val);
      virtual void draw(QPainter&);
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      void getSig(int* n, int* z1) const;
      static void getSig(int st, int* n, int* z) {
            *n = st & 0x3f;
            *z =    ((st>>24)& 0x3f)
                  + ((st>>18)& 0x3f)
                  + ((st>>12)& 0x3f)
                  + ((st>>6) & 0x3f);
            }
      void getSig(int* n, int* z1, int* z2, int*z3=0, int*z4=0) const;
      void setSig(int n, int z1, int z2=0, int z3=0, int z4=0);
      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);
      virtual QRectF bbox() const;
      };

#endif

