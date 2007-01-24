//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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

/**
 \file
 Implementation of Element, ElementList, SStaff.
*/

#include "element.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "preferences.h"
#include "staff.h"
#include "utils.h"
#include "painter.h"
#include "sym.h"
#include "symbol.h"
#include "clef.h"

extern bool debugMode;
extern bool showInvisible;

// for debugging:
const char* elementNames[] = {
      "Symbol", "Text", "Staff", "SlurSegment", "Note", "BarLine",
      "Stem", "Compound", "Line", "SystemBracket",
      "Accidental",
      "Cursor", "Selection", "Lasso", "Clef", "KeySig", "TimeSig", "Chord", "Rest",
      "Tie", "Slur", "Measure",
      "Attribute", "Dynamic", "Page", "Beam", "Flag", "Lyrics",
      "InstrumentLong", "InstrumentShort", "System",
      "HairPin", "Tuplet", "RubberBand", "VSpacer",
      "Segment", "TempoText", "ShadowNote", "Volta", "Ottava",
      "Pedal", "Trill", "LayoutBreak", "HelpLine"
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

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Element::init()
      {
      _prev       = 0;
      _next       = 0;
      _parent     = 0;
      _anchor     = 0;
      _selected   = false;
      _dropTarget = false;
      _visible    = true;
      _generated  = false;
      _voice      = 0;
      _staff      = 0;
      _color      = Qt::black;
      _mxmlOff    = 0;
      _pos.setX(0.0);
      _pos.setY(0.0);
      _userOff.setX(0.0);
      _userOff.setY(0.0);
      }

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

Element::Element(Score* s)
      {
      _score = s;
      init();
      setSubtype(0);
      }

//---------------------------------------------------------
//   staffIdx
//---------------------------------------------------------

int Element::staffIdx() const
      {
      if (_staff)
            return _staff->idx();
      return -1;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

/**
 Return update Rect relative to canvas.
*/

QRectF Element::drag(const QPointF& s)
      {
      QRectF r(abbox());
      setUserOff(s / _spatium);
      return abbox() | r;
      }

//---------------------------------------------------------
//   aref
//---------------------------------------------------------

QPointF Element::aref() const
      {
      QPointF p(pos());     // war _pos
      for (Element* e = _parent; e; e = e->parent())
            p += e->pos();
      return p;
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

/**
 Return true if \a p is inside the shape of the object.

 Note: \a p is relative to the coordinate system of parent().
*/

bool Element::contains(const QPointF& p) const
      {
      return shape().contains(p - pos());
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
  accurate shape for non-rectangular items.
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
//   writeProperties
//---------------------------------------------------------

void Element::writeProperties(Xml& xml) const
      {
      if (_subtype)
            xml.tag("subtype", subtypeName());
      if (!_userOff.isNull())
            xml.tag("offset", _userOff);
      if (voice())
            xml.tag("voice", voice());
      if (selected())
            xml.tag("selected", selected());
      if (!visible())
            xml.tag("visible", visible());
      if (_time.tick() && (_time.tick() != xml.curTick))
            xml.tag("tick", _time.tick());
      if (_duration.tick())
            xml.tag("ticklen", _duration.tick());
      if (_color != Qt::black)
            xml.tagE("color r=\"%d\" g=\"%d\" b=\"%d\"",
               _color.red(), _color.green(), _color.blue());
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(QDomNode node)
      {
      QDomElement e = node.toElement();
      if (e.isNull())
            return true;
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "tick")
            _time.setTick(score()->fileDivision(i));
      else if (tag == "subtype") {
            // do not always call Element::setSubtype():
            this->setSubtype(val);
            }
      else if (tag == "ticklen")
            setTickLen(score()->fileDivision(i));
      else if (tag == "offset")
            setUserOff(readPoint(node));
      else if (tag == "visible")
            setVisible(i);
      else if (tag == "voice")
            setVoice(i);
      else if (tag == "selected")
            setSelected(i);
      else if (tag == "color") {
            int r = e.attribute("r", "0").toInt();
            int g = e.attribute("g", "0").toInt();
            int b = e.attribute("b", "0").toInt();
            _color.setRgb(r, g, b);
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
      xml.etag(name());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Element::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!Element::readProperties(node))
                  domError(node);
            }
      }

//---------------------------------------------------------
//   Element::draw
//---------------------------------------------------------

void Element::draw(Painter& p)
      {
      if (!visible() && (p.print() || !(_score && _score->showInvisible())))
            return;

      QRect r(bbox().translated(pos()).toRect());
      if (!p.clipRect().intersects(r))
            return;
      p.translate(pos());
      QColor c(visible() ? _color : Qt::gray);
      if (!p.print()) {
            if (selected())
                  c = preferences.selectColor[voice()];
            else if (dropTarget())
                  c = preferences.dropColor;
            }
      p.setPen(QPen(c));
      draw1(p);
      if (debugMode && selected()) {
            //
            //  draw bounding box rectangle for all
            //  selected Elements
            //
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
            p.drawRect(bbox());
            }
      p.translate(-pos());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ElementList::draw(Painter& p)
      {
      for (ciElement i = begin(); i != end(); ++i)
            (*i)->draw(p);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

/**
 Remove \a el from the list. Return true on success.
*/

bool ElementList::remove(Element* el)
      {
      for (iElement i = begin(); i != end(); ++i) {
            if ((*i) == el) {
                  erase(i);
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   replace
//---------------------------------------------------------

void ElementList::replace(Element* o, Element* n)
      {
      for (iElement i = begin(); i != end(); ++i) {
            if ((*i) == o) {
                  iElement in = erase(i);
                  insert(in, n);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void ElementList::move(Element* el, int tick)
      {
      for (iElement i = begin(); i != end(); ++i) {
            if ((*i) == el) {
                  erase(i);
                  el->setTick(tick);
                  add(el);
                  break;
                  }
            }
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
      push_back(e);
      }

//---------------------------------------------------------
//   SStaff
//---------------------------------------------------------

SStaff::SStaff(Score* s)
   : Element(s)
      {
      lines = 5;
      qreal lw = point(::style->staffLineWidth);
      setBboxY(-lw/2);
      setHeight((lines-1) * _spatium + lw);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SStaff::draw1(Painter& p)
      {
      QPointF _pos(0.0, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(point(style->staffLineWidth));
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);

      qreal x1 = _pos.x();
      qreal x2 = x1 + width();
      for (int i = 0; i < lines; ++i) {
            qreal y = _pos.y() + i * _spatium;
            p.drawLine(QLineF(x1, y, x2, y));
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
//   bbox
//---------------------------------------------------------

QRectF Line::bbox() const
      {
      double w = point(_width);
      double l = point(_len);
      if (vertical)
            return QRectF(-w*.5, 0, w, l);
      else
            return QRectF(0, -w*.5, l, w);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Line::draw1(Painter& p)
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

bool Line::readProperties(QDomNode node)
      {
      QDomElement e = node.toElement();
      if (e.isNull())
            return true;
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

void Compound::draw1(Painter& p)
      {
      for (ciSymbol i = elemente.begin(); i != elemente.end(); ++i)
            (*i)->draw(p);
#if 0
      if (debugMode && selected()) {
            //
            //  draw bounding box rectangle for all
            //  selected Elements
            //
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::red, 4, Qt::SolidLine));
            p.drawRect(_bbox);
            }
#endif
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
      _bbox = QRectF(0,0,0,0);
      for (ciSymbol i = elemente.begin(); i != elemente.end(); ++i) {
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
      for (ciSymbol i = elemente.begin(); i != elemente.end(); ++i)
            (*i)->setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Compound::setVisible(bool f)
      {
      Element::setVisible(f);
      for (ciSymbol i = elemente.begin(); i != elemente.end(); ++i)
            (*i)->setVisible(f);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Compound::clear()
      {
      for (ciSymbol i = elemente.begin(); i != elemente.end(); ++i)
            delete *i;
      elemente.clear();
      }

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Score* s)
  : Element(s)
      {
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void KeySig::setSubtype(int st)
      {
      Element::setSubtype(st);
      layout();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void KeySig::addLayout(bool flat, double x, double y)
      {
      _bbox |= symbols[flat ? flatSym : sharpSym].bbox().translated(x*_spatium, y * _spatium);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout()
      {
      double yoff;
      if (staff()) {
            int clef       = staff()->clef()->clef(tick());
            int clefOffset = clefTable[clef].yOffset;
            yoff = double(-((clefOffset % 10) / 2.0));
            }
      else
            yoff = 0.0;

      _bbox = QRectF(0, 0, 0, 0);
      switch(subtype()) {
            case 7:     addLayout(false, 6.0, yoff + 2);
            case 6:     addLayout(false, 5.0, yoff + .5);
            case 5:     addLayout(false, 4.0, yoff + 2.5);
            case 4:     addLayout(false, 3.0, yoff + 1);
            case 3:     addLayout(false, 2.0, yoff - .5);
            case 2:     addLayout(false, 1.0, yoff + 1.5);
            case 1:     addLayout(false, 0.0, yoff);
                        break;
            case -7:    addLayout(true, 6, yoff + 3.5);
            case -6:    addLayout(true, 5, yoff + 1.5);
            case -5:    addLayout(true, 4, yoff + 3);
            case -4:    addLayout(true, 3, yoff + 1);
            case -3:    addLayout(true, 2, yoff + 2.5);
            case -2:    addLayout(true, 1, yoff + .5);
            case -1:    addLayout(true, 0, yoff + 2);
            case 0:
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void KeySig::add(Painter& p, bool flat, double x, double y)
      {
      symbols[flat ? flatSym : sharpSym].draw(p, x * _spatium, y * _spatium);
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void KeySig::draw1(Painter& p)
      {
      double yoff;
      if (staff()) {
            int clef       = staff()->clef()->clef(tick());
            int clefOffset = clefTable[clef].yOffset;
            yoff = double(-((clefOffset % 10) / 2.0));
            }
      else
            yoff = 0.0;

      switch(subtype()) {
            case 7:     add(p, false, 6.0, yoff + 2);
            case 6:     add(p, false, 5.0, yoff + .5);
            case 5:     add(p, false, 4.0, yoff + 2.5);
            case 4:     add(p, false, 3.0, yoff + 1);
            case 3:     add(p, false, 2.0, yoff - .5);
            case 2:     add(p, false, 1.0, yoff + 1.5);
            case 1:     add(p, false, 0.0, yoff);
                        break;
            default:
            case 0:
                  return;

            case -7:    add(p, true, 6, yoff + 3.5);
            case -6:    add(p, true, 5, yoff + 1.5);
            case -5:    add(p, true, 4, yoff + 3);
            case -4:    add(p, true, 3, yoff + 1);
            case -3:    add(p, true, 2, yoff + 2.5);
            case -2:    add(p, true, 1, yoff + .5);
            case -1:    add(p, true, 0, yoff + 2);
                  break;
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(const QPointF&, int type, const QDomNode&) const
      {
      return type == KEYSIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void KeySig::drop(const QPointF&, int type, const QDomNode& node)
      {
      if (type == KEYSIG) {
            KeySig* k = new KeySig(0);
            k->read(node);
            int stype = k->subtype();
            delete k;
            int st = subtype();
            if (st == stype)
                  return;
            // change keysig applies to all staves, can't simply set subtype
            // for this one only
            score()->changeKeySig(tick(), stype);
            }
      }

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s, double l)
   : Element(s)
      {
      dlen      = l;
      lineWidth = .4;
      _on       = false;
      _blink    = true;
      double w  = lineWidth * _spatium;
      setbbox(QRectF(-w/2, 0, lineWidth * _spatium, dlen * _spatium * 1.1));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Cursor::draw1(Painter& p)
      {
      if (!(_on && _blink))
            return;

      QPen pen(preferences.selectColor[voice()]);
      pen.setWidthF(2.0 * p.matrix().m11());
      p.setPen(pen);

      p.drawLine(QLineF(0.0, 0.0, 0.0, dlen * _spatium));
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

void Lasso::draw1(Painter& p)
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
      printf("---Element type %s, pos(%4.2f,%4.2f)\n"
         "   bbox(%g,%g,%g,%g)\n"
         "   abox(%g,%g,%g,%g)\n"
         "  parent: %p\n",
         elementNames[type()], _pos.x(), _pos.y(),
         _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
         abbox().x(), abbox().y(), abbox().width(), abbox().height(),
         parent());
      }

//---------------------------------------------------------
//   RubberBand::draw
//---------------------------------------------------------

void RubberBand::draw(Painter& p)
      {
      if (!showRubberBand)
            return;
      p.setPen(Qt::red);
      p.drawLine(QLineF(_p1, _p2));
      }

//---------------------------------------------------------
//   VSpacer
//---------------------------------------------------------

VSpacer::VSpacer(Score* s, double h)
   : Element(s)
      {
      height = h;
      double w = .5 * _spatium;
      setbbox(QRectF(-w, 0, 2*w, height));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void VSpacer::draw1(Painter&)
      {
//      int lw       = lrint(.5 * tf->mag() * _spatium);
//      int len      = lrint(height * tf->mag() * _spatium);
//      QPoint _pos  = tf->fpos2ipoint(QPointF(0, 0));

//      p.setPen(QPen(QColor(Qt::blue), lw));
//TODO      p.drawLine(_pos.x(), _pos.y(), _pos.x(), _pos.y() + height);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Element::space(double& min, double& extra) const
      {
      min   = 0.0;
      extra = width();
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Element::mimeData() const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      write(xml);
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Volta::draw1(Painter& p)
      {
      qreal voltaLineWidth = _spatium * .18;
      qreal h              = _spatium * 1.8;

      QPointF p0(_p1.x(), h);
      QPointF p3(_p2.x(), h);

      QPen pen(p.pen());
      pen.setWidthF(voltaLineWidth);
      p.setPen(pen);
      p.drawLine(QLineF(p0, _p1));
      p.drawLine(QLineF(_p1, _p2));
      if (subtype() != 4)
            p.drawLine(QLineF(_p2, p3));

      TextStyle* s = &textStyles[TEXT_STYLE_SYMBOL1];
      QFont f(s->family, s->size);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      p.setFont(f);

      QPointF tp(p0.x() + _spatium * .5, p0.y());

      switch(subtype()) {
            default:
            case PRIMA_VOLTA:
                  p.drawText(tp, "1.");
                  break;
            case SECONDA_VOLTA:
            case SECONDA_VOLTA2:
                  p.drawText(tp, "2.");
                  break;
            case TERZA_VOLTA:
                  p.drawText(tp, "3.");
                  break;
            }
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Volta::setLen(qreal l)
      {
      qreal voltaHeight = _spatium * 1.8;
      _p2.setX(l);
      setbbox(QRectF(0.0, 0.0, _p2.x() - _p1.x(), voltaHeight));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Volta::layout()
      {
      if (!parent())
            return;
      qreal voltaHeight   = _spatium * 1.8;
      qreal voltaDistance = _spatium * .7;

      Measure* measure = (Measure*)parent();
      System* system   = measure->system();
      SysStaff* sstaff = system->staff(staffIdx());
      qreal y  = sstaff->bbox().top();
      qreal x2 = measure->width() - _spatium * .5;

      _p1.setX(0.0);
      _p1.setY(0.0);
      _p2.setX(x2);
      _p2.setY(0.0);

      setbbox(QRectF(0.0, 0.0, _p2.x() - _p1.x(), voltaHeight));
      setPos(0.0, y - (voltaHeight + voltaDistance));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      xml.stag("Volta");
      Element::writeProperties(xml);
      xml.etag("Volta");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!Element::readProperties(node))
                  domError(node);
            }
      }

