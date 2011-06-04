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

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QLineF>
#include <QtCore/QString>

#include "globals.h"
#include "../al/color.h"

/**
 \file
 Definition of classes Element, ElementList, StaffLines.
*/

#include "spatium.h"

class Measure;
class Staff;
class Score;
class Sym;
class Segment;
class Element;
class Painter;
class XmlReader;

//---------------------------------------------------------
//   ElementFlag
//---------------------------------------------------------

enum ElementFlag {
      ELEMENT_SYSTEM_FLAG = 0x1,
      ELEMENT_DROP_TARGET = 0x2,
      ELEMENT_SELECTABLE  = 0x4,
      ELEMENT_MOVABLE     = 0x8,
      ELEMENT_SEGMENT     = 0x10
      };

typedef QFlags<ElementFlag> ElementFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(ElementFlags)

//---------------------------------------------------------
///   \brief Unit of horizontal measure
//---------------------------------------------------------

class Space {
      qreal _lw;       // space needed to the left
      qreal _rw;       // space needed to the right

   public:
      Space() : _lw(0.0), _rw(0.0)  {}
      Space(qreal a, qreal b) : _lw(a), _rw(b) {}
      qreal lw() const             { return _lw; }
      qreal rw() const             { return _rw; }
      qreal& rLw()                 { return _lw; }
      qreal& rRw()                 { return _rw; }
      void setLw(qreal e)          { _lw = e; }
      void setRw(qreal m)          { _rw = m; }
      inline void max(const Space& s);
      };

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

class LinkedElements : public QList<Element*> {
      static int _linkId;
      int _lid;                     // unique id for every linked list

   public:
      LinkedElements();
      LinkedElements(int id);
      void setLid(int val);
      int lid() const                         { return _lid;                }
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
      Element* _parent;

      bool _selected:1;           ///< set if element is selected
      bool _generated:1;          ///< automatically generated Element
      bool _visible:1;            ///< visibility attribute

      mutable ElementFlags _flags;

      int _subtype;
      int _track;                 ///< staffIdx * VOICES + voice
                                  ///< -1 if this is a system element
      Color _color;
      qreal _mag;                ///< standard magnification (derived value)

      QPointF _pos;               ///< Reference position, relative to _parent.
      QPointF _userOff;           ///< offset from normal layout position:
                                  ///< user dragged object this amount.
                                  ///< depends on Spatium ("space") units!
      QPointF _readPos;

   protected:
      Score* _score;

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
      virtual void setScore(Score* s)         { _score = s;         }
      Element* parent() const                 { return _parent;     }
      void setParent(Element* e)              { _parent = e;        }

      qreal spatium() const;

      bool selected() const                   { return _selected;   }
      virtual void setSelected(bool f)        { _selected = f;      }

      bool visible() const                    { return _visible;    }
      virtual void setVisible(bool f)         { _visible = f;       }
      bool generated() const                  { return _generated;  }
      void setGenerated(bool val)             { _generated = val;   }

      const QPointF& ipos() const             { return _pos;                    }
      virtual QPointF pos() const             { return _pos + _userOff;         }
      virtual qreal x() const                { return _pos.x() + _userOff.x(); }
      virtual qreal y() const                { return _pos.y() + _userOff.y(); }
      void setPos(qreal x, qreal y);
      void setPos(const QPointF& p)           { setPos(p.x(), p.y());           }
      void movePos(const QPointF& p)          { _pos += p;               }
      qreal& rxpos()                          { return _pos.rx();        }
      qreal& rypos()                          { return _pos.ry();        }
      virtual void move(qreal xd, qreal yd) { _pos += QPointF(xd, yd); }
      virtual void move(const QPointF& s)     { _pos += s;               }

      virtual QPointF canvasPos() const;      ///< position in canvas coordinates

      const QPointF& userOff() const          { return _userOff;  }
      void setUserOff(const QPointF& o)       { _userOff = o;     }
      void setUserXoffset(qreal v)            { _userOff.setX(v); }
      void setUserYoffset(qreal v)            { _userOff.setY(v); }
      int mxmlOff() const                     { return _mxmlOff;  }
      void setMxmlOff(int o)                  { _mxmlOff = o;     }

      QPointF readPos() const                 { return _readPos;           }
      void setReadPos(const QPointF& p)       { _readPos = p;              }
      void adjustReadPos();

      virtual QRectF bbox() const             { return _bbox;              }
      virtual qreal height() const           { return bbox().height();    }
      virtual void setHeight(qreal v)         { _bbox.setHeight(v);        }
      virtual qreal width() const            { return bbox().width();     }
      virtual void setWidth(qreal v)          { _bbox.setWidth(v);         }
      QRectF abbox() const;
      virtual void setbbox(const QRectF& r) const   { _bbox = r;           }
      virtual bool contains(const QPointF& p) const;
      bool intersects(const QRectF& r) const;
//      virtual QPainterPath shape() const;
      virtual qreal baseLine() const          { return -height();       }

