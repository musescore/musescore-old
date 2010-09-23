//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "xml.h"
#include "globals.h"

/**
 \file
 Definition of classes Element, ElementList, StaffLines.
*/

#include "spatium.h"

class Xml;
class Measure;
class Staff;
class Score;
class Sym;
class ScoreView;
class Segment;
class TextStyle;

/**
  The value of this enum determines the "stacking order" the elements are
  put on the canvas.
*/

enum ElementType {
      INVALID = -1,
      SYMBOL  = 0,
      TEXT, SLUR_SEGMENT, BAR_LINE,
      STEM_SLASH, LINE, BRACKET,
      ARPEGGIO,
      ACCIDENTAL, NOTE,
      STEM,
      CLEF, KEYSIG, TIMESIG, REST,
      BREATH, GLISSANDO,
      REPEAT_MEASURE,
      IMAGE,
/*18*/TIE,
      ARTICULATION, DYNAMIC, PAGE, BEAM, HOOK, LYRICS, MARKER, JUMP,
      TUPLET,
      TEMPO_TEXT,
      STAFF_TEXT,
      HARMONY,
      VOLTA,
      HAIRPIN_SEGMENT, OTTAVA_SEGMENT, PEDAL_SEGMENT, TRILL_SEGMENT, TEXTLINE_SEGMENT,
      VOLTA_SEGMENT,
      LAYOUT_BREAK, SPACER,
      LEDGER_LINE,
      NOTEHEAD, TREMOLO,
      MEASURE, STAFF_LINES,
      CURSOR, SELECTION, LASSO, SHADOW_NOTE, RUBBERBAND,

      // not drawable elements:
      HAIRPIN, OTTAVA, PEDAL, TRILL, TEXTLINE,
      SEGMENT, SYSTEM, COMPOUND, CHORD, SLUR,

      // special types for drag& drop:
      ELEMENT, ELEMENT_LIST, STAFF_LIST, MEASURE_LIST, LAYOUT,
      HBOX, VBOX,
      ICON,
      ACCIDENTAL_BRACKET,
      MAXTYPE
      };

//---------------------------------------------------------
///   \brief Unit of horizontal measure
//---------------------------------------------------------

class Space {
      double _lw;       // space needed to the left
      double _rw;       // space needed to the right

   public:
      Space() : _lw(0.0), _rw(0.0)  {}
      Space(double a, double b) : _lw(a), _rw(b) {}
      double lw() const             { return _lw; }
      double rw() const             { return _rw; }
      double& rLw()                 { return _lw; }
      double& rRw()                 { return _rw; }
      void setLw(double e)          { _lw = e; }
      void setRw(double m)          { _rw = m; }
      inline void max(const Space& s);
      };

//---------------------------------------------------------
///   \brief base class of score layout elements
///
///   The Element class is the virtual base class of all
///   score layout elements.
///
///   More details: TBD
//---------------------------------------------------------

class Element {
      Q_DECLARE_TR_FUNCTIONS(Element)

      Element* _parent;

      bool _selected:1;           ///< set if element is selected
      bool _selectable:1;         ///< true if element is selectable
      mutable bool _dropTarget:1; ///< true, if element accepts drops
      bool _generated:1;          ///< automatically generated Element
      bool _visible:1;            ///< visibility attribute
      bool _systemFlag:1;         ///< system elements appear on all excerpts

      int _subtype;

      int _track;                 ///< staffIdx * VOICES + voice
                                  ///< -1 if this is a system element

      int _tick;
      QColor _color;
      double _mag;                ///< standard magnification (derived value)

      QPointF _userOff;           ///< Offset from normal layout position:
                                  ///< user dragged object this amount.
                                  ///< depends on Spatium ("space") units!

   protected:
      Score* _score;
      QPointF _pos;               ///< Reference position, relative to _parent.
                                  ///< Usually set from layout().
      Align  _align;
      double _xoff, _yoff;
      QPointF _reloff;
      OffsetType _offsetType;

      int _mxmlOff;               ///< MusicXML offset in ticks.
                                  ///< Note: interacts with userXoffset.

      mutable QRectF _bbox;       ///< Bounding box relative to _pos + _userOff
                                  ///< valid after call to layout()

      QPointF _startDragPosition;   ///< used during drag


   public:
      Element(Score* s = 0);
      Element(const Element&);
      virtual ~Element();
      Element &operator=(const Element&);
      virtual Element* clone() const = 0;

      Score* score() const                    { return _score;      }
      void setScore(Score* s)                 { _score = s;         }
      Element* parent() const                 { return _parent;     }
      void setParent(Element* e)              { _parent = e;        }

      double spatium() const;

      bool selected() const                   { return _selected;   }
      virtual void setSelected(bool f)        { _selected = f;      }
      bool selectable() const                 { return _selectable; }
      void setSelectable(bool val)            { _selectable = val;  }

      bool visible() const                    { return _visible;    }
      virtual void setVisible(bool f)         { _visible = f;       }
      bool generated() const                  { return _generated;  }
      void setGenerated(bool val)             { _generated = val;   }
      bool dropTarget() const                 { return _dropTarget; }
      void setDropTarget(bool f) const        { _dropTarget = f;    }

