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

extern bool debugMode;
extern bool showInvisible;

// for debugging:
const char* elementNames[] = {
      "Symbol", "Text", "Staff", "SlurSegment", "Note", "BarLine",
      "Stem", "Compound", "Line", "SystemBracket",
      "Accidental",
      "Cursor", "Selection", "Lasso", "Clef", "Keysig", "Timesig", "Chord", "Rest",
      "Tie", "Slur", "Measure",
      "Attribute", "Dynamic", "Page", "Beam", "Flag", "Lyrics",
      "InstrumentLong", "InstrumentShort", "Fingering", "System",
      "Hairpin", "Tuplet", "RubberBand", "VSpacer",
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
   : _bbox(0.0, 0.0, 0.0, 0.0)
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
//    return update Rect relative to canvas
//---------------------------------------------------------

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
//    return true if p is inside of bounding box of object
//    p is relative to the coordinate system of parent()
//---------------------------------------------------------

bool Element::contains(const QPointF& p) const
      {
      return bbox().contains(p - pos());
      }

//---------------------------------------------------------
//   intersects
//    return true if r intersects bounding box of object
//    r is relative to the coordinate system of parent()
//---------------------------------------------------------

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
            xml.tag("subtype", _subtype);
      if (!_userOff.isNull())
            xml.tag("offset", _userOff);
      if (voice())
            xml.tag("voice", voice());
      if (selected())
            xml.tag("selected", selected());
      if (!visible())
            xml.tag("visible", visible());
      if (_time.tick())
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
            _time.setTick(i);
      else if (tag == "subtype")
            setSubtype(i);
      else if (tag == "ticklen")
            setTickLen(i);
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
//    return true on success
//---------------------------------------------------------

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
//   Accidental
//    0 - no accidental
//    1 - sharp
//    2 - flat
//    3 - double sharp
//    4 - double flat
//    5 - natural
//    6 - (sharp)          11 - [sharp]
//    7 - (flat)           12 - [flat]
//    8 - (double sharp)   13 - [double sharp]
//    9 - (double flat)    14 - [double flat]
//    10 - (natural)       15 - [natural]
//---------------------------------------------------------

Accidental::Accidental(Score* sc, int i, bool s)
  : Compound(sc)
      {
      small = s;
      val   = -1;
      setIdx(i);
      }

//---------------------------------------------------------
//   setIdx
//---------------------------------------------------------

void Accidental::setIdx(int i)
      {
      if (val == i)
            return;
      val = i;
      clear();
      if (small) {
            Symbol* s = new Symbol(score());
            switch(i) {
                  default:
                  case  0: delete s; return;
                  case  1: s->setSym(s_sharpSym);      break;
                  case  2: s->setSym(s_flatSym);       break;
                  case  3: s->setSym(s_sharpsharpSym); break;
                  case  4: s->setSym(s_flatflatSym);   break;
                  case  5: s->setSym(s_naturalSym);    break;

                  case  6: s->setSym(s_sharpSym);      break;
                  case  7: s->setSym(s_flatSym);       break;
                  case  8: s->setSym(s_sharpsharpSym); break;
                  case  9: s->setSym(s_flatflatSym);   break;
                  case 10: s->setSym(s_naturalSym);    break;

                  case 11: s->setSym(s_sharpSym);      break;
                  case 12: s->setSym(s_flatSym);       break;
                  case 13: s->setSym(s_sharpsharpSym); break;
                  case 14: s->setSym(s_flatflatSym);   break;
                  case 15: s->setSym(s_naturalSym);    break;
                  }
            addElement(s, 0.0, 0.0);
            }
      else {
            Symbol* s = new Symbol(score());
            switch(i) {
                  default:
                  case  0: delete s; return;
                  case  1: s->setSym(sharpSym);      break;
                  case  2: s->setSym(flatSym);       break;
                  case  3: s->setSym(sharpsharpSym); break;
                  case  4: s->setSym(flatflatSym);   break;
                  case  5: s->setSym(naturalSym);    break;

                  case  6 ... 10:
                        {
                        s->setSym(leftparenSym);
                        addElement(s, 0.0, 0.0);
                        double x = symbols[leftparenSym].width() / _spatium;

                        s = new Symbol(score());
                        switch(i) {
                              case  6: s->setSym(s_sharpSym);    break;
                              case  7: s->setSym(flatSym);       break;
                              case  8: s->setSym(sharpsharpSym); break;
                              case  9: s->setSym(flatflatSym);   break;
                              case 10: s->setSym(naturalSym);    break;
                              }
                        addElement(s, x, 0.0);
                        x += (s->width() / _spatium);

                        s = new Symbol(score());
                        s->setSym(rightparenSym);
                        addElement(s, x, 0.0);
                        }
                        return;

                  case 11: s->setSym(sharpSym);      break;
                  case 12: s->setSym(flatSym);       break;
                  case 13: s->setSym(sharpsharpSym); break;
                  case 14: s->setSym(flatflatSym);   break;
                  case 15: s->setSym(naturalSym);    break;
                  }
            addElement(s, 0.0, 0.0);
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
      bboxUpdate();
      }

//---------------------------------------------------------
//   setLineWidth
//---------------------------------------------------------

void Line::setLineWidth(Spatium w)
      {
      _width = w;
      bboxUpdate();
      }

//---------------------------------------------------------
//   bboxUpdate
//---------------------------------------------------------

void Line::bboxUpdate()
      {
      double w = point(_width);
      double l = point(_len);
      if (vertical)
            setbbox(QRectF(-w*.5, 0, w, l));
      else
            setbbox(QRectF(0, -w*.5, l, w));
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

      if (!(visible() || _score->showInvisible()) && debugMode && selected()) {
            //
            //  draw bounding box rectangle for all
            //  selected Elements
            //
            p.setPen(QColor(Qt::green));
            p.setBrush(Qt::NoBrush);
            p.drawRect(abbox());
            }
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

void Compound::addElement(Element* e, double x, double y)
      {
      e->setUserOff(QPointF(x, y));
      e->setParent(this);
      elemente.push_back(e);
      orBbox(e->bbox().translated(e->pos()));
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
      setbbox(QRectF());
      }

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Score* s)
  : Compound(s)
      {
      }

KeySig::KeySig(Score* s, int i, int yoffset)
  : Compound(s)
      {
      setIdx(i, yoffset);
      }

//---------------------------------------------------------
//   setIdx
//---------------------------------------------------------

void KeySig::setIdx(int v, int offset)
      {
      clear();
      val = v;
      off = offset % 10;

      Accidental* p;
      double yoff = -(off / 2.0);
      switch(v) {
            case 7:
                  p = new Accidental(score(), 1, false);
                  addElement(p, 6.0, yoff + 2);
            case 6:
                  p = new Accidental(score(), 1, false);
                  addElement(p, 5.0, yoff + .5);
            case 5:
                  p = new Accidental(score(), 1, false);
                  addElement(p, 4.0, yoff + 2.5);
            case 4:
                  p = new Accidental(score(), 1, false);
                  addElement(p, 3.0, yoff + 1);
            case 3:
                  p = new Accidental(score(), 1, false);
                  addElement(p, 2.0, yoff - .5);
            case 2:
                  p = new Accidental(score(), 1, false);
                  addElement(p, 1.0, yoff + 1.5);
            case 1:
                  p = new Accidental(score(), 1, false);
                  addElement(p, 0.0, yoff);
                  break;
            default:
            case 0:
                  return;
            case -7:
                  p = new Accidental(score(), 2, false);
                  addElement(p, 6, yoff + 3.5);
            case -6:
                  p = new Accidental(score(), 2, false);
                  addElement(p, 5, yoff + 1.5);
            case -5:
                  p = new Accidental(score(), 2, false);
                  addElement(p, 4, yoff + 3);
            case -4:
                  p = new Accidental(score(), 2, false);
                  addElement(p, 3, yoff + 1);
            case -3:
                  p = new Accidental(score(), 2, false);
                  addElement(p, 2, yoff + 2.5);
            case -2:
                  p = new Accidental(score(), 2, false);
                  addElement(p, 1, yoff + .5);
            case -1:
                  p = new Accidental(score(), 2, false);
                  addElement(p, 0, yoff + 2);
                  break;
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
      char buffer[32];
      sprintf(buffer, "%d/%d", int(type()), subtype());
      return QByteArray(buffer);
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
      QFont f(s->family);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      f.setPixelSize(lrint(s->size * _spatium / 5.0 * .7));
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
      qreal voltaHeight   = _spatium * 1.8;
      qreal voltaDistance = _spatium * .7;

      Measure* measure = (Measure*)parent();
      System* system   = measure->system();
      SysStaff* sstaff = system->staff(staffIdx());

      _p1.setX(0.0);
      _p1.setY(0.0);
      _p2.setX(measure->width() - _spatium * .5);
      _p2.setY(0.0);

      setbbox(QRectF(0.0, 0.0, _p2.x() - _p1.x(), voltaHeight));
      setPos(0.0, sstaff->bbox().top() - (voltaHeight + voltaDistance));
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

