//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

/**
 \file
 Definition of classes Element, ElementList, SStaff.
*/

#include "spatium.h"
#include "mtime.h"

class QKeyEvent;
class Xml;
class Measure;
class Staff;
class Score;
class Painter;
class Sym;

enum ElementType {
      SYMBOL, TEXT, STAFF, SLUR_SEGMENT, NOTE, BAR_LINE,
      STEM, COMPOUND, LINE, BRACKET,
/*11*/      ACCIDENTAL,
      CURSOR, SELECTION, LASSO,
      CLEF, KEYSIG, TIMESIG, CHORD, REST,
      TIE, SLUR, MEASURE,
      ATTRIBUTE, DYNAMIC, PAGE, BEAM, HOOK, LYRICS,
      SYSTEM, HAIRPIN, TUPLET, RUBBERBAND, VSPACER,
      SEGMENT, TEMPO_TEXT,
      SHADOW_NOTE, VOLTA, OTTAVA, PEDAL, TRILL,
      LAYOUT_BREAK,
      HELP_LINE
      };

// Accidentals
enum { ACC_NONE, ACC_SHARP, ACC_FLAT, ACC_SHARP2, ACC_FLAT2, ACC_NATURAL };

extern const char* elementNames[];  // for debugging

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

/**
 The Element class is the virtual base class of all score layout elements.

 More details: TBD
*/

class Element {
      Element* _next;
      Element* _prev;
      Element* _parent;
      Measure* _anchor;

      bool _selected;
      bool _dropTarget;
      bool _generated;  ///< automatically generated Element
      bool _visible;    ///< visibility attribute

      int _subtype;

      int _voice;       ///< 0 - VOICES
      Staff* _staff;
      MTime _time;
      QColor _color;

      void init();

   protected:
      Score* _score;
      QPointF _pos;     ///< Reference position, relative to _parent.
                        ///< Usually set from layout().
      QPointF _userOff; ///< Offset from normal layout position:
                        ///< user dragged object this amount.
                        ///< In Spatium ("space") units!
      int _mxmlOff;     ///< MusicXML offset in ticks.
                        ///< Note: interacts with userXoffset.

      mutable QRectF _bbox;     ///< Bounding box relative to _pos + _userOff
      mutable MTime _duration;  ///< Note: lazy evaluation

   public:
      Element(Score*);
      virtual ~Element() {}
      Element &operator=(const Element&);
      virtual Element* clone() const = 0;

      Score* score() const                    { return _score;  }
      void setScore(Score* s)                 { _score = s;     }
      Element* next() const                   { return _next;   }
      void setNext(Element* e)                { _next = e;      }
      Element* prev() const                   { return _prev;   }
      void setPrev(Element* e)                { _prev = e;      }
      Element* parent() const                 { return _parent; }
      void setParent(Element* e)              { _parent = e;    }

      Measure* anchor() const                 { return _anchor; }
      void setAnchor(Measure* m)              { _anchor = m;    }

      bool selected() const                   { return _selected;   }
      virtual void setSelected(bool f)        { _selected = f;      }
      bool visible() const                    { return _visible;    }
      virtual void setVisible(bool f)         { _visible = f;       }
      bool generated() const                  { return _generated;  }
      void setGenerated(bool val)             { _generated = val;   }
      bool dropTarget() const                 { return _dropTarget; }
      void setDropTarget(bool f)              { _dropTarget = f;    }

      virtual QPointF ipos() const            { return _pos;       }
      virtual QPointF pos() const             { return _pos + (_userOff * _spatium);         }
      virtual double x() const                { return _pos.x() + (_userOff.x() * _spatium); }
      virtual double y() const                { return _pos.y() + (_userOff.y() * _spatium); }
      virtual void setPos(const QPointF& p)   { _pos = p;                }
      virtual void setPos(double x, double y) { _pos = QPointF(x, y);    }
      virtual void move(double xd, double yd) { _pos += QPointF(xd, yd); }
      virtual void move(const QPointF& s)     { _pos += s;               }

      QPointF aref() const;     ///< Canvas reference pos
      const QPointF& userOff() const          { return _userOff;  }
      void setUserOff(const QPointF& o)       { _userOff = o;     }
      void setUserXoffset(qreal v)            { _userOff.setX(v); }
      void setUserYoffset(qreal v)            { _userOff.setY(v); }
      int mxmlOff() const                     { return _mxmlOff;  }
      void setMxmlOff(int o)                  { _mxmlOff = o;     }

