//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
class ScoreView;
class Text;
class Painter;

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

class Box : public MeasureBase {
      Spatium _boxWidth;       // only valid for HBox
      Spatium _boxHeight;      // only valid for VBox
      double _leftMargin, _rightMargin;   // values in metric mm
      double _topMargin, _bottomMargin;
      bool editMode;
      qreal dragX;            // used during drag of hbox

   public:
      Box(Score*);
      virtual void draw(Painter*) const;
      virtual bool isEditable() const { return true; }
      virtual void startEdit(ScoreView*, const QPointF&);
      virtual bool edit(ScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void editDrag(const EditData&);
      virtual void endEditDrag();
      virtual void endEdit();
      virtual void updateGrips(int* grips, QRectF*) const;
      virtual void layout();
      virtual void write(Xml&) const;
      virtual void write(Xml& xml, int, bool) const { write(xml); }
      virtual void read(QDomElement);
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      virtual void add(Element* e);

      Spatium boxWidth() const         { return _boxWidth;     }
      void setBoxWidth(Spatium val)    { _boxWidth = val;      }
      Spatium boxHeight() const        { return _boxHeight;    }
      void setBoxHeight(Spatium val)   { _boxHeight = val;     }
      double leftMargin() const        { return _leftMargin;   }
      double rightMargin() const       { return _rightMargin;  }
      double topMargin() const         { return _topMargin;    }
      double bottomMargin() const      { return _bottomMargin; }
      void setLeftMargin(double val)   { _leftMargin = val;    }
      void setRightMargin(double val)  { _rightMargin = val;   }
      void setTopMargin(double val)    { _topMargin = val;     }
      void setBottomMargin(double val) { _bottomMargin = val;  }
      };

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

class HBox : public Box {
      Q_DECLARE_TR_FUNCTIONS(HBox)

   public:
      HBox(Score* score) : Box(score) {}
      ~HBox() {}
      virtual HBox* clone() const      { return new HBox(*this); }
      virtual ElementType type() const { return HBOX;       }

      virtual void layout();

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      virtual QRectF drag(const QPointF& s);
      void layout2();
      virtual bool isMovable() const;
      };

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

class VBox : public Box {
      Q_DECLARE_TR_FUNCTIONS(VBox)

   public:
      VBox(Score* score) : Box(score) {}
      ~VBox() {}
      virtual VBox* clone() const      { return new VBox(*this); }
      virtual ElementType type() const { return VBOX;       }

      virtual void layout();

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      virtual QPointF getGrip(int) const;
      virtual void setGrip(int, const QPointF&);
      };

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

class TBox : public VBox {
      Q_DECLARE_TR_FUNCTIONS(TBox)

   public:
      TBox(Score* score);
      ~TBox() {}
      virtual TBox* clone() const      { return new TBox(*this); }
      virtual ElementType type() const { return TBOX;       }

      virtual void layout();
      virtual void add(Element*);
      Text* getText();
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      };

//---------------------------------------------------------
//   FBox
//    frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox {
      Q_DECLARE_TR_FUNCTIONS(FBox)

   public:
      FBox(Score* score) : VBox(score) {}
      ~FBox() {}
      virtual FBox* clone() const      { return new FBox(*this); }
      virtual ElementType type() const { return FBOX;       }

      virtual void layout();
      void add(Element*);
      };

#endif

