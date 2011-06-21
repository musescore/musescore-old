//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h 3424 2010-08-28 14:44:18Z wschweer $
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

#ifndef __LASSO_H__
#define __LASSO_H__

#include "element.h"

class Painter;

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element {
      QRectF _rect;
      MuseScoreView* view;        // valid in edit mode

   public:
      Lasso(Score*);
      virtual Lasso* clone() const       { return new Lasso(*this); }
      virtual ElementType type() const   { return LASSO; }
      virtual void draw(Painter*) const;
      virtual bool isEditable() const     { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      QRectF rect() const                 { return _rect; }
      void setRect(const QRectF& r)       { _rect = r;    }
      virtual QRectF bbox() const;
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      };

#endif