      const QPointF& ipos() const             { return _pos;                    }
      virtual QPointF pos() const             { return _pos + _userOff;         }
      virtual double x() const                { return _pos.x() + _userOff.x(); }
      virtual double y() const                { return _pos.y() + _userOff.y(); }
      void setPos(const QPointF& p)           { _pos = p;                }
      void setPos(double x, double y)         { _pos.rx() = x; _pos.ry() = y;    }
      void movePos(const QPointF& p)          { _pos += p;               }
      qreal& rxpos()                          { return _pos.rx();        }
      qreal& rypos()                          { return _pos.ry();        }
      virtual void move(double xd, double yd) { _pos += QPointF(xd, yd); }
      virtual void move(const QPointF& s)     { _pos += s;               }

      virtual QPointF canvasPos() const;      ///< position in canvas coordinates

      const QPointF& userOff() const          { return _userOff;  }
      void setUserOff(const QPointF& o)       { _userOff = o;     }
      void setUserXoffset(qreal v)            { _userOff.setX(v); }
      void setUserYoffset(qreal v)            { _userOff.setY(v); }
      int mxmlOff() const                     { return _mxmlOff;  }
      void setMxmlOff(int o)                  { _mxmlOff = o;     }

      virtual QRectF bbox() const             { return _bbox;              }
      virtual double height() const           { return bbox().height();    }
      virtual void setHeight(qreal v)         { return _bbox.setHeight(v); }
      virtual double width() const            { return bbox().width();     }
      virtual void setWidth(qreal v)          { return _bbox.setWidth(v);  }
      QRectF abbox() const;
      virtual void setbbox(const QRectF& r) const   { _bbox = r;           }
      virtual bool contains(const QPointF& p) const;
      bool intersects(const QRectF& r) const;
      virtual QPainterPath shape() const;
      virtual qreal baseLine() const          { return -height();       }

      virtual ElementType type() const = 0;
      int subtype() const                     { return _subtype;        }
      virtual void setSubtype(int val)        { _subtype = val;         }
      bool isChordRest() const {
            return type() == REST || type() == CHORD;
            }
      bool isDurationElement() const {
            return isChordRest() || (type() == TUPLET);
            }
      bool isSLine() const {
            return type() == HAIRPIN || type() == OTTAVA || type() == PEDAL
               || type() == TRILL || type() == VOLTA || type() == TEXTLINE;
            }

      virtual void draw(QPainter&) const {}

      void writeProperties(Xml& xml, const Element* proto = 0) const;
      bool readProperties(QDomElement);

      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      virtual bool isMovable() const          { return false; }
      virtual QRectF drag(const QPointF& s);
      virtual void endDrag()                  {}
      virtual QLineF dragAnchor() const       { return QLineF(); }

      virtual bool isEditable()   { return !_generated; }
      virtual void startEdit(ScoreView*, const QPointF&) {}
      virtual bool edit(ScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void editDrag(int, const QPointF&);
      virtual void endEditDrag()                               {}
      virtual void endEdit()                                   {}
      virtual void updateGrips(int* grips, QRectF*) const      { *grips = 0;       }
      virtual QPointF gripAnchor(int) const   { return QPointF(); }

      int track() const                       { return _track; }
      virtual void setTrack(int val)          { _track = val;  }
      int staffIdx() const                    { return _track / VOICES; }
      int voice() const                       { return _track % VOICES; }
      void setVoice(int v)                    { _track = (_track / VOICES) + v; }
      Staff* staff() const;

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);

      virtual void layout();
      virtual void spatiumChanged(double /*oldValue*/, double /*newValue*/);
      virtual void resetMode() {}

      // debug functions
      virtual void dump() const;
      const char* name() const;
      virtual QString userName() const;
      void dumpQPointF(const char*) const;

      bool operator>(const Element&) const;

      int tick() const                { return _tick;         }
      virtual void setTick(int t)     { _tick = t;            }

      virtual Space space() const     { return Space(0.0, width()); }

      QColor color() const            { return _color; }
      QColor curColor() const;
      void setColor(const QColor& c)  { _color = c;    }
      static ElementType readType(QDomElement& node, QPointF*);

      virtual QByteArray mimeData(const QPointF&) const;
/**
 Return true if this element accepts a drop at canvas relative \a pos
 of given element \a type and \a subtype.

 Reimplemented by elements that accept drops. Used to change cursor shape while
 dragging to indicate drop targets.
*/
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const { return false; }

/**
 Handle a dropped element at canvas relative \a pos of given element
 \a type and \a subtype. Returns dropped element if any.

 Reimplemented by elements that accept drops.
*/
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*) { return 0;}

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
 extend property menu by elemement specific items
 */
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

/**
 delivers mouseEvent to element in edit mode
 returns true if mouse event is accepted by element
 */
      virtual bool mousePress(const QPointF&, QMouseEvent*) { return false; }

      mutable int itemDiscovered;     ///< helper flag for bsp

      virtual QList<Prop> properties(Xml&, const Element* proto = 0) const;
      virtual void scanElements(void* data, void (*func)(void*, Element*)) { func(data, this); }

      virtual void toDefault() {  setUserOff(QPointF()); }

      double mag() const                        { return _mag;   }
      double magS() const;
      virtual void setMag(double val)           { _mag = val;    }

