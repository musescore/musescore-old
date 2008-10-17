//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layoutbreak.h,v 1.1 2006/03/27 14:16:24 wschweer Exp $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#ifndef __SPACER_H__
#define __SPACER_H__

#include "element.h"

//---------------------------------------------------------
//   Spacer
//---------------------------------------------------------

class Spacer : public Element {
      Spatium height;
      QPainterPath path;

      virtual void draw(QPainter&) const;
      virtual void layout(ScoreLayout*);

   public:
      Spacer(Score*);
      virtual Spacer* clone() const { return new Spacer(*this); }
      virtual ElementType type() const { return SPACER; }
      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);
      };

#endif
