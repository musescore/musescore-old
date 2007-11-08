//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __BOX_H__
#define __BOX_H__

/**
 \file
 Definition of HBox and VBox classes.
*/

#include "measurebase.h"


//---------------------------------------------------------
//   Box
//---------------------------------------------------------

class Box : public MeasureBase {

      double _boxWidth;       // only valid for HBox
      double _boxHeight;      // only valid for VBox
      bool editMode;

   public:
      Box(Score*);
      virtual void draw(QPainter& p) const;

      virtual bool startEdit(const QPointF&);
      virtual bool edit(int, QKeyEvent*);
      virtual void editDrag(int, const QPointF&, const QPointF&);
      virtual void endEditDrag();
      virtual void endEdit();
      virtual void updateGrips(int* grips, QRectF*) const;
      virtual void layout(ScoreLayout*);
      virtual void write(Xml&, int) const;
      virtual void read(QDomElement);

      double boxWidth() const          { return _boxWidth;  }
      void setBoxWidth(double val)     { _boxWidth = val;   }
      double boxHeight() const         { return _boxHeight; }
      void setBoxHeight(double val)    { _boxHeight = val;  }
      };

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

class HBox : public Box {
   public:
      HBox(Score* score) : Box(score) {}
      virtual HBox* clone() const      { return new HBox(*this); }
      virtual ElementType type() const { return HBOX;       }
      };

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

class VBox : public Box {

   public:
      VBox(Score* score) : Box(score) {}
      virtual VBox* clone() const      { return new VBox(*this); }
      virtual ElementType type() const { return VBOX;       }
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);
      };

#endif