      virtual QRectF bbox() const             { return _bbox;           }
      virtual double height() const           { return bbox().height(); }
      virtual double width() const            { return bbox().width();  }
      QRectF abbox() const                    { return bbox().translated(aref()); }
      QPointF apos() const                    { return aref();          }
      virtual void setbbox(const QRectF& r)   { _bbox = r;              }
      virtual bool contains(const QPointF& p) const;
      virtual QPainterPath shape() const;
      bool intersects(const QRectF& r) const;

      virtual ElementType type() const = 0;
      int subtype() const                     { return _subtype;        }
      virtual void setSubtype(int val)        { _subtype = val;         }
      bool isChordRest() const {
            return type() == REST || type() == CHORD;
            }

      virtual void draw(Painter&p);
      virtual void draw1(Painter&) {}

      void writeProperties(Xml& xml) const;
      bool readProperties(QDomNode);

      virtual void write(Xml&) const;
      virtual void read(QDomNode);

      virtual bool isMovable() const          { return false; }
      virtual QRectF drag(const QPointF& s);
      virtual void endDrag()                  {}

      virtual bool startEdit(QMatrix&)        { return false; }
      virtual bool edit(QKeyEvent*)           { return false;}
      virtual bool startEditDrag(const QPointF&)  { return false; }
      virtual bool editDrag(QMatrix&, QPointF*, const QPointF&) { return false; }
      virtual bool endEditDrag()              { return false;}
      virtual QPointF dragOff() const         { return QPointF(0,0); }
      virtual void endEdit()                  {}

      Staff* staff() const                    { return _staff; }
      int staffIdx() const;
      void setStaff(Staff* v)                 { _staff = v;    }
      int voice() const                       { return _voice; }
      void setVoice(int v)                    { _voice = v;    }

      virtual void add(Element*)    {}
      virtual void remove(Element*) {}

      virtual void layout() {}
      virtual void resetMode() {}

      // debug functions
      virtual void dump() const;
      const char* name() const  { return elementNames[type()]; }
      void dumpQPointF(const char*) const;

      bool operator>(const Element&) const;

      MTime time() const              { return _time; }
      MTime duration() const          { return _duration; }

      int tick() const                { return _time.tick(); }
      void setTick(int t)             { _time.setTick(t); }
      virtual int tickLen() const     { return _duration.tick(); }
      virtual void setTickLen(int t)  { _duration.setTick(t); }
      virtual void space(double& min, double& extra) const;
      QColor color() const            { return _color; }
      void setColor(const QColor& c)  { _color = c;    }
      static int readType(QDomNode& node);

      virtual QByteArray mimeData() const;
/**
 Return true if this element accepts a drop at canvas relative \a pos
 of given element \a type and \a subtype.

 Reimplemented by elements that accept drops. Used to change cursor shape while
 dragging to indicate drop targets.
*/
      virtual bool acceptDrop(const QPointF&, int, const QDomNode&) const { return false; }

/**
 Handle a dropped element at canvas relative \a pos of given element
 \a type and \a subtype.

 Reimplemented by elements that accept drops.
*/
      virtual void drop(const QPointF&, int, const QDomNode&) {}

/**
 Return a name for a \a subtype. Used for outputting xml data.
 Reimplemented by elements with subtype names.
 */
      virtual const QString subtypeName() const { return QString("%1").arg(_subtype); }

/**
 Set subtype by name
 Used for reading xml data.
 Reimplemented by elements with subtype names.
 */
      virtual void setSubtype(const QString& s) { setSubtype(s.toInt()); }

/**
 entend property menu by elemement specific items
 */
      virtual bool genPropertyMenu(QMenu*) const { return false; }
      virtual void propertyAction(const QString&) {}

/**
 delivers mouseEvent to element in edit mode
 returns true if mouse event is accepted by element
 */
      virtual bool mousePress(const QPointF&) { return false; }
      };

//---------------------------------------------------------
//   ElementList
//---------------------------------------------------------

class ElementList : public QList<Element*> {
   public:
      ElementList() {}
      bool remove(Element*);
      void replace(Element* old, Element* n);
      void write(Xml&) const;
      void draw(Painter& p);

      void add(Element*);
      void move(Element* el, int tick);
      void write(Xml&, const char* name) const;
      void read(QDomNode);
      };

typedef ElementList::iterator iElement;
typedef ElementList::const_iterator ciElement;

//---------------------------------------------------------
//   SStaff
//---------------------------------------------------------

