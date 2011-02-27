//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "fret.h"
#include "staffstate.h"
#include "fingering.h"
#include "bend.h"
#include "tremolobar.h"
#include "chordline.h"
#include "undo.h"
#include "segment.h"
#include "box.h"
#include "instrchange.h"
#include "stafftype.h"
#include "stem.h"
#include "painter.h"
#include "iname.h"

extern bool debugMode;
extern bool showInvisible;

//
// list has to synchronized with ElementType enum
//
static const char* elementNames[] = {
      QT_TRANSLATE_NOOP("elementName", "invalid"),
      QT_TRANSLATE_NOOP("elementName", "Symbol"),
      QT_TRANSLATE_NOOP("elementName", "Text"),
      QT_TRANSLATE_NOOP("elementName", "InstrumentName"),
      QT_TRANSLATE_NOOP("elementName", "SlurSegment"),
      QT_TRANSLATE_NOOP("elementName", "BarLine"),
      QT_TRANSLATE_NOOP("elementName", "StemSlash"),
      QT_TRANSLATE_NOOP("elementName", "Line"),
      QT_TRANSLATE_NOOP("elementName", "Bracket"),
      QT_TRANSLATE_NOOP("elementName", "Arpeggio"),
      QT_TRANSLATE_NOOP("elementName", "Accidental"),
      QT_TRANSLATE_NOOP("elementName", "Note"),
      QT_TRANSLATE_NOOP("elementName", "Stem"),             // 10
      QT_TRANSLATE_NOOP("elementName", "Clef"),
      QT_TRANSLATE_NOOP("elementName", "KeySig"),
      QT_TRANSLATE_NOOP("elementName", "TimeSig"),
      QT_TRANSLATE_NOOP("elementName", "Rest"),
      QT_TRANSLATE_NOOP("elementName", "Breath"),
      QT_TRANSLATE_NOOP("elementName", "Glissando"),
      QT_TRANSLATE_NOOP("elementName", "RepeatMeasure"),
      QT_TRANSLATE_NOOP("elementName", "Image"),
      QT_TRANSLATE_NOOP("elementName", "Tie"),
      QT_TRANSLATE_NOOP("elementName", "Articulation"),     // 20
      QT_TRANSLATE_NOOP("elementName", "ChordLine"),
      QT_TRANSLATE_NOOP("elementName", "Dynamic"),
      QT_TRANSLATE_NOOP("elementName", "Beam"),
      QT_TRANSLATE_NOOP("elementName", "Hook"),
      QT_TRANSLATE_NOOP("elementName", "Lyrics"),
      QT_TRANSLATE_NOOP("elementName", "Marker"),
      QT_TRANSLATE_NOOP("elementName", "Jump"),
      QT_TRANSLATE_NOOP("elementName", "Fingering"),
      QT_TRANSLATE_NOOP("elementName", "Tuplet"),
      QT_TRANSLATE_NOOP("elementName", "Tempo"),
      QT_TRANSLATE_NOOP("elementName", "StaffText"),
      QT_TRANSLATE_NOOP("elementName", "InstrumentChange"),
      QT_TRANSLATE_NOOP("elementName", "Harmony"),
      QT_TRANSLATE_NOOP("elementName", "FretDiagram"),
      QT_TRANSLATE_NOOP("elementName", "Bend"),
      QT_TRANSLATE_NOOP("elementName", "TremoloBar"),
      QT_TRANSLATE_NOOP("elementName", "Volta"),
      QT_TRANSLATE_NOOP("elementName", "HairpinSegment"),
      QT_TRANSLATE_NOOP("elementName", "OttavaSegment"),
      QT_TRANSLATE_NOOP("elementName", "TrillSegment"),
      QT_TRANSLATE_NOOP("elementName", "TextLineSegment"),
      QT_TRANSLATE_NOOP("elementName", "VoltaSegment"),
      QT_TRANSLATE_NOOP("elementName", "LayoutBreak"),
      QT_TRANSLATE_NOOP("elementName", "Spacer"),
      QT_TRANSLATE_NOOP("elementName", "StaffState"),
      QT_TRANSLATE_NOOP("elementName", "LedgerLine"),
      QT_TRANSLATE_NOOP("elementName", "NoteHead"),
      QT_TRANSLATE_NOOP("elementName", "NoteDot"),
      QT_TRANSLATE_NOOP("elementName", "Tremolo"),
      QT_TRANSLATE_NOOP("elementName", "Measure"),
      QT_TRANSLATE_NOOP("elementName", "StaffLines"),
      QT_TRANSLATE_NOOP("elementName", "Cursor"),
      QT_TRANSLATE_NOOP("elementName", "Selection"),
      QT_TRANSLATE_NOOP("elementName", "Lasso"),
      QT_TRANSLATE_NOOP("elementName", "ShadowNote"),
      QT_TRANSLATE_NOOP("elementName", "RubberBand"),
      QT_TRANSLATE_NOOP("elementName", "TabDurationSymbol"),
      QT_TRANSLATE_NOOP("elementName", "FSymbol"),
      QT_TRANSLATE_NOOP("elementName", "Page"),
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
      QT_TRANSLATE_NOOP("elementName", "TBox"),
      QT_TRANSLATE_NOOP("elementName", "FBox"),
      QT_TRANSLATE_NOOP("elementName", "Icon"),
      QT_TRANSLATE_NOOP("elementName", "AccidentalBracket")
      };

