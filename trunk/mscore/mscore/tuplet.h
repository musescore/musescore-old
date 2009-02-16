//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: tuplet.h,v 1.9 2006/03/13 21:35:59 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __TUPLET_H__
#define __TUPLET_H__

#include "duration.h"
#include "ui_tupletdialog.h"
#include "ui_tupletproperties.h"

class Text;

//------------------------------------------------------------------------
//   Tuplet
//     Example of 1/8 triplet:
//       _baseLen     = 192   (division/2 == 1/8 duration)
//       _actualNotes = 3
//       _normalNotes = 2     (3 notes played in the time of 2/8)
//
//    the tuplet has a len of _baseLen * _normalNotes
//    a tuplet note has len of _baseLen * _normalNotes / _actualNotes
//------------------------------------------------------------------------

class Tuplet : public DurationElement {
   public:
      enum { SHOW_NUMBER, SHOW_RELATION, NO_TEXT };
      enum { AUTO_BRACKET, SHOW_BRACKET, SHOW_NO_BRACKET };

   private:
      Q_DECLARE_TR_FUNCTIONS(Tuplet)

      QList<DurationElement*> _elements;
      int _numberType;
      int _bracketType;
      bool _hasBracket;

      int _baseLen;     // tick len of a "normal note"
      int _normalNotes;
      int _actualNotes;

      bool _userModified;
      QPointF p1, p2;
      QPointF _p1, _p2;
      mutable int _id;          // used during read/write

      Text* _number;
      QPointF bracketL[4];
      QPointF bracketR[3];

      virtual bool genPropertyMenu(QMenu* menu) const;
      virtual void propertyAction(const QString&);
      virtual void setSelected(bool f);

   public:
      Tuplet(Score*);
      ~Tuplet();
      virtual Tuplet* clone() const    { return new Tuplet(*this); }
      virtual ElementType type() const { return TUPLET; }
      virtual QRectF bbox() const;
      virtual bool isMovable() const   { return true; }

      virtual void add(Element*);
      virtual void remove(Element*);

      virtual bool startEdit(Viewer*, const QPointF&);
      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;

      Measure* measure() const      { return (Measure*)parent(); }

      int numberType() const        { return _numberType;   }
      int bracketType() const       { return _bracketType;  }
      void setNumberType(int val)   { _numberType = val;    }
      void setBracketType(int val)  { _bracketType = val;   }
      bool hasBracket() const       { return _hasBracket;   }

      void setBaseLen(int val)      { _baseLen = val;       }
      void setNormalNotes(int val)  { _normalNotes = val;   }
      void setActualNotes(int val)  { _actualNotes = val;   }
      int baseLen() const           { return _baseLen;      }
      int normalNotes() const       { return _normalNotes;  }
      int actualNotes() const       { return _actualNotes;  }
      int normalLen() const         { return _baseLen / _normalNotes; }
      int noteLen() const           { return _baseLen * _normalNotes / _actualNotes; }
      const QList<DurationElement*>& elements() const   { return _elements; }
      void clear()                  { _elements.clear(); }
      virtual void layout(ScoreLayout*);
      Text* number() const { return _number; }

      virtual void read(QDomElement);
      void write(Xml&) const;

      virtual void toDefault();

      virtual void draw(QPainter&) const;
      int id() const                  { return _id; }
      void setId(int i) const         { _id = i; }
      virtual int tickLen() const     { return _baseLen * _normalNotes; }
      };


//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

class TupletDialog : public QDialog, Ui::TupletDialog {

   public:
      TupletDialog(QWidget* parent = 0);
      void setupTuplet(Tuplet* tuplet);
      int getNormalNotes() const { return normalNotes->value(); }
      };

//---------------------------------------------------------
//   TupletProperties
//---------------------------------------------------------

class TupletProperties : public QDialog, public Ui::TupletProperties {
      Q_OBJECT
      Tuplet* tuplet;

   public slots:
      virtual void accept();

   public:
      TupletProperties(Tuplet* tuplet, QWidget* parent = 0);
      };

#endif