/**
 The SStaff class is the graphic representation of a staff,
 it draws the horizontal staff lines.
*/

class SStaff : public Element {
      // Spatium lineWidth;
      qreal _width;
      int lines;

   public:
      SStaff(Score*);
      virtual SStaff* clone() const    { return new SStaff(*this); }
      virtual ElementType type() const { return STAFF; }
      void setWidth(qreal v)           { _width = v; }
      virtual QRectF bbox() const;
      virtual void draw1(Painter&);
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      };

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

class Cursor : public Element {
   protected:
      double dlen;
      double lineWidth;
      bool _blink;
      bool _on;

   public:
      Cursor(Score*, double l);
      virtual Cursor* clone() const { return new Cursor(*this); }
      virtual ElementType type() const { return CURSOR; }

      virtual void draw1(Painter&);
      void setOn(bool f)  { _on = f; }
      bool isOn() const   { return _on; }
      void blink()        { _blink = !_blink; }
      void noBlinking()   { _blink = true; }
      };

//---------------------------------------------------------
//   VSpacer
//---------------------------------------------------------

class VSpacer : public Element {
   protected:
      double height;

   public:
      VSpacer(Score*, double h);
      virtual VSpacer* clone() const { return new VSpacer(*this); }
      virtual ElementType type() const { return VSPACER; }
      virtual void draw1(Painter&);
      };

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element {
   public:
      Lasso(Score*);
      virtual Lasso* clone() const { return new Lasso(*this); }
      virtual ElementType type() const   { return LASSO; }
      virtual void draw1(Painter&);
      };

//---------------------------------------------------------
//   Line
//---------------------------------------------------------

class Line : public Element {
      Spatium _width;
      Spatium _len;

   protected:
      bool vertical;

   public:
      Line(Score*);
      Line(Score*, bool vertical);
      Line &operator=(const Line&);

      virtual Line* clone() const { return new Line(*this); }
      virtual ElementType type() const { return LINE; }
      virtual QRectF bbox() const;

      virtual void draw1(Painter&);
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomNode);
      void dump() const;

      Spatium len()    const { return _len; }
      Spatium lineWidth()  const { return _width; }
      void setLen(Spatium);
      void setLineWidth(Spatium);
      };

//---------------------------------------------------------
//   Compound
//---------------------------------------------------------

class Compound : public Element {
      ElementList elemente;

   public:
      Compound(Score*);
      virtual ElementType type() const = 0;

      virtual void draw1(Painter&);
      virtual void addElement(Element*, double x, double y);
      void clear();
      virtual void setSelected(bool f);
      virtual void setVisible(bool);
      virtual QRectF bbox() const;
      };

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

/**
 The KeySig class represents a Key Signature on a staff
*/

class KeySig : public Element {
      void add(Painter&, bool, double x, double y);
      void addLayout(bool flat, double x, double y);

   public:
      KeySig(Score*);
      virtual void setSubtype(int n);
      virtual KeySig* clone() const { return new KeySig(*this); }
      virtual void draw1(Painter&);
      virtual ElementType type() const { return KEYSIG; }
      virtual bool acceptDrop(const QPointF&, int, const QDomNode&) const;
      virtual void drop(const QPointF&, int, const QDomNode&);
      virtual void layout();
      };

//---------------------------------------------------------
//   RubberBand
//---------------------------------------------------------

class RubberBand : public Element {
      QPointF _p1, _p2;

   public:
      RubberBand(Score* s) : Element(s) {}
      virtual RubberBand* clone() const { return new RubberBand(*this); }
      virtual ElementType type() const { return RUBBERBAND; }
      virtual void draw(Painter&);

      void set(const QPointF& p1, const QPointF& p2) { _p1 = p1; _p2 = p2; }
      QPointF p1() const { return _p1; }
      QPointF p2() const { return _p2; }
      };

//---------------------------------------------------------
//   Volta
//    brackets
//---------------------------------------------------------

enum {
      PRIMA_VOLTA = 1, SECONDA_VOLTA, TERZA_VOLTA, SECONDA_VOLTA2
      };

class Volta : public Element {
      QPointF _p1, _p2;

   public:
      Volta(Score* s) : Element(s) {}
      virtual Volta* clone() const { return new Volta(*this); }
      virtual ElementType type() const { return VOLTA; }
      virtual void draw1(Painter&);
      virtual void layout();
      void setLen(qreal);
      virtual void write(Xml&) const;
      virtual void read(QDomNode);
      virtual QRectF bbox() const;
      };

#endif

