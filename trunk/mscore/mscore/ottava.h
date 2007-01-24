//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: ottava.h,v 1.2 2006/03/02 17:08:40 wschweer Exp $
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

#ifndef __OTTAVA_H__
#define __OTTAVA_H__

#include "line.h"

//---------------------------------------------------------
//   Ottava
//    brackets
//---------------------------------------------------------

class Ottava : public SLine {
      QString text;

   public:
      Ottava(Score* s) : SLine(s) {}
      virtual Ottava* clone() const { return new Ottava(*this); }
      virtual ElementType type() const { return OTTAVA; }
      virtual void draw1(Painter&);
      virtual void layout();
      void setLen(qreal);
      virtual void setSubtype(int val);
      virtual void write(Xml&) const;
      virtual void read(QDomNode);
      virtual QRectF bbox() const;
      };

#endif