/**
 Layout hints
 for some element types this hints are fixed and not saved to
 the *.msc file
 */
      Align align() const                   { return _align;        }
      OffsetType offsetType() const         { return _offsetType;   }
      double xoff() const                   { return _xoff;         }
      double yoff() const                   { return _yoff;         }
      QPointF reloff() const                { return _reloff;       }
      void setReloff(const QPointF& val)    { _reloff = val;        }
      void setAlign(Align val)              { _align  = val;        }
      void setXoff(double val)              { _xoff   = val;        }
      void setYoff(double val)              { _yoff   = val;        }
      void setOffsetType(OffsetType val)    { _offsetType = val;    }

      bool systemFlag() const               { return _systemFlag;   }
      void setSystemFlag(bool f)            { _systemFlag = f;      }

      bool isTextB() { return
                  type()  == TEXT
                || type() == LYRICS
                || type() == DYNAMIC
                || type() == HARMONY
                || type() == MARKER
                || type() == JUMP
                || type() == STAFF_TEXT
                || type() == TEMPO_TEXT;
            }
      double point(const Spatium sp) const { return sp.val() * spatium(); }

      //
      // check element for consistency; return false if element
      // is not valid
      //
      virtual bool check() const { return true; }

      QPointF startDragPosition() const           { return _startDragPosition; }
      void setStartDragPosition(const QPointF& v) { _startDragPosition = v; }

      static const char* name(ElementType type);
      static Element* create(ElementType type, Score*);
      static ElementType name2type(const QString&);
      static Element* name2Element(const QString&, Score*);
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

      void add(Element*);
      void move(Element* el, int tick);
      void write(Xml&, const char* name) const;
      void read(QDomElement);
      };

typedef ElementList::iterator iElement;
typedef ElementList::const_iterator ciElement;

//---------------------------------------------------------
//   StaffLines
//---------------------------------------------------------

/**
 The StaffLines class is the graphic representation of a staff,
 it draws the horizontal staff lines.
*/

class StaffLines : public Element {
      // Spatium lineWidth;
      qreal _width;

   public:
      StaffLines(Score*);
      virtual StaffLines* clone() const    { return new StaffLines(*this); }
      virtual ElementType type() const     { return STAFF_LINES; }
      Measure* measure() const             { return (Measure*)parent(); }
      void setWidth(qreal v)               { _width = v;         }
      virtual QRectF bbox() const;
      virtual void draw(QPainter&) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual QPointF canvasPos() const;   ///< position in canvas coordinates
      int lines() const                    { return subtype(); }
      void setLines(int val)               { setSubtype(val);  }
      double y1() const;
      double y2() const;
      };

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

class Cursor : public Element {
      ScoreView* viewer;
      bool _blink;
      bool _on;
      double _h;
      Segment* _seg;

   public:
      Cursor(Score*, ScoreView*);
      virtual Cursor* clone() const    { return new Cursor(*this); }
      virtual ElementType type() const { return CURSOR; }
      virtual void draw(QPainter&) const;
      virtual QRectF bbox() const;

      void setOn(bool f)               { _on = f; }
      bool isOn() const                { return _on; }
      void blink()                     { _blink = !_blink; }
      void noBlinking()                { _blink = true; }
      virtual void setHeight(qreal v)  { _h = v; }
      void setSegment(Segment* s)      { _seg = s; }
      Segment* segment() const         { return _seg;  }
      };

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

class Lasso : public Element {
   public:
      Lasso(Score*);
      virtual Lasso* clone() const       { return new Lasso(*this); }
      virtual ElementType type() const   { return LASSO; }
      virtual void draw(QPainter&) const;
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
      virtual void layout();

      virtual void draw(QPainter&) const;
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement);
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
      QList<Element*> elemente;

   protected:
      const QList<Element*>& getElemente() const { return elemente; }

   public:
      Compound(Score*);
      virtual ElementType type() const = 0;

      virtual void draw(QPainter&) const;
      virtual void addElement(Element*, double x, double y);
      void clear();
      virtual void setSelected(bool f);
      virtual void setVisible(bool);
      virtual QRectF bbox() const;
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
      virtual void draw(QPainter&) const;

      void set(const QPointF& p1, const QPointF& p2) { _p1 = p1; _p2 = p2; }
      QPointF p1() const { return _p1; }
      QPointF p2() const { return _p2; }
      };

//---------------------------------------------------------
//   Icon
//    dummy element, used for drag&drop
//---------------------------------------------------------

class Icon : public Element {
      QAction* _action;

   public:
      Icon(Score* s) : Element(s) {}
      virtual Icon* clone() const        { return new Icon(*this); }
      virtual ElementType type() const   { return ICON;    }
      void setAction(QAction* a)         { _action = a;    }
      QIcon icon() const                 { return _action->icon();   }
      QAction* action() const            { return _action; }
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      };

extern void collectElements(void*, Element*);

#endif

