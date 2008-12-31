//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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
#include "layout.h"
#include "viewer.h"
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
#include "canvas.h"
#include "glissando.h"
#include "articulation.h"
#include "chord.h"
#include "spacer.h"
#include "mscore.h"
#include "tempotext.h"
#include "harmony.h"
#include "lyrics.h"
#include "rest.h"

extern bool debugMode;
extern bool showInvisible;

const char* elementNames[] = {
      "Symbol", "Text", "SlurSegment", "BarLine",
      "StemSlash", "Line", "Bracket",
      "Arpeggio",
      "Accidental",
      "Note",
      "Stem",
      "Clef", "KeySig", "TimeSig", "Rest",
      "Breath", "Glissando",
      "RepeatMeasure",
      "Image",
      "Tie",
      "Attribute", "Dynamic", "Page", "Beam", "Hook", "Lyrics", "Marker", "Jump",
      "Tuplet",
      "Tempo", "StaffText",
      "Harmony",
      "Volta",
      "HairpinSegment", "OttavaSegment", "PedalSegment", "TrillSegment", "TextLineSegment",
      "VoltaSegment",
      "LayoutBreak", "Spacer",
      "LedgerLine",
      "NoteHead", "Tremolo",
      "Measure", "StaffLines",
      "Cursor", "Selection", "Lasso", "ShadowNote", "RubberBand",
      "HairPin", "Ottava", "Pedal", "Trill", "TextLine",
      "Segment", "System", "Compound", "Chord", "Slur",
      "Element", "ElementList", "StaffList", "MeasureList", "Layout",
      "HBox", "VBox",
      "Icon"
      };

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
      if (score()) {
            Selection* s = score()->selection();
            QList<Element*>* el = s->elements();
            foreach(Element* e, *el) {
                  if (e == this) {
                        if (debugMode)
                              printf("======~Element: %p still in selection!\n", this);
                        el->removeAt(el->indexOf(this));
                        }
                  }
            UndoList* ul = score()->getUndoList();
            if (!ul->isEmpty()) {
                  Undo* undo = ul->back();
                  s = &undo->selection;
                  QList<Element*>* el = s->elements();
                  foreach(Element* e, *el) {
                        if (e == this) {
                              if (debugMode)
                                    printf("======~Element: %p still in undo!\n", this);
                              el->removeAt(el->indexOf(this));
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Element::init()
      {
      _tick       = -1;
      _duration   = -1;
      _parent     = 0;
      _selected   = false;
      _selectable = true;
      _dropTarget = false;
      _visible    = true;
      _generated  = false;
      _track      = -1;
      _color      = preferences.defaultColor;
      _mxmlOff    = 0;
      _pos.setX(0.0);
      _pos.setY(0.0);
      _userOff.setX(0.0);
      _userOff.setY(0.0);
      itemDiscovered = 0;
      _align       = ALIGN_LEFT | ALIGN_TOP;
      _xoff        = 0;
      _yoff        = 0;
      _rxoff       = 0;
      _ryoff       = 0;
      _offsetType  = OFFSET_SPATIUM;
      _systemFlag  = false;
      }

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

Element::Element(Score* s)
      {
      _score = s;
      init();
      setSubtype(0);
      _mag = 1.0;
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
      _rxoff      = e._rxoff;
      _ryoff      = e._ryoff;
      _offsetType = e._offsetType;
      _userOff    = e._userOff;
      _mxmlOff    = e._mxmlOff;
      _bbox       = e._bbox;
      _duration   = e._duration;
      itemDiscovered = 0;
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
      setUserOff(pos / _spatium);
      return abbox() | r;
      }

//---------------------------------------------------------
//   canvasPos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF Element::canvasPos() const
      {
      QPointF p(pos());
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

void Element::layout(ScoreLayout* layout)
      {
      QPointF o(QPointF(_xoff, _yoff));
      if (_offsetType == OFFSET_SPATIUM)
            if (layout)
                  o *= layout->spatium();
            else
                  o *= _spatium;
      else
            o *= DPI;
      if (parent())
            o += QPointF(_rxoff * parent()->width() * 0.01, _ryoff * parent()->height() * 0.01);
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

QList<Prop> Element::properties(Xml& xml) const
      {
      QList<Prop> pl;
      if (_subtype) {
            QString s(subtypeName());
            if (!s.isEmpty())
                  pl.append(Prop("subtype", subtypeName()));
            }
      if (!_userOff.isNull())
            pl.append(Prop("offset", _userOff));
      if (track() != xml.curTrack) {
            int t;
            if (track() == -1)
                  t = -1;
            else
                  t = track() + xml.trackDiff;
            pl.append(Prop("track", t));
            }
      if (selected())
            pl.append(Prop("selected", selected()));
      if (!visible())
            pl.append(Prop("visible", visible()));
      if (_tick != -1 && (_tick != xml.curTick))
            pl.append(Prop("tick", _tick));
      if (_duration != -1)
            pl.append(Prop("ticklen", _duration));
      if (_color != preferences.defaultColor)
            pl.append(Prop("color", _color));
      if (_systemFlag)
            pl.append(Prop("systemFlag", _systemFlag));
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Element::writeProperties(Xml& xml) const
      {
      xml.prop(properties(xml));
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
//      setTrack(score()->curTrack);

      if (tag == "tick") {
            setTick(score()->fileDivision(i));
            score()->curTick = _tick;
            }
      else if (tag == "subtype") {
            // do not always call Element::setSubtype():
            this->setSubtype(val);
            }
      else if (tag == "ticklen")
            setTickLen(score()->fileDivision(i));
      else if (tag == "offset")
            setUserOff(readPoint(e));
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

void Element::propertyAction(const QString& s)
      {
      if (s == "invisible")
            score()->toggleInvisible(this);
      else if (s == "color") {
            score()->colorItem(this);
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
      int l    = lines() - 1;
      qreal lw = point(score()->style()->staffLineWidth);

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

//      p.save();
//      p.setRenderHint(QPainter::Antialiasing, false);
//      p.setRenderHint(QPainter::NonCosmeticDefaultPen, true);

      QPen pen(p.pen());
      pen.setWidthF(point(score()->style()->staffLineWidth) * mag());
      if (pen.widthF() * p.worldMatrix().m11() < 1.0)
            pen.setWidth(0);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);

      qreal x1 = _pos.x();
      qreal x2 = x1 + width();

      switch(lines()) {
            case 1:
                  {
                  qreal y = _pos.y() + 2 * _spatium * mag();
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
                  break;
            case 2:
                  {
                  qreal y = _pos.y() + 1 * _spatium * mag();
                  p.drawLine(QLineF(x1, y, x2, y));
                  y += 2 * _spatium * mag();
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
                  break;
            case 3:
                  for (int i = 0; i < lines(); ++i) {
                        qreal y = _pos.y() + i * _spatium * mag() * 2.0;
                        p.drawLine(QLineF(x1, y, x2, y));
                        }
                  break;

            default:
                  for (int i = 0; i < lines(); ++i) {
                        qreal y = _pos.y() + i * _spatium * mag();
                        p.drawLine(QLineF(x1, y, x2, y));
                        }
                  break;
            }
//      p.restore();
      }

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

double StaffLines::y1() const
      {
      double y = measure()->system()->staff(staffIdx())->y();
      switch(lines()) {
            case 1:
                  return y + _pos.y() + 1 * _spatium * mag();
            case 2:
                  return y + _pos.y() + 1 * _spatium * mag();
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
      double y = measure()->system()->staff(staffIdx())->y();
      switch(lines()) {
            case 1:
                  return y + _pos.y() + 3 * _spatium * mag();
            case 2:
                  return y + _pos.y() + 3 * _spatium * mag();
            case 3:
            default:
                  return y + _pos.y() + 4 * _spatium * mag();
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
      printf("  width:%g height:%g vert:%d\n",
         point(_width), point(_len), vertical);
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

void Line::layout(ScoreLayout* layout)
      {
      double spatium = layout->spatium();
      double w = _width.val() * spatium;
      double l = _len.val() * spatium;
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
      QPen pen(p.pen());
      pen.setWidthF(point(_width));
      p.setPen(pen);
      if (vertical)
            p.drawLine(QLineF(0.0, 0.0, 0.0, point(_len)));
      else
            p.drawLine(QLineF(0.0, 0.0, point(_len), 0.0));
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
      for (ciElement i = elemente.begin(); i != elemente.end(); ++i)
            delete *i;
      elemente.clear();
      }

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s, Viewer* v)
   : Element(s)
      {
      viewer    = v;
      _on       = false;
      _blink    = true;
      _h        = 6 * _spatium;
      _seg      = 0;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Cursor::bbox() const
      {
      double w  = 2.0 / static_cast<Viewer*>(score()->canvas())->matrix().m11();
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
            x = _seg->canvasPos().x() - _spatium;

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
//   name2type
//---------------------------------------------------------

int Element::name2type(const QString& s)
      {
      int n = sizeof(elementNames)/sizeof(*elementNames);
      for (int i = 0; i < n; ++i) {
            if (s == elementNames[i])
                  return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   name2Element
//---------------------------------------------------------

Element* Element::name2Element(const QString& s, Score* sc)
      {
      int type = Element::name2type(s);
      if (type == -1)
            return 0;
      return Element::create(type, sc);
      }

//---------------------------------------------------------
//   readType
//    return -1 if no valid type found
//---------------------------------------------------------

int Element::readType(QDomElement& e, QPointF* dragOffset)
      {
      int type = -1;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "dragOffset")
                  *dragOffset = readPoint(e);
            else if ((type = name2type(e.tagName())) == -1) {
                  domError(e);
                  break;
                  }
            if (type >= 0)
                  break;
            }
      return type;
      }

//---------------------------------------------------------
//   create
//    Element factory
//---------------------------------------------------------

Element* Element::create(int type, Score* score)
      {
      Element* el = 0;
      switch(type) {
            case VOLTA:
                  el = new Volta(score);
                  break;
            case OTTAVA:
                  el = new Ottava(score);
                  break;
            case TEXTLINE:
                  el = new TextLine(score);
                  break;
            case TRILL:
                  el = new Trill(score);
                  break;
            case PEDAL:
                  el = new Pedal(score);
                  break;
            case HAIRPIN:
                  el = new Hairpin(score);
                  break;
            case CLEF:
                  el = new Clef(score);
                  break;
            case KEYSIG:
                  el = new KeySig(score);
                  break;
            case TIMESIG:
                  el = new TimeSig(score);
                  break;
            case BAR_LINE:
                  el = new BarLine(score);
                  break;
            case ARPEGGIO:
                  el = new Arpeggio(score);
                  break;
            case BREATH:
                  el = new Breath(score);
                  break;
            case GLISSANDO:
                  el = new Glissando(score);
                  break;
            case BRACKET:
                  el = new Bracket(score);
                  break;
            case ATTRIBUTE:
                  el = new Articulation(score);
                  break;
            case ACCIDENTAL:
                  el = new Accidental(score);
                  break;
            case DYNAMIC:
                  el = new Dynamic(score);
                  break;
            case TEXT:
                  el = new Text(score);
                  break;
            case STAFF_TEXT:
                  el = new StaffText(score);
                  break;
            case NOTEHEAD:
                  el = new NoteHead(score);
                  break;
            case TREMOLO:
                  el = new Tremolo(score);
                  break;
            case LAYOUT_BREAK:
                  el = new LayoutBreak(score);
                  break;
            case MARKER:
                  el = new Marker(score);
                  break;
            case JUMP:
                  el = new Jump(score);
                  break;
            case REPEAT_MEASURE:
                  el = new RepeatMeasure(score);
                  break;
            case ICON:
                  el = new Icon(score);
                  break;
            case NOTE:
                  el = new Note(score);
                  break;
            case SYMBOL:
                  el = new Symbol(score);
                  break;
            case CHORD:
                  el = new Chord(score);
                  break;
            case REST:
                  el = new Rest(score);
                  break;
            case SPACER:
                  el = new Spacer(score);
                  break;
            case TEMPO_TEXT:
                  el = new TempoText(score);
                  break;
            case HARMONY:
                  el = new Harmony(score);
                  break;
            case LYRICS:
                  el = new Lyrics(score);
                  break;
            default:
                  printf("Element::create(): cannot create element type %s\n",
                     elementNames[type]);
                  break;
            }
      return el;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Element::startEdit(Viewer*, const QPointF&)
      {
      return !_generated;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Element::editDrag(int, const QPointF& delta)
      {
      setUserOff(userOff() + delta / _spatium);
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Element::edit(Viewer*, int, int key, Qt::KeyboardModifiers, const QString&)
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