int LinkedElements::_linkId = 0;    // highest id in use

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

LinkedElements::LinkedElements()
      {
      _lid = ++_linkId; // create new unique id
      }

LinkedElements::LinkedElements(int id)
      {
      _lid = id;
      if (_linkId <= id)
            _linkId = id;
      }

//---------------------------------------------------------
//   setLid
//---------------------------------------------------------

void LinkedElements::setLid(int id)
      {
      _lid = id;
      if (_linkId <= id)
            _linkId = id;
      }

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

Element::~Element()
      {
      if (_links) {
            _links->removeOne(this);
            if (_links->isEmpty()) {
                  //DEBUG:
                  score()->links().remove(_links->lid());
                  //
                  delete _links;
                  }
            }
      if (score()) {
            foreach(Element* e, score()->selection().elements()) {
                  if (e == this) {
//                        if (debugMode)
                              printf("======~Element: %p still in selection!\n", this);
                        if (debugMode)
                              abort();
                        score()->deselect(this);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

Element::Element(Score* s) :
   _links(0),
   _parent(0),
   _selected(false),
   _generated(false),
   _visible(true),
   _flags(ELEMENT_SELECTABLE),
   _subtype(0),
   _track(-1),
   _color(preferences.defaultColor),
   _mag(1.0),
   _score(s),
   _mxmlOff(0),
   itemDiscovered(0)
      {
      }

Element::Element(const Element& e)
      {
      _links      = 0;
      _parent     = e._parent;
      _selected   = e._selected;
      _generated  = e._generated;
      _visible    = e._visible;
      _flags      = e._flags;
      _subtype    = e._subtype;
      _track      = e._track;
      _color      = e._color;
      _mag        = e._mag;
      _pos        = e._pos;
      _userOff    = e._userOff;
      _readPos    = e._readPos;
      _score      = e._score;
      _mxmlOff    = e._mxmlOff;
      _bbox       = e._bbox;
      itemDiscovered = 0;
      }

//---------------------------------------------------------
//   linkTo
//---------------------------------------------------------

void Element::linkTo(Element* element)
      {
      if (!_links) {
            if (element->links())
                  _links = element->links();
            else {
                  _links = new LinkedElements;
                  _links->append(element);
                  element->setLinks(_links);
                  }
            }
      _links->append(this);
      }

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

Element* Element::linkedClone()
      {
      Element* e = clone();
      linkTo(e);
      return e;
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void Element::setPos(double x, double y)
      {
      _pos.rx() = x;
      _pos.ry() = y;
      }

//---------------------------------------------------------
//   adjustReadPos
//---------------------------------------------------------

void Element::adjustReadPos()
      {
      if (!_readPos.isNull()) {
            _userOff = _readPos - _pos;
            _readPos = QPointF();
            }
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Element::toDefault()
      {
      score()->undo()->push(new ChangeUserOffset(this, QPointF()));
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
      Staff* st = score()->staff(staffIdx());
      if (debugMode && st == 0) {
            printf("no staff: <%s> track %d staffIdx %d, staves %d\n",
               name(), track(), track()/VOICES, score()->staves().size());
            }
      return st;
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

      if (flag(ELEMENT_DROP_TARGET))
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

      qreal x = pos.x();
      qreal y = pos.y();

      qreal _spatium = spatium();
      if (mscore->hRaster()) {
            qreal hRaster = _spatium / preferences.hRaster;
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (mscore->vRaster()) {
            qreal vRaster = _spatium / preferences.vRaster;
            int n = lrint(y / vRaster);
            y = vRaster * n;
            }

      setUserOff(QPointF(x, y));
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
//      return bbox().intersects(r);
      return shape().intersects(r);
      }

//---------------------------------------------------------
//   properties
//---------------------------------------------------------

QList<Prop> Element::properties(Xml& xml, const Element* proto) const
      {
      QList<Prop> pl;
      if (_links && (_links->size() > 1))
            pl.append(Prop("lid", _links->lid()));
      if (_subtype) {
            QString s(subtypeName());
            if (!s.isEmpty())
                  pl.append(Prop("subtype", subtypeName()));
            }
      if (isMovable() && !userOff().isNull())
            pl.append(Prop("pos", pos() / spatium()));
      if ((track() != xml.curTrack) && (track() != -1)) {
            int t;
            t = track() + xml.trackDiff;
            pl.append(Prop("track", t));
            }
      if (selected())
            pl.append(Prop("selected", selected()));
      if (!visible())
            pl.append(Prop("visible", visible()));
      if (_color != preferences.defaultColor)
            pl.append(Prop("color", _color));
      if (flag(ELEMENT_SYSTEM_FLAG) && (proto == 0 || proto->systemFlag() != flag(ELEMENT_SYSTEM_FLAG)))
            pl.append(Prop("systemFlag", flag(ELEMENT_SYSTEM_FLAG)));
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Element::writeProperties(Xml& xml, const Element* proto) const
      {
      xml.prop(properties(xml, proto));
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "lid") {
            _links = score()->links().value(i);
            if (!_links) {
                  if (score()->parentScore())   // DEBUG
                        printf("---link %d not found (%d)\n", i, score()->links().size());
                  _links = new LinkedElements(i);
                  score()->links().insert(i, _links);
                  }
            _links->append(this);
            }
      else if (tag == "subtype") {
            // does not always call Element::setSubtype():
            this->setSubtype(val);
            }
      else if (tag == "tick")
            score()->curTick = score()->fileDivision(i);
      else if (tag == "offset") {         // ??obsolete
            QPointF pt(readPoint(e) * spatium());
            setUserOff(pt);
            _readPos = QPointF();
            }
      else if (tag == "pos")
            _readPos = readPoint(e) * spatium();
      else if (tag == "visible")
            setVisible(i);
      else if (tag == "voice")
            setTrack((_track/VOICES)*VOICES + i);
      else if (tag == "track") {
            // score()->curTrack = i;
            setTrack(i);
            }
      else if (tag == "selected")
            setSelected(i);
      else if (tag == "color")
            _color = readColor(e);
      else if (tag == "systemFlag") {
            setFlag(ELEMENT_SYSTEM_FLAG, i);
            if (i)
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
      if ((!_generated || type() == BAR_LINE) && (type() != LAYOUT_BREAK)) {
            if (visible())
                  a = popup->addAction(tr("Set Invisible"));
            else
                  a = popup->addAction(tr("Set Visible"));
            a->setData("invisible");
            a = popup->addAction(tr("Color..."));
            a->setData("color");
            }
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Element::propertyAction(ScoreView*, const QString& s)
      {
      foreach(Element* e, score()->selection().elements()) {
            if (e->type() == type()) {
                  if (s == "invisible") {
                        score()->toggleInvisible(e);
                        }
                  else if (s == "color") {
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
      double _dist;
      int l;
      if (staff()) {
            _dist = staff()->useTablature() ? 1.5 : 1.0;
            l     = staff()->lines();
            }
      else {
            _dist = 1.0;
            l     = 5;
            }

      double d = _dist * spatium();
      qreal lw = point(score()->styleS(ST_staffLineWidth));

      switch (l) {
            case 0:
                  return QRectF(0.0, - 2.0 * d - lw*.5, _width, 4 * d + lw);
            case 1:
                  return QRectF(0.0,  -lw*.5, _width, 4 * d + lw);
            case 2:
                  return QRectF(0.0, -lw*.5, _width, l * d * 2.0 + lw);
            default:
                  return QRectF(0.0, -lw*.5, _width, l * d + lw);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffLines::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
      double _dist;
      int l;
      if (staff()) {
            _dist = staff()->useTablature() ? 1.5 : 1.0;
            l     = staff()->lines();
            }
      else {
            _dist = 1.0;
            l     = 5;
            }
      QPointF _pos(0.0, 0.0);
      double d = _dist * spatium();

      QPen pen(p.pen());
      pen.setWidthF(point(score()->styleS(ST_staffLineWidth)));
      if (pen.widthF() * p.worldMatrix().m11() < 1.0)
            pen.setWidth(0);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);

      qreal x1 = _pos.x();
      qreal x2 = x1 + width();

      switch(l) {
            case 1:
                  {
                  qreal y = _pos.y() + 2 * d;
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
                  break;
            case 2:
                  {
                  qreal y = _pos.y() + 1 * d;
                  p.drawLine(QLineF(x1, y, x2, y));
                  y += 2 * d;
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
                  break;
            case 3:
                  for (int i = 0; i < l; ++i) {
                        qreal y = _pos.y() + i * d * 2.0;
                        p.drawLine(QLineF(x1, y, x2, y));
                        }
                  break;

            default:
                  for (int i = 0; i < l; ++i) {
                        qreal y = _pos.y() + i * d;
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
     System* system = measure()->system();
     if (system == 0)
           return 0.0;
      double _spatium = spatium();
      double y = system->staff(staffIdx())->y();
      int l = staff() ? staff()->lines() : 5;
      switch (l) {
            case 1:
                  return y + ipos().y() + _spatium;
            case 2:
                  return y + ipos().y() + _spatium;
            case 3:
            default:
                  return y + ipos().y();
            }
      }

//---------------------------------------------------------
//   y2
//---------------------------------------------------------

double StaffLines::y2() const
      {
      double _dist;
      int l;
      if (staff()) {
            _dist = staff()->useTablature() ? 1.5 : 1.0;
            l     = staff()->lines();
            }
      else {
            _dist = 1.0;
            l     = 5;
            }
      System* system = measure()->system();
      if (system == 0)
            return 0.0;
      double y = system->staff(staffIdx())->y();
      double d = _dist * spatium();

      switch (l) {
            case 1:
                  return y + ipos().y() + 3 * d;
            case 2:
                  return y + ipos().y() + 3 * d;
            case 3:
            default:
                  return y + ipos().y() + (l - 1) * d;
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

void Line::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
      QPen pen(p.pen());
      pen.setCapStyle(Qt::FlatCap);
      double sp = spatium();
      pen.setWidthF(_width.val() * sp);
      p.setPen(pen);

      double l = _len.val() * sp;
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

Compound::Compound(const Compound& c)
   : Element(c)
      {
      elemente.clear();
      foreach(Element* e, c.elemente)
            elemente.append(e->clone());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Compound::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
      foreach(Element* e, elemente) {
            QPointF pt(e->pos());
            p.translate(pt);
            e->draw(painter);
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
//   layout
//---------------------------------------------------------

void Compound::layout()
      {
      _bbox = QRectF();
      for (iElement i = elemente.begin(); i != elemente.end(); ++i) {
            Element* e = *i;
            e->layout();
            _bbox |= e->bbox().translated(e->pos());
            }
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
//   dump
//---------------------------------------------------------

void Element::dump() const
      {
      printf("---Element: %s, pos(%4.2f,%4.2f)\n"
         "   bbox(%g,%g,%g,%g)\n"
         "   abox(%g,%g,%g,%g)\n"
         "  parent: %p\n",
         name(), ipos().x(), ipos().y(),
         _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
         abbox().x(), abbox().y(), abbox().width(), abbox().height(),
         parent());
      }

//---------------------------------------------------------
//   RubberBand::draw
//---------------------------------------------------------

void RubberBand::draw(Painter* painter) const
      {
      if (!showRubberBand)
            return;
      QPainter& p = *painter->painter();
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
      score()->addRefresh(abbox());
      setUserOff(userOff() + delta);
      score()->addRefresh(abbox());
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
            case CHORDLINE:         return new ChordLine(score);
            case ACCIDENTAL:        return new Accidental(score);
            case DYNAMIC:           return new Dynamic(score);
            case TEXT:              return new Text(score);
            case INSTRUMENT_NAME:   return new InstrumentName(score);
            case STAFF_TEXT:        return new StaffText(score);
            case INSTRUMENT_CHANGE: return new InstrumentChange(score);
            case NOTEHEAD:          return new NoteHead(score);
            case NOTEDOT:           return new NoteDot(score);
            case TREMOLO:           return new Tremolo(score);
            case LAYOUT_BREAK:      return new LayoutBreak(score);
            case MARKER:            return new Marker(score);
            case JUMP:              return new Jump(score);
            case REPEAT_MEASURE:    return new RepeatMeasure(score);
            case ICON:              return new Icon(score);
            case NOTE:              return new Note(score);
            case SYMBOL:            return new Symbol(score);
            case FSYMBOL:           return new FSymbol(score);
            case CHORD:             return new Chord(score);
            case REST:              return new Rest(score);
            case SPACER:            return new Spacer(score);
            case STAFF_STATE:       return new StaffState(score);
            case TEMPO_TEXT:        return new TempoText(score);
            case HARMONY:           return new Harmony(score);
            case FRET_DIAGRAM:      return new FretDiagram(score);
            case BEND:              return new Bend(score);
            case TREMOLOBAR:        return new TremoloBar(score);
            case LYRICS:            return new Lyrics(score);
            case STEM:              return new Stem(score);
            case SLUR:              return new Slur(score);
            case ACCIDENTAL_BRACKET: return new AccidentalBracket(score);
            case FINGERING:          return new Fingering(score);
            case HBOX:              return new HBox(score);
            case VBOX:              return new VBox(score);
            case TBOX:              return new TBox(score);
            case FBOX:              return new FBox(score);
            case MEASURE:           return new Measure(score);
            case TAB_DURATION_SYMBOL: return new TabDurationSymbol(score);

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
            case TRILL_SEGMENT:
            case TEXTLINE_SEGMENT:
            case VOLTA_SEGMENT:
            case LEDGER_LINE:
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
            case FSYMBOL:           return "FSymbol";
            case TEXT:              return "Text";
            case INSTRUMENT_NAME:   return "InstrumentName";
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
            case CHORDLINE:         return "ChordLine";
            case DYNAMIC:           return "Dynamic";
            case PAGE:              return "Page";
            case BEAM:              return "Beam";
            case HOOK:              return "Hook";
            case LYRICS:            return "Lyrics";
            case MARKER:            return "Marker";
            case JUMP:              return "Jump";
            case FINGERING:         return "Fingering";
            case TUPLET:            return "Tuplet";
            case TEMPO_TEXT:        return "Tempo";
            case STAFF_TEXT:        return "StaffText";
            case INSTRUMENT_CHANGE: return "InstrumentChange";
            case HARMONY:           return "Harmony";
            case FRET_DIAGRAM:      return "FretDiagram";
            case BEND:              return "Bend";
            case TREMOLOBAR:        return "TremoloBar";
            case VOLTA:             return "Volta";
            case HAIRPIN_SEGMENT:   return "HairpinSegment";
            case OTTAVA_SEGMENT:    return "OttavaSegment";
            case TRILL_SEGMENT:     return "TrillSegment";
            case TEXTLINE_SEGMENT:  return "TextLineSegment";
            case VOLTA_SEGMENT:     return "VoltaSegment";
            case LAYOUT_BREAK:      return "LayoutBreak";
            case SPACER:            return "Spacer";
            case STAFF_STATE:       return "StaffState";
            case LEDGER_LINE:       return "LedgerLine";
            case NOTEHEAD:          return "NoteHead";
            case NOTEDOT:           return "NoteDot";
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
            case TBOX:              return "TBox";
            case FBOX:              return "FBox";
            case ICON:              return "Icon";
            case ACCIDENTAL_BRACKET:  return "AccidentalBracket";
            case TAB_DURATION_SYMBOL: return "TabDurationSymbol";
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

//---------------------------------------------------------
//   elementLessThan
//---------------------------------------------------------

bool elementLessThan(const Element* const e1, const Element* const e2)
      {
//      return e1->type() > e2->type();
      return e1->z() > e2->z();
      }
#if 0
//---------------------------------------------------------
//   setAlign
//---------------------------------------------------------

void Element::setAlign(Align val)
      {
      _align = val;
      }
#endif

