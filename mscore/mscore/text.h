//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: text.h,v 1.24 2006/04/05 08:15:12 wschweer Exp $
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

#ifndef __TEXT_H__
#define __TEXT_H__

#include "element.h"

//---------------------------------------------------------
//   HBox
//    basic text element
//---------------------------------------------------------

class HBox {
   public:
      HBox() {}
      virtual ~HBox() {}
      virtual HBox* clone() const = 0;
      virtual void draw(Painter& p) const = 0;
      virtual QString txt() const = 0;
      virtual qreal width() const = 0;
      virtual qreal height() const = 0;
      virtual void setPos(const QPointF&) = 0;
      virtual const QPointF pos() const = 0;
      virtual void write(Xml& xml,int style) const = 0;
      virtual int size() const = 0;
      virtual QRectF bbox() const = 0;
      virtual QRectF cursor(int col) const = 0;
      virtual bool insertChar(int col, const QString& s) = 0;
      virtual void removeChar(int col) = 0;
      virtual void setSelected(bool f) = 0;
      virtual const QFont* font() const = 0;
      virtual QFont* font() = 0;
      virtual qreal lineSpacing() const = 0;
      virtual HBox* split(int col) = 0;
      };

//---------------------------------------------------------
//   HBoxText
//---------------------------------------------------------

class HBoxText : public HBox {
      QFont _font;
      QString _txt;
      QRectF _bbox;
      QPointF _pos;
      qreal _w;

   public:
      HBoxText() {}
      HBoxText(const QString& s, const QFont& f);
      ~HBoxText() {}
      virtual HBox* clone() const           { return new HBoxText(*this); }
      virtual void draw(Painter& p) const;
      virtual QString txt() const           { return _txt;           }
      virtual qreal width() const           { return _w;             }
      virtual qreal height() const          { return _bbox.height(); }
      virtual void setPos(const QPointF& p) { _pos = p;              }
      virtual const QPointF pos() const     { return _pos;           }
      virtual void write(Xml& xml, int style) const;
      virtual int size() const              { return _txt.size();    }
      virtual QRectF bbox() const           { return _bbox;          }
      virtual QRectF cursor(int col) const;
      virtual bool insertChar(int col, const QString& s);
      virtual void removeChar(int col);
      virtual void setSelected(bool)        {}
      virtual const QFont* font() const     { return &_font; }
      virtual QFont* font()                 { return &_font; }
      virtual qreal lineSpacing() const;
      virtual HBox* split(int col);
      };

//---------------------------------------------------------
//   HBoxElement
//---------------------------------------------------------

class HBoxElement : public HBox {
      Element* element;

   public:
      HBoxElement();
      HBoxElement(const HBoxElement&);
      HBoxElement(Element* e)               { element = e; }
      ~HBoxElement();
      virtual HBox* clone() const           { return new HBoxElement(*this); }
      virtual void draw(Painter& p) const   { element->draw(p);          }
      virtual QString txt() const           { return "";                 }
      virtual qreal width() const           { return element->width();   }
      virtual qreal height() const          { return element->height();  }
      virtual void setPos(const QPointF& p) { element->setPos(p);        }
      virtual const QPointF pos() const     { return element->pos();     }
      virtual void write(Xml& xml, int style) const;
      virtual int size() const              { return 1;                  }
      virtual QRectF bbox() const           { return element->bbox();    }
      virtual QRectF cursor(int col) const;
      virtual bool insertChar(int, const QString&) { return false; }
      virtual void removeChar(int)          {}
      virtual void setSelected(bool f)      { element->setSelected(f);   }
      virtual const QFont* font() const     { return 0;                  }
      virtual QFont* font()                 { return 0;                  }
      virtual qreal lineSpacing() const     { return element->bbox().height(); }
      virtual HBox* split(int)              { return 0; }
      };

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

class VBox {
   public:
      QRectF bbox;            // note: the bounding box excludes spaces
      QList<HBox*> hlist;
      qreal w;                // text width of this textline including spaces
      qreal lineSpacing;      // vertical spacing of this line

      QString text() const;
      VBox();
      ~VBox();
      VBox(const VBox&);
      void clear();
      void append(HBox* hb);
      void insert(int idx, HBox* hb);
      };

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text : public QList<VBox> {
      int textStyle;
      bool bold, italic;
      QString family;
      int _fontSize;
      void parseData(VBox&, QDomNode);

   public:
      Text() : QList<VBox>() { }
      Text(int style) : QList<VBox>() {
            textStyle = style;
            }
      void setStyle(int style) { textStyle = style; }
      Text(const QString& s, int style);
      Text(const QString& s, int style, bool bold, double size);
      QString text() const;
      void read(QDomNode);
      void write(Xml&, const char*) const;
      int fontSize() const                 { return _fontSize; }
      void setFontSize(int v)              { _fontSize = v; }
      QString fontFamily() const           { return family; }
      void setFontFamily(const QString& s) { family = s; }
      bool isBold() const                  { return bold; }
      bool isItalic() const                { return italic; }
      void setBold(bool val)               { bold = val; }
      void setItalic(bool val)             { italic = val; }
      };

#endif

