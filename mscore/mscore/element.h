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

#include "plist.h"
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
      INSTRUMENT_NAME1, INSTRUMENT_NAME2, FINGERING,
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
//    virtual base class of score layout element
//---------------------------------------------------------

class Element {
      Element* _next;
      Element* _prev;
      Element* _parent;
      Measure* _anchor;

      bool _selected;
      bool _dropTarget;
      bool _generated;  // automatically generated Element
      bool _visible;    // visibility attribute

      int _subtype;

      int _voice;       // 0 - VOICES
      Staff* _staff;
      MTime _time;
      QColor _color;

      void init();

   protected:
      Score* _score;
      QPointF _pos;     // point: reference position, relative to _parent
                        // usually set from layout()
      QPointF _userOff; // offset from normal layout position
                        // user dragged object this amount
                        // in Spatium ("space") units!
      int _mxmlOff;     // MusicXML offset in ticks
                        // note: interacts with userXoffset

      mutable QRectF _bbox;     // bounding box relative to _pos + _userOff
      mutable MTime _duration;  // lazy evaluation

   public:
      Element(Score*);
      virtual ~Element() {}
      Element(const Element&);
      Element &operator=(const Element&);

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
      virtual QPointF pos() const             { return _pos + _userOff * _spatium;         }
      virtual double x() const                { return _pos.x() + _userOff.x() * _spatium; }
      virtual double y() const                { return _pos.y() + _userOff.y() * _spatium; }
      virtual void setPos(const QPointF& p)   { _pos = p;                }
      virtual void setPos(double x, double y) { _pos = QPointF(x, y);    }
      virtual void move(double xd, double yd) { _pos += QPointF(xd, yd); }
      virtual void move(const QPointF& s)     { _pos += s;               }

      QPointF aref() const;     // canvas reference pos
      const QPointF& userOff() const          { return _userOff;  }
      void setUserOff(const QPointF& o)       { _userOff = o;     }
      void setUserXoffset(qreal v)            { _userOff.setX(v); }
      void setUserYoffset(qreal v)            { _userOff.setY(v); }
      int mxmlOff() const                     { return _mxmlOff;  }
      void setMxmlOff(int o)                  { _mxmlOff = o;     }

      virtual bool contains(const QPointF& p) const;  // p relative to parent()
      bool intersects(const QRectF& r) const;       // r relative to parent()

      QRectF abbox() const                    { return bbox().translated(aref()); }
      QPointF apos() const                    { return aref();          }

      void setbbox(const QRectF& r)           { _bbox = r;              }
      virtual const QRectF& bbox() const      { return _bbox;           }
      void setWidth(double v)                 { _bbox.setWidth(v);      }
      void setHeight(double v)                { _bbox.setHeight(v);     }
      void setBboxX(qreal x)                  { _bbox.setX(x);          }
      void setBboxY(qreal y)                  { _bbox.setY(y);          }
      void orBbox(const QRectF& f)            { _bbox |= f;             }
      virtual double height() const           { return bbox().height(); }
      virtual double width() const            { return bbox().width();  }

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

      virtual void write(Xml&) const  {}
      virtual void read(QDomNode) {}

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
      virtual void bboxUpdate()       {}
      virtual QByteArray mimeData() const;
      virtual bool acceptDrop(int, int) const { return false; }
      virtual void drop(const QPointF&, int, int) {}
      };

//---------------------------------------------------------
//   ElementList
//---------------------------------------------------------

class ElementList : public pstl::plist<Element*> {
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
typedef ElementList::reverse_iterator riElement;

//---------------------------------------------------------
//   SStaff
//    graphic representation of a staff
//---------------------------------------------------------

class SStaff : public Element {
      Spatium lineWidth;
      int lines;

   public:
      SStaff(Score*);
      virtual ElementType type() const { return STAFF; }

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
      virtual ElementType type() const { return VSPACER; }
      virtual void draw1(Painter&);
      };

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element {
   public:
      Lasso(Score*);
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
      Line(const Line&);
      Line &operator=(const Line&);

      virtual ElementType type() const { return LINE; }

      virtual void draw1(Painter&);
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomNode);
      void dump() const;

      Spatium len()    const { return _len; }
      Spatium lineWidth()  const { return _width; }
      void setLen(Spatium);
      void setLineWidth(Spatium);
      virtual void bboxUpdate();
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
      };

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

class Accidental : public Compound {
      int val;
      bool small;

   public:
      Accidental(Score*, int i, bool small);
      virtual ElementType type() const { return ACCIDENTAL; }
      int idx() const                  { return val; }
      void setIdx(int v);
      };

//---------------------------------------------------------
//   KeySig
//    Key Signature
//---------------------------------------------------------

class KeySig : public Compound {
      int val;
      int off;

   public:
      KeySig(Score*);
      KeySig(Score*, int, int);
      virtual ElementType type() const { return KEYSIG; }
      void setIdx(int v, int yoffset);
      int idx() const { return val; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      };

//---------------------------------------------------------
//   RubberBand
//---------------------------------------------------------

class RubberBand : public Element {
      QPointF _p1, _p2;

   public:
      RubberBand(Score* s) : Element(s) {}
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
      virtual ElementType type() const { return VOLTA; }
      virtual void draw1(Painter&);
      virtual void layout();
      void setLen(qreal);
      virtual void write(Xml&) const;
      virtual void read(QDomNode);
      };

#endif

