//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: hairpin.h,v 1.13 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __HAIRPIN_H__

#include "line.h"

class Score;

//---------------------------------------------------------
//   Hairpin
//
//    subtype: 0 = crescendo,  1 = decrescendo
//---------------------------------------------------------

class Hairpin : public SLine {
   public:
      Hairpin(Score*);
      virtual Hairpin* clone() const { return new Hairpin(*this); }
      virtual ElementType type() const { return HAIRPIN; }

      void setLen(double);
      virtual void draw(QPainter&);
      virtual void layout(ScoreLayout*);
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      virtual QRectF bbox() const;
      virtual void setSubtype(int);
      };

#define __HAIRPIN_H__

#endif

