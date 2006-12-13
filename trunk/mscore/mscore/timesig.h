//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: timesig.h,v 1.2 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __TIMESIG_H__
#define __TIMESIG_H__

#include "element.h"

//---------------------------------------------------------
//   TimeSig
//    Time Signature
//---------------------------------------------------------

enum { TSIG_2_2, TSIG_2_4, TSIG_3_4, TSIG_4_4, TSIG_5_4, TSIG_6_4,
       TSIG_3_8, TSIG_6_8, TSIG_12_8, TSIG_44, TSIG_34,
       TSIG_9_8,
       TSIG_N
      };

class TimeSig : public Compound {
      int _z, _n;

   public:
      TimeSig(Score*);
      TimeSig(Score*, int);
      virtual ElementType type() const { return TIMESIG; }

      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      virtual void setSubtype(int val);
      void zn(int&, int&) const;
      void setSig(int z, int n);
      virtual bool acceptDrop(const QPointF&, int, int) const;
      virtual void drop(const QPointF&, int, int);
      };

#endif

