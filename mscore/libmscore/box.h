//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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
class Text;
class Painter;

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

class Box : public MeasureBase {
      Spatium _boxWidth;       // only valid for HBox
      Spatium _boxHeight;      // only valid for VBox
      qreal _leftMargin, _rightMargin;   // values in metric mm
      qreal _topMargin, _bottomMargin;
      bool editMode;
      qreal dragX;            // used during drag of hbox

   public:
      Box(Score*);
      virtual void draw(Painter*) const;
      virtual void layout();
      virtual void read(XmlReader*);
      virtual void add(Element* e);

      Spatium boxWidth() const         { return _boxWidth;     }
      void setBoxWidth(Spatium val)    { _boxWidth = val;      }
      Spatium boxHeight() const        { return _boxHeight;    }
      void setBoxHeight(Spatium val)   { _boxHeight = val;     }
      qreal leftMargin() const        { return _leftMargin;   }
      qreal rightMargin() const       { return _rightMargin;  }
      qreal topMargin() const         { return _topMargin;    }
      qreal bottomMargin() const      { return _bottomMargin; }
      void setLeftMargin(qreal val)   { _leftMargin = val;    }
      void setRightMargin(qreal val)  { _rightMargin = val;   }
      void setTopMargin(qreal val)    { _topMargin = val;     }
      void setBottomMargin(qreal val) { _bottomMargin = val;  }
      };

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

class HBox : public Box {
   public:
      HBox(Score* score) : Box(score) {}
      ~HBox() {}
      virtual HBox* clone() const      { return new HBox(*this); }
      virtual ElementType type() const { return HBOX;       }

      virtual void layout();

      void layout2();
      virtual bool isMovable() const;
      };

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

class VBox : public Box {
   public:
      VBox(Score* score) : Box(score) {}
      ~VBox() {}
      virtual VBox* clone() const      { return new VBox(*this); }
      virtual ElementType type() const { return VBOX;       }

      virtual void layout();
      };

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

class TBox : public VBox {
   public:
      TBox(Score* score);
      ~TBox() {}
      virtual TBox* clone() const      { return new TBox(*this); }
      virtual ElementType type() const { return TBOX;       }

      virtual void layout();
      virtual void add(Element*);
      Text* getText();
      };

//---------------------------------------------------------
//   FBox
//    frame containing fret diagrams
//---------------------------------------------------------

class FBox : public VBox {
   public:
      FBox(Score* score) : VBox(score) {}
      ~FBox() {}
      virtual FBox* clone() const      { return new FBox(*this); }
      virtual ElementType type() const { return FBOX;       }

      virtual void layout();
      void add(Element*);
      };

#endif

