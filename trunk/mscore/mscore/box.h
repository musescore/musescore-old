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

class BarLine;

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

class Box : public MeasureBase {

      Spatium _boxWidth;       // only valid for HBox
      Spatium _boxHeight;      // only valid for VBox
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
      virtual void write(Xml& xml, int) const { write(xml); }
      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      Spatium boxWidth() const        { return _boxWidth;  }
      void setBoxWidth(Spatium val)   { _boxWidth = val;   }
      Spatium boxHeight() const       { return _boxHeight; }
      void setBoxHeight(Spatium val)  { _boxHeight = val;  }
      };

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

class HBox : public Box {

   public:
      HBox(Score* score);
      ~HBox();
      virtual HBox* clone() const      { return new HBox(*this); }
      virtual ElementType type() const { return HBOX;       }

      virtual void layout(ScoreLayout*);
      virtual void collectElements(QList<const Element*>& el) const;

      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);
      };

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

class VBox : public Box {

   public:
      VBox(Score* score) : Box(score) {}
      virtual VBox* clone() const      { return new VBox(*this); }
      virtual ElementType type() const { return VBOX;       }

      virtual void layout(ScoreLayout*);

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);
      };

#endif

