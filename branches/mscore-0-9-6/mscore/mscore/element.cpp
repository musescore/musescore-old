//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

/**
 \file
 Implementation of Element, ElementList, StaffLines.
*/

#include "element.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "preferences.h"
#include "staff.h"
#include "utils.h"
#include "sym.h"
#include "symbol.h"
#include "clef.h"
#include "scoreview.h"
#include "volta.h"
#include "ottava.h"
#include "textline.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "keysig.h"
#include "timesig.h"
#include "barline.h"
#include "arpeggio.h"
#include "breath.h"
#include "bracket.h"
#include "chordrest.h"
#include "accidental.h"
#include "dynamics.h"
#include "text.h"
#include "note.h"
#include "tremolo.h"
#include "layoutbreak.h"
#include "repeat.h"
#include "page.h"
#include "system.h"
#include "stafftext.h"
#include "scoreview.h"
#include "glissando.h"
#include "articulation.h"
#include "chord.h"
#include "spacer.h"
#include "mscore.h"
#include "tempotext.h"
#include "harmony.h"
#include "lyrics.h"
#include "rest.h"
#include "slur.h"
#include "measure.h"

extern bool debugMode;
extern bool showInvisible;

static const char* elementNames[] = {
      QT_TRANSLATE_NOOP("elementName", "Symbol"),
      QT_TRANSLATE_NOOP("elementName", "Text"),
      QT_TRANSLATE_NOOP("elementName", "SlurSegment"),
      QT_TRANSLATE_NOOP("elementName", "BarLine"),
      QT_TRANSLATE_NOOP("elementName", "StemSlash"),
      QT_TRANSLATE_NOOP("elementName", "Line"),
      QT_TRANSLATE_NOOP("elementName", "Bracket"),
      QT_TRANSLATE_NOOP("elementName", "Arpeggio"),
      QT_TRANSLATE_NOOP("elementName", "Accidental"),
      QT_TRANSLATE_NOOP("elementName", "Note"),
      QT_TRANSLATE_NOOP("elementName", "Stem"),
      QT_TRANSLATE_NOOP("elementName", "Clef"),
      QT_TRANSLATE_NOOP("elementName", "KeySig"),
      QT_TRANSLATE_NOOP("elementName", "TimeSig"),
      QT_TRANSLATE_NOOP("elementName", "Rest"),
      QT_TRANSLATE_NOOP("elementName", "Breath"),
      QT_TRANSLATE_NOOP("elementName", "Glissando"),
      QT_TRANSLATE_NOOP("elementName", "RepeatMeasure"),
      QT_TRANSLATE_NOOP("elementName", "Image"),
      QT_TRANSLATE_NOOP("elementName", "Tie"),
      QT_TRANSLATE_NOOP("elementName", "Articulation"),
      QT_TRANSLATE_NOOP("elementName", "Dynamic"),
      QT_TRANSLATE_NOOP("elementName", "Page"),
      QT_TRANSLATE_NOOP("elementName", "Beam"),
      QT_TRANSLATE_NOOP("elementName", "Hook"),
      QT_TRANSLATE_NOOP("elementName", "Lyrics"),
      QT_TRANSLATE_NOOP("elementName", "Marker"),
      QT_TRANSLATE_NOOP("elementName", "Jump"),
      QT_TRANSLATE_NOOP("elementName", "Tuplet"),
      QT_TRANSLATE_NOOP("elementName", "Tempo"),
      QT_TRANSLATE_NOOP("elementName", "StaffText"),
      QT_TRANSLATE_NOOP("elementName", "Harmony"),
      QT_TRANSLATE_NOOP("elementName", "Volta"),
      QT_TRANSLATE_NOOP("elementName", "HairpinSegment"),
      QT_TRANSLATE_NOOP("elementName", "OttavaSegment"),
      QT_TRANSLATE_NOOP("elementName", "PedalSegment"),
      QT_TRANSLATE_NOOP("elementName", "TrillSegment"),
      QT_TRANSLATE_NOOP("elementName", "TextLineSegment"),
      QT_TRANSLATE_NOOP("elementName", "VoltaSegment"),
      QT_TRANSLATE_NOOP("elementName", "LayoutBreak"),
      QT_TRANSLATE_NOOP("elementName", "Spacer"),
      QT_TRANSLATE_NOOP("elementName", "LedgerLine"),
      QT_TRANSLATE_NOOP("elementName", "NoteHead"),
      QT_TRANSLATE_NOOP("elementName", "Tremolo"),
      QT_TRANSLATE_NOOP("elementName", "Measure"),
      QT_TRANSLATE_NOOP("elementName", "StaffLines"),
      QT_TRANSLATE_NOOP("elementName", "Cursor"),
      QT_TRANSLATE_NOOP("elementName", "Selection"),
      QT_TRANSLATE_NOOP("elementName", "Lasso"),
      QT_TRANSLATE_NOOP("elementName", "ShadowNote"),
      QT_TRANSLATE_NOOP("elementName", "RubberBand"),
      QT_TRANSLATE_NOOP("elementName", "HairPin"),
      QT_TRANSLATE_NOOP("elementName", "Ottava"),
      QT_TRANSLATE_NOOP("elementName", "Pedal"),
      QT_TRANSLATE_NOOP("elementName", "Trill"),
      QT_TRANSLATE_NOOP("elementName", "TextLine"),
      QT_TRANSLATE_NOOP("elementName", "Segment"),
      QT_TRANSLATE_NOOP("elementName", "System"),
      QT_TRANSLATE_NOOP("elementName", "Compound"),
      QT_TRANSLATE_NOOP("elementName", "Chord"),
      QT_TRANSLATE_NOOP("elementName", "Slur"),
      QT_TRANSLATE_NOOP("elementName", "Element"),
      QT_TRANSLATE_NOOP("elementName", "ElementList"),
      QT_TRANSLATE_NOOP("elementName", "StaffList"),
      QT_TRANSLATE_NOOP("elementName", "MeasureList"),
      QT_TRANSLATE_NOOP("elementName", "Layout"),
      QT_TRANSLATE_NOOP("elementName", "HBox"),
      QT_TRANSLATE_NOOP("elementName", "VBox"),
      QT_TRANSLATE_NOOP("elementName", "Icon"),
      QT_TRANSLATE_NOOP("elementName", "AccidentalBracket")
      };

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Element::spatiumChanged(double oldValue, double newValue)
      {
      _userOff *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

double Element::spatium() const
      {
      Staff* s = staff();
      double v = _score->spatium();
      return s ? v * s->mag() : v;
      }

//---------------------------------------------------------
//   magS
//---------------------------------------------------------
double Element::magS() const
      {
      return _mag * (_score->spatium() /(DPI * SPATIUM20));   
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* Element::name() const
      {
      return name(type());
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString Element::userName() const
      {
      return qApp->translate("elementName", name(type()));
      }

//---------------------------------------------------------
//   abbox
//---------------------------------------------------------

QRectF Element::abbox() const
      {
      return bbox().translated(canvasPos());
      }

//---------------------------------------------------------
//   operator >
//---------------------------------------------------------

bool Element::operator>(const Element& el) const
      {
      if (tick() == el.tick())
            return type() > el.type();
      return tick() > el.tick();
      }

Element::~Element()
      {
//      printf("~Element %p\n", this);
      if (selected())
            printf("===========~Element: selected\n");
      if (score()) {
            foreach(Element* e, score()->selection().elements()) {
                  if (e == this) {
//                        if (debugMode)
                              printf("======~Element: %p still in selection!\n", this);
//                        abort();
                        score()->deselect(this);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

Element::Element(Score* s) :
   _parent(0),
   _selected(false),
   _selectable(true),
   _dropTarget(false),
   _generated(false),
   _visible(true),
   _systemFlag(false),
   _subtype(0),
   _track(-1),
   _tick(-1),
   _color(preferences.defaultColor),
   _mag(1.0),
   _score(s),
   _align(ALIGN_LEFT | ALIGN_TOP),
   _xoff(0), _yoff(0),
   _offsetType(OFFSET_SPATIUM),
   _mxmlOff(0),
   itemDiscovered(0)
      {
      }

Element::Element(const Element& e)
      {
      _parent     = e._parent;
      _selected   = e._selected;
      _selectable = e._selectable;
      _dropTarget = e._dropTarget;
      _generated  = e._generated;
      _visible    = e._visible;
      _systemFlag = e._systemFlag;
      _subtype    = e._subtype;
      _track      = e._track;
      _tick       = e._tick;
      _color      = e._color;
      _mag        = e._mag;
      _score      = e._score;
      _pos        = e._pos;
      _align      = e._align;
      _xoff       = e._xoff;
      _yoff       = e._yoff;
      _reloff     = e._reloff;
      _offsetType = e._offsetType;
      _userOff    = e._userOff;
      _mxmlOff    = e._mxmlOff;
      _bbox       = e._bbox;
      itemDiscovered = 0;
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void Element::change(Element* o, Element* n)
      {
      remove(o);
      add(n);
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Element::staff() const
      {
      if (_track == -1)
            return 0;
      return score()->staff(staffIdx());
      }

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

QColor Element::curColor() const
      {
      // the default element color is always interpreted as black in
      // printing
      if (score() && score()->printing())
            return (_color == preferences.defaultColor) ? Qt::black : _color;

      if (_dropTarget)
            return preferences.dropColor;
      if (_selected) {
            if (track() == -1)
                  return preferences.selectColor[0];
            else
                  return preferences.selectColor[voice()];
            }
      if (!_visible)
            return Qt::gray;
      return _color;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

/**
 Return update Rect relative to canvas.
*/

QRectF Element::drag(const QPointF& pos)
      {
      QRectF r(abbox());
      setUserOff(pos);
      return abbox() | r;
      }

//---------------------------------------------------------
//   canvasPos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF Element::canvasPos() const
      {
      QPointF p(_pos + _userOff);
      if (parent())
            p += parent()->canvasPos();
      return p;
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

/**
 Return true if \a p is inside the shape of the object.

 Note: \a p is in canvas coordinates
*/

bool Element::contains(const QPointF& p) const
      {
      return shape().contains(p - canvasPos());
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

/**
  Returns the shape of this element as a QPainterPath in local
  coordinates. The shape is used for collision detection and
  hit tests (contains())

  The default implementation calls bbox() to return a simple rectangular
  shape, but subclasses can reimplement this function to return a more
  accurate shape for non-rectangular elements.
*/

QPainterPath Element::shape() const
      {
      QPainterPath pp;
      pp.addRect(bbox());
      return pp;
      }

//---------------------------------------------------------
//  intersects
//---------------------------------------------------------

/**
 Return true if \a rr intersects bounding box of object.

 Note: \a rr is relative to the coordinate system of parent().
*/

bool Element::intersects(const QRectF& rr) const
      {
      QRectF r(rr);
      r.translate(pos());
      return bbox().intersects(r);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Element::layout()
      {
      QPointF o(_xoff, _yoff);
      if (_offsetType == OFFSET_SPATIUM)
            o *= spatium();
      else
            o *= DPI;
      if (parent())
            o += QPointF(_reloff.x() * parent()->width() * 0.01, _reloff.y() * parent()->height() * 0.01);
      double h = height();
      double w = width();
      QPointF p;
      if (_align & ALIGN_BOTTOM)
            p.setY(-h);
      else if (_align & ALIGN_VCENTER)
            p.setY(-(h * .5));
      else if (_align & ALIGN_BASELINE)
            p.setY(-baseLine());
      if (_align & ALIGN_RIGHT)
            p.setX(-w);
      else if (_align & ALIGN_HCENTER)
            p.setX(-(w * .5));
      setPos(p + o);
      }

//---------------------------------------------------------
//   properties
//---------------------------------------------------------

QList<Prop> Element::properties(Xml& xml, const Element* proto) const
      {
      QList<Prop> pl;
      if (_subtype) {
            QString s(subtypeName());
            if (!s.isEmpty())
                  pl.append(Prop("subtype", subtypeName()));
            }
      if (!_userOff.isNull())
            pl.append(Prop("offset", _userOff / spatium()));
      if ((track() != xml.curTrack) && (track() != -1)) {
            int t;
            t = track() + xml.trackDiff;
            pl.append(Prop("track", t));
            }
      if (selected())
            pl.append(Prop("selected", selected()));
      if (!visible())
            pl.append(Prop("visible", visible()));
      if (_tick != -1 && (_tick != xml.curTick))
            pl.append(Prop("tick", _tick));
      if (_color != preferences.defaultColor)
            pl.append(Prop("color", _color));
      if (_systemFlag && (proto == 0 || proto->systemFlag() != _systemFlag))
            pl.append(Prop("systemFlag", _systemFlag));
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Element::writeProperties(Xml& xml, const Element* proto) const
      {
      xml.prop(properties(xml, proto));
      if ((_tick != -1) && (_tick != xml.curTick || debugMode))
            xml.curTick = _tick;
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "tick") {
            setTick(score()->fileDivision(i));
            score()->curTick = _tick;
            }
      else if (tag == "subtype") {
            // do not always call Element::setSubtype():
            this->setSubtype(val);
            }
      else if (tag == "offset")
            setUserOff(readPoint(e) * spatium());
      else if (tag == "visible")
            setVisible(i);
      else if (tag == "voice")
            setTrack((_track/VOICES)*VOICES + i);
      else if (tag == "track")
            setTrack(i);
      else if (tag == "selected")
            setSelected(i);
      else if (tag == "color")
            _color = readColor(e);
      else if (tag == "systemFlag") {
            _systemFlag = i;
            if (_systemFlag)
                  _track = 0;
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Element::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Element::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      if (_subtype == 0)      // make sure setSubtype() is called at least once
            setSubtype(0);
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Element::genPropertyMenu(QMenu* popup) const
      {
      QAction* a;
      if (visible())
            a = popup->addAction(tr("Set Invisible"));
      else
            a = popup->addAction(tr("Set Visible"));
      a->setData("invisible");
      a = popup->addAction(tr("Color..."));
      a->setData("color");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Element::propertyAction(ScoreView*, const QString& s)
      {
      foreach(Element* e, score()->selection().elements()) {
            if (e->type() == type()) {
                  if (s == "invisible")
                        score()->toggleInvisible(e);
                  else if (s == "color"){
                        score()->colorItem(e);
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

/**
 Remove \a el from the list. Return true on success.
*/

bool ElementList::remove(Element* el)
      {
      int idx = indexOf(el);
      if (idx == -1)
            return false;
      removeAt(idx);
      return true;
      }

//---------------------------------------------------------
//   replace
//---------------------------------------------------------

void ElementList::replace(Element* o, Element* n)
      {
      int idx = indexOf(o);
      if (idx == -1) {
            printf("ElementList::replace: element not found\n");
            return;
            }
      QList<Element*>::replace(idx, n);
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void ElementList::move(Element* el, int tick)
      {
      int idx = indexOf(el);
      if (idx == -1) {
            printf("ElementList::move: element not found\n");
            return;
            }
      QList<Element*>::removeAt(idx);
      el->setTick(tick);
      add(el);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ElementList::add(Element* e)
      {
      int tick = e->tick();
      for (iElement ie = begin(); ie != end(); ++ie) {
            if ((*ie)->tick() > tick) {
                  insert(ie, e);
                  return;
                  }
            }
      append(e);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ElementList::write(Xml& xml) const
      {
      for (ciElement ie = begin(); ie != end(); ++ie)
            (*ie)->write(xml);
      }


//---------------------------------------------------------
//   StaffLines
//---------------------------------------------------------

StaffLines::StaffLines(Score* s)
   : Element(s)
      {
      setLines(5);
      _width = 1.0;      // dummy
      setSelectable(false);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF StaffLines::canvasPos() const
      {
      System* system = measure()->system();
      return QPointF(measure()->x() + system->x() + system->page()->x(),
         system->staff(staffIdx())->y() + system->y());
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF StaffLines::bbox() const
      {
      double _spatium = spatium();
      int l    = lines() - 1;
      qreal lw = point(score()->styleS(ST_staffLineWidth));

      switch(l) {
            case 0:
                  return QRectF(0.0, - 2.0 * _spatium - lw*.5, _width, 4 * _spatium + lw);
            case 1:
                  return QRectF(0.0,  -lw*.5, _width, 4 * _spatium + lw);
            case 2:
                  return QRectF(0.0, -lw*.5, _width, l * _spatium * 2.0 + lw);
            default:
                  return QRectF(0.0, -lw*.5, _width, l * _spatium + lw);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffLines::draw(QPainter& p) const
      {
      QPointF _pos(0.0, 0.0);
      double _spatium = spatium();

      QPen pen(p.pen());
      pen.setWidthF(point(score()->styleS(ST_staffLineWidth)));
      if (pen.widthF() * p.worldMatrix().m11() < 1.0)
            pen.setWidth(0);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);

      qreal x1 = _pos.x();
      qreal x2 = x1 + width();

      switch(lines()) {
            case 1:
                  {
                  qreal y = _pos.y() + 2 * _spatium;
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
                  break;
            case 2:
                  {
                  qreal y = _pos.y() + 1 * _spatium;
                  p.drawLine(QLineF(x1, y, x2, y));
                  y += 2 * _spatium * mag();
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
                  break;
            case 3:
                  for (int i = 0; i < lines(); ++i) {
                        qreal y = _pos.y() + i * _spatium * 2.0;
                        p.drawLine(QLineF(x1, y, x2, y));
                        }
                  break;

            default:
                  for (int i = 0; i < lines(); ++i) {
                        qreal y = _pos.y() + i * _spatium;
                        p.drawLine(QLineF(x1, y, x2, y));
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

double StaffLines::y1() const
      {
      double _spatium = spatium();

      double y = measure()->system()->staff(staffIdx())->y();
      switch(lines()) {
            case 1:
                  return y + _pos.y() + 1 * _spatium;
            case 2:
                  return y + _pos.y() + 1 * _spatium;
            case 3:
            default:
                  return y + _pos.y();
            }
      }

//---------------------------------------------------------
//   y2
//---------------------------------------------------------

double StaffLines::y2() const
      {
      double _spatium = spatium();

      double y = measure()->system()->staff(staffIdx())->y();
      switch(lines()) {
            case 1:
                  return y + _pos.y() + 3 * _spatium;
            case 2:
                  return y + _pos.y() + 3 * _spatium;
            case 3:
            default:
                  return y + _pos.y() + 4 * _spatium;
            }
      }

//---------------------------------------------------------
//   StaffLines::write
//---------------------------------------------------------

void StaffLines::write(Xml& xml) const
      {
      xml.stag("Staff");
      if (lines() != 5)
            xml.tag("lines", lines());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   StaffLines::read
//---------------------------------------------------------

void StaffLines::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "lines")
                  setLines(e.text().toInt());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }


//---------------------------------------------------------
//   Line
//---------------------------------------------------------

Line::Line(Score* s, bool v)
   : Element(s)
      {
      vertical = v;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Line::dump() const
      {
      printf("  width:%g height:%g vert:%d\n", point(_width), point(_len), vertical);
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Line::setLen(Spatium l)
      {
      _len = l;
      }

//---------------------------------------------------------
//   setLineWidth
//---------------------------------------------------------

void Line::setLineWidth(Spatium w)
      {
      _width = w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Line::layout()
      {
      double sp = spatium();
      double w  = _width.val() * sp;
      double l  = _len.val() * sp;
      double w2 = w * .5;
      if (vertical)
            setbbox(QRectF(-w2, -w2, w, l + w));
      else
            setbbox(QRectF(-w2, -w2, l + w, w));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Line::draw(QPainter& p) const
      {
      double sp = spatium();
      QPen pen(p.pen());
      pen.setWidthF(_width.val() * sp);
      p.setPen(pen);
      double l  = _len.val() * sp;
      if (vertical)
            p.drawLine(QLineF(0.0, 0.0, 0.0, l));
      else
            p.drawLine(QLineF(0.0, 0.0, l, 0.0));
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Line::writeProperties(Xml& xml) const
      {
      xml.tag("lineWidth", _width.val());
      xml.tag("lineLen", _len.val());
      if (!vertical)
            xml.tag("vertical", vertical);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Line::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "lineWidth")
            _width = Spatium(val.toDouble());
      else if (tag == "lineLen")
            _len = Spatium(val.toDouble());
      else if (tag == "vertical")
            vertical = val.toInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   Compound
//---------------------------------------------------------

Compound::Compound(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Compound::draw(QPainter& p) const
      {
      foreach(Element* e, elemente) {
            QPointF pt(e->pos());
            p.translate(pt);
            e->draw(p);
            p.translate(-pt);
            }
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 offset \a x and \a y are in Point units
*/

void Compound::addElement(Element* e, double x, double y)
      {
      e->setPos(x, y);
      e->setParent(this);
      elemente.push_back(e);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Compound::bbox() const
      {
      _bbox = QRectF();
      for (ciElement i = elemente.begin(); i != elemente.end(); ++i) {
            const Element* e = *i;
            _bbox |= e->bbox().translated(e->pos());
            }
      return _bbox;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Compound::setSelected(bool f)
      {
      Element::setSelected(f);
      for (ciElement i = elemente.begin(); i != elemente.end(); ++i)
            (*i)->setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Compound::setVisible(bool f)
      {
      Element::setVisible(f);
      for (ciElement i = elemente.begin(); i != elemente.end(); ++i)
            (*i)->setVisible(f);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Compound::clear()
      {
      foreach(Element* e, elemente) {
            if (e->selected())
                  score()->deselect(e);
            delete e;
            }
      elemente.clear();
      }

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s, ScoreView* v)
   : Element(s)
      {
      viewer    = v;
      _on       = false;
      _blink    = true;
      _h        = 6 * spatium();
      _seg      = 0;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Cursor::bbox() const
      {
      double w  = 3.0 / viewer->matrix().m11();
      _bbox = QRectF(0.0, 0.0, w, _h);
      return _bbox;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Cursor::draw(QPainter& p) const
      {
      if (!(_on && _blink))
            return;
      double x = pos().x();
      if (_seg)
            x = _seg->canvasPos().x() - spatium();

      int v = track() == -1 ? 0 : voice();
      p.fillRect(QRectF(x, pos().y(), _bbox.width(), _bbox.height()), QBrush(preferences.selectColor[v]));
      }

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

Lasso::Lasso(Score* s)
   : Element(s)
      {
      setVisible(false);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lasso::draw(QPainter& p) const
      {
      p.setBrush(Qt::NoBrush);
      QPen pen(QColor(preferences.selectColor[0]));
      // always 2 pixel width
      qreal w = 2.0 / p.matrix().m11();
      pen.setWidthF(w);
      p.setPen(pen);
      p.drawRect(bbox());
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Element::dump() const
      {
      printf("---Element: %s, pos(%4.2f,%4.2f)\n"
         "   bbox(%g,%g,%g,%g)\n"
         "   abox(%g,%g,%g,%g)\n"
         "  parent: %p\n",
         name(), _pos.x(), _pos.y(),
         _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
         abbox().x(), abbox().y(), abbox().width(), abbox().height(),
         parent());
      }

//---------------------------------------------------------
//   RubberBand::draw
//---------------------------------------------------------

void RubberBand::draw(QPainter& p) const
      {
      if (!showRubberBand)
            return;
      p.setPen(Qt::red);
      p.drawLine(QLineF(_p1, _p2));
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Element::mimeData(const QPointF& dragOffset) const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.stag("Element");
      if (!dragOffset.isNull())
            xml.tag("dragOffset", dragOffset);
      write(xml);
      xml.etag();
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   readType
//    return -1 if no valid type found
//---------------------------------------------------------

ElementType Element::readType(QDomElement& e, QPointF* dragOffset)
      {
      ElementType type = INVALID;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "dragOffset")
                  *dragOffset = readPoint(e);
            else if ((type = name2type(e.tagName())) == INVALID) {
                  domError(e);
                  break;
                  }
            if (type != INVALID)
                  break;
            }
      return type;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Element::editDrag(int, const QPointF& delta)
      {
      setUserOff(userOff() + delta);
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Element::edit(ScoreView*, int, int key, Qt::KeyboardModifiers, const QString&)
      {
      if (key ==  Qt::Key_Home) {
            setUserOff(QPoint());
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Element::add(Element* e)
      {
      printf("cannot add %s to %s\n", e->name(), name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Element::remove(Element* e)
      {
      printf("cannot remove %s from %s\n", e->name(), name());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Icon::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.tag("action", _action->data().toString());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Icon::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "action") {
                  _action = getAction(qPrintable(e.text()));
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   create
//    Element factory
//---------------------------------------------------------

Element* Element::create(ElementType type, Score* score)
      {
      switch(type) {
            case VOLTA:             return new Volta(score);
            case OTTAVA:            return new Ottava(score);
            case TEXTLINE:          return new TextLine(score);
            case TRILL:             return new Trill(score);
            case PEDAL:             return new Pedal(score);
            case HAIRPIN:           return new Hairpin(score);
            case CLEF:              return new Clef(score);
            case KEYSIG:            return new KeySig(score);
            case TIMESIG:           return new TimeSig(score);
            case BAR_LINE:          return new BarLine(score);
            case ARPEGGIO:          return new Arpeggio(score);
            case BREATH:            return new Breath(score);
            case GLISSANDO:         return new Glissando(score);
            case BRACKET:           return new Bracket(score);
            case ARTICULATION:      return new Articulation(score);
            case ACCIDENTAL:        return new Accidental(score);
            case DYNAMIC:           return new Dynamic(score);
            case TEXT:              return new Text(score);
            case STAFF_TEXT:        return new StaffText(score);
            case NOTEHEAD:          return new NoteHead(score);
            case TREMOLO:           return new Tremolo(score);
            case LAYOUT_BREAK:      return new LayoutBreak(score);
            case MARKER:            return new Marker(score);
            case JUMP:              return new Jump(score);
            case REPEAT_MEASURE:    return new RepeatMeasure(score);
            case ICON:              return new Icon(score);
            case NOTE:              return new Note(score);
            case SYMBOL:            return new Symbol(score);
            case CHORD:             return new Chord(score);
            case REST:              return new Rest(score);
            case SPACER:            return new Spacer(score);
            case TEMPO_TEXT:        return new TempoText(score);
            case HARMONY:           return new Harmony(score);
            case LYRICS:            return new Lyrics(score);
            case STEM:              return new Stem(score);
            case SLUR:              return new Slur(score);
            case ACCIDENTAL_BRACKET: return new AccidentalBracket(score);

            case SLUR_SEGMENT:
            case STEM_SLASH:
            case LINE:
            case IMAGE:
            case TIE:
            case PAGE:
            case BEAM:
            case HOOK:
            case TUPLET:
            case HAIRPIN_SEGMENT:
            case OTTAVA_SEGMENT:
            case PEDAL_SEGMENT:
            case TRILL_SEGMENT:
            case TEXTLINE_SEGMENT:
            case VOLTA_SEGMENT:
            case LEDGER_LINE:
            case MEASURE:
            case STAFF_LINES:
            case CURSOR:
            case SELECTION:
            case LASSO:
            case SHADOW_NOTE:
            case RUBBERBAND:
            case SEGMENT:
            case SYSTEM:
            case COMPOUND:
            case ELEMENT:
            case ELEMENT_LIST:
            case STAFF_LIST:
            case MEASURE_LIST:
            case LAYOUT:
            case HBOX:
            case VBOX:
            case MAXTYPE:
            case INVALID:  break;
            }
      printf("cannot create type <%s>\n", Element::name(type));
      return 0;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* Element::name(ElementType type)
      {
      switch(type) {
            case SYMBOL:            return "Symbol";
            case TEXT:              return "Text";
            case SLUR_SEGMENT:      return "SlurSegment";
            case BAR_LINE:          return "BarLine";
            case STEM_SLASH:        return "StemSlash";
            case LINE:              return "Line";
            case BRACKET:           return "Bracket";
            case ARPEGGIO:          return "Arpeggio";
            case ACCIDENTAL:        return "Accidental";
            case NOTE:              return "Note";
            case STEM:              return "Stem";
            case CLEF:              return "Clef";
            case KEYSIG:            return "KeySig";
            case TIMESIG:           return "TimeSig";
            case REST:              return "Rest";
            case BREATH:            return "Breath";
            case GLISSANDO:         return "Glissando";
            case REPEAT_MEASURE:    return "RepeatMeasure";
            case IMAGE:             return "Image";
            case TIE:               return "Tie";
            case ARTICULATION:      return "Articulation";
            case DYNAMIC:           return "Dynamic";
            case PAGE:              return "Page";
            case BEAM:              return "Beam";
            case HOOK:              return "Hook";
            case LYRICS:            return "Lyrics";
            case MARKER:            return "Marker";
            case JUMP:              return "Jump";
            case TUPLET:            return "Tuplet";
            case TEMPO_TEXT:        return "Tempo";
            case STAFF_TEXT:        return "StaffText";
            case HARMONY:           return "Harmony";
            case VOLTA:             return "Volta";
            case HAIRPIN_SEGMENT:   return "HairpinSegment";
            case OTTAVA_SEGMENT:    return "OttavaSegment";
            case PEDAL_SEGMENT:     return "PedalSegment";
            case TRILL_SEGMENT:     return "TrillSegment";
            case TEXTLINE_SEGMENT:  return "TextLineSegment";
            case VOLTA_SEGMENT:     return "VoltaSegment";
            case LAYOUT_BREAK:      return "LayoutBreak";
            case SPACER:            return "Spacer";
            case LEDGER_LINE:       return "LedgerLine";
            case NOTEHEAD:          return "NoteHead";
            case TREMOLO:           return "Tremolo";
            case MEASURE:           return "Measure";
            case STAFF_LINES:       return "StaffLines";
            case CURSOR:            return "Cursor";
            case SELECTION:         return "Selection";
            case LASSO:             return "Lasso";
            case SHADOW_NOTE:       return "ShadowNote";
            case RUBBERBAND:        return "RubberBand";
            case HAIRPIN:           return "HairPin";
            case OTTAVA:            return "Ottava";
            case PEDAL:             return "Pedal";
            case TRILL:             return "Trill";
            case TEXTLINE:          return "TextLine";
            case SEGMENT:           return "Segment";
            case SYSTEM:            return "System";
            case COMPOUND:          return "Compound";
            case CHORD:             return "Chord";
            case SLUR:              return "Slur";
            case ELEMENT:           return "Element";
            case ELEMENT_LIST:      return "ElementList";
            case STAFF_LIST:        return "StaffList";
            case MEASURE_LIST:      return "MeasureList";
            case LAYOUT:            return "Layout";
            case HBOX:              return "HBox";
            case VBOX:              return "VBox";
            case ICON:              return "Icon";
            case ACCIDENTAL_BRACKET: return "AccidentalBracket";
            case INVALID:
            case MAXTYPE:
                  break;
            }
      return "??";
      }

//---------------------------------------------------------
//   name2type
//---------------------------------------------------------

ElementType Element::name2type(const QString& s)
      {
      for (int i = 0; i < MAXTYPE; ++i) {
            if (s == elementNames[i])
                  return ElementType(i);
            }
      return INVALID;
      }

//---------------------------------------------------------
//   name2Element
//---------------------------------------------------------

Element* Element::name2Element(const QString& s, Score* sc)
      {
      ElementType type = Element::name2type(s);
      if (type == INVALID)
            return 0;
      return Element::create(type, sc);
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void collectElements(void* data, Element* e)
      {
      QList<Element*>* el = static_cast<QList<Element*>*>(data);
      el->append(e);
      }