      virtual ElementType type() const = 0;
      int subtype() const                     { return _subtype;        }
      virtual void setSubtype(int val)        { _subtype = val;         }
      bool isChordRest() const                { return type() == REST || type() == CHORD;   }
      bool isDurationElement() const          { return isChordRest() || (type() == TUPLET); }
      bool isSLine() const {
            return type() == HAIRPIN || type() == OTTAVA || type() == PEDAL
               || type() == TRILL || type() == VOLTA || type() == TEXTLINE;
            }

      virtual void draw(Painter*) const {}

      bool readProperties(XmlReader*);

      virtual void read(XmlReader*);

      int track() const                       { return _track; }
      virtual void setTrack(int val)          { _track = val;  }
      int staffIdx() const                    { return _track / VOICES; }
      int voice() const                       { return _track % VOICES; }
      void setVoice(int v)                    { _track = (_track / VOICES) + v; }
      Staff* staff() const;

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);

      virtual void layout() {}
      virtual void resetMode() {}

      // debug functions
      const char* name() const;
      virtual QString userName() const;
      void dumpQPointF(const char*) const;

      virtual Space space() const     { return Space(0.0, width()); }

      Color color() const            { return _color; }
      Color curColor() const;
      void setColor(const Color& c)  { _color = c;    }
      static ElementType readType(XmlReader*, QPointF*);

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

      mutable int itemDiscovered;     ///< helper flag for bsp

      virtual void scanElements(void* data, void (*func)(void*, Element*)) { func(data, this); }

      qreal mag() const                        { return _mag;   }
      qreal magS() const;
      virtual void setMag(qreal val)           { _mag = val;    }

      bool isText() { return
                  type()  == TEXT
                || type() == LYRICS
                || type() == DYNAMIC
                || type() == HARMONY
                || type() == MARKER
                || type() == JUMP
                || type() == STAFF_TEXT
                || type() == INSTRUMENT_CHANGE
                || type() == TEMPO_TEXT;
            }
      qreal point(const Spatium sp) const { return sp.val() * spatium(); }

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

      void setFlag(ElementFlag f, bool v)  {
            if (v)
                  _flags |= f;
            else
                  _flags &= ~f;
            }
      bool flag(ElementFlag f) const   { return _flags & f; }
      void setFlags(ElementFlags f)    { _flags = f;    }
      ElementFlags flags() const       { return _flags; }
      bool systemFlag() const          { return flag(ELEMENT_SYSTEM_FLAG); }
      void setSystemFlag(bool f)       { setFlag(ELEMENT_SYSTEM_FLAG, f);  }
      bool selectable() const          { return flag(ELEMENT_SELECTABLE);  }
      void setSelectable(bool val)     { setFlag(ELEMENT_SELECTABLE, val); }
      bool dropTarget() const          { return flag(ELEMENT_DROP_TARGET); }
      void setDropTarget(bool v) const {
            if (v)
                  _flags |= ELEMENT_DROP_TARGET;
            else
                  _flags &= ~ELEMENT_DROP_TARGET;
            }
      virtual bool isMovable() const   { return flag(ELEMENT_MOVABLE);     }
      bool isSegment() const           { return flag(ELEMENT_SEGMENT);     }
      };

//---------------------------------------------------------
//   ElementList
//---------------------------------------------------------

class ElementList : public QList<Element*> {
   public:
      ElementList() {}
      bool remove(Element*);
      void replace(Element* old, Element* n);
      void read(XmlReader*);
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
      qreal _dist;

   public:
      StaffLines(Score*);
      virtual StaffLines* clone() const    { return new StaffLines(*this); }
      virtual ElementType type() const     { return STAFF_LINES; }
      Measure* measure() const             { return (Measure*)parent(); }
      void setWidth(qreal v)               { _width = v;         }
      virtual void draw(Painter*) const;
      virtual QPointF canvasPos() const;   ///< position in canvas coordinates
      qreal y1() const;
      qreal y2() const;
      virtual QRectF bbox() const;
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

      virtual void draw(Painter*) const;
      bool readProperties(XmlReader*);
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
      Compound(const Compound&);
      virtual ElementType type() const = 0;

      virtual void draw(Painter*) const;
      virtual void addElement(Element*, qreal x, qreal y);
      void clear();
      virtual void setSelected(bool f);
      virtual void setVisible(bool);
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
      virtual void draw(Painter*) const;

      void set(const QPointF& p1, const QPointF& p2) { _p1 = p1; _p2 = p2; }
      QPointF p1() const { return _p1; }
      QPointF p2() const { return _p2; }
      };

extern void collectElements(void*, Element*);
extern bool elementLessThan(const Element* const, const Element* const);

#endif

