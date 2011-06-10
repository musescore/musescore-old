//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp 3678 2010-11-05 13:33:01Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#include "score.h"
#include "preferences.h"
#include "staff.h"
#include "utils.h"
#include "sym.h"
#include "symbol.h"
#include "clef.h"
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
#include "glissando.h"
#include "articulation.h"
#include "chord.h"
#include "spacer.h"
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
#include "segment.h"
#include "box.h"
#include "instrchange.h"
#include "m-al/xml.h"
#include "painter.h"

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
//      QT_TRANSLATE_NOOP("elementName", "Icon"),
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
//   spatium
//---------------------------------------------------------

qreal Element::spatium() const
      {
      Staff* s = staff();
      qreal v = _score->spatium();
      return s ? v * s->mag() : v;
      }

//---------------------------------------------------------
//   magS
//---------------------------------------------------------

qreal Element::magS() const
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
      return QString(name(type()));
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
      if (score()) {
            foreach(Element* e, score()->selection().elements()) {
                  if (e == this) {
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
//   setPos
//---------------------------------------------------------

void Element::setPos(qreal x, qreal y)
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

Color Element::curColor() const
      {
      // the default element color is always interpreted as black in
      // printing
      if (score() && score()->printing())
            return (_color == preferences.defaultColor) ? Color(0,0,0) : _color;

      if (flag(ELEMENT_DROP_TARGET))
            return preferences.dropColor;
      if (_selected) {
            if (track() == -1)
                  return preferences.selectColor[0];
            else
                  return preferences.selectColor[voice()];
            }
      if (!_visible)
            return Color(80,80,80);
      return _color;
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

bool Element::contains(const QPointF& /*p*/) const
      {
      return false; // shape().contains(p - canvasPos());
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
#if 0
QPainterPath Element::shape() const
      {
      QPainterPath pp;
      pp.addRect(bbox());
      return pp;
      }
#endif
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
//TODO      return shape().intersects(r);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(XmlReader* r)
      {
      MString8 tag = r->tag();
      QString val;
      int i;
      QPointF pt;

      if (r->readInt("lid", &i)) {
#if 0
            _links = score()->links().value(i);
            if (!_links) {
                  _links = new LinkedElements(i);
                  score()->links().insert(i, _links);
                  }
            _links->append(this);
#endif
            }
      else if (r->readString("subtype", &val)) {
            // does not always call Element::setSubtype():
            this->setSubtype(val);
            }
      else if (r->readInt("tick", &i))
            score()->curTick = score()->fileDivision(i);
      else if (r->readPoint("offset", &pt)) {
            setUserOff(pt * spatium());
            _readPos = QPointF();
            }
      else if (r->readPoint("pos", &_readPos))
            _readPos *= spatium();
      else if (r->readInt("visible", &i))
            setVisible(i);
      else if (r->readInt("voice", &i))
            setTrack((_track/VOICES)*VOICES + i);
      else if (r->readInt("track", &i))
            setTrack(i);
      else if (r->readInt("selected", &i))
            setSelected(i);
      else if (r->readColor("color", &_color))
            ;
      else if (r->readInt("systemFlag", &i)) {
            setFlag(ELEMENT_SYSTEM_FLAG, i);
            if (i)
                  _track = 0;
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Element::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (!Element::readProperties(r))
                  r->unknown();
            }
      if (_subtype == 0)      // make sure setSubtype() is called at least once
            setSubtype(0);
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
            return;
            }
      QList<Element*>::replace(idx, n);
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
      qreal _dist;
      int l;
      if (staff()) {
            _dist = staff()->useTablature() ? 1.5 : 1.0;
            l     = staff()->lines();
            }
      else {
            _dist = 1.0;
            l     = 5;
            }

      qreal d = _dist * spatium();
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

void StaffLines::draw(Painter* p) const
      {
      qreal _dist;
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
      qreal d = _dist * spatium();

      p->setPenWidth(point(score()->styleS(ST_staffLineWidth)));
      p->setLineCap(CAP_BUTT);

      qreal x1 = _pos.x();
      qreal x2 = x1 + width();

      switch(l) {
            case 1:
                  {
                  qreal y = _pos.y() + 2 * d;
                  p->drawLine(x1, y, x2, y);
                  }
                  break;
            case 2:
                  {
                  qreal y = _pos.y() + 1 * d;
                  p->drawLine(x1, y, x2, y);
                  y += 2 * d;
                  p->drawLine(x1, y, x2, y);
                  }
                  break;
            case 3:
                  for (int i = 0; i < l; ++i) {
                        qreal y = _pos.y() + i * d * 2.0;
                        p->drawLine(x1, y, x2, y);
                        }
                  break;

            default:
                  for (int i = 0; i < l; ++i) {
                        qreal y = _pos.y() + i * d;
                        p->drawLine(x1, y, x2, y);
                        }
                  break;
            }
      }

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

qreal StaffLines::y1() const
      {
     System* system = measure()->system();
     if (system == 0)
           return 0.0;
      qreal _spatium = spatium();
      qreal y = system->staff(staffIdx())->y();
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

qreal StaffLines::y2() const
      {
      qreal _dist;
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
      qreal y = system->staff(staffIdx())->y();
      qreal d = _dist * spatium();

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
      qreal sp = spatium();
      qreal w  = _width.val() * sp;
      qreal l  = _len.val() * sp;
      qreal w2 = w * .5;
      if (vertical)
            setbbox(QRectF(-w2, -w2, w, l + w));
      else
            setbbox(QRectF(-w2, -w2, l + w, w));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Line::draw(Painter* p) const
      {
      p->save();

      p->setLineCap(Qt::FlatCap);
      qreal sp = spatium();
      p->setPenWidth(_width.val() * sp);

      qreal l = _len.val() * sp;
      if (vertical)
            p->drawLine(0.0, 0.0, 0.0, l);
      else
            p->drawLine(0.0, 0.0, l, 0.0);
      p->restore();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Line::readProperties(XmlReader* r)
      {
      qreal v;

      if (r->readReal("lineWidth", &v))
            _width = Spatium(v);
      else if (r->readReal("lineLen", &v))
            _len = Spatium(v);
      else if (r->readBool("vertical", &vertical))
            ;
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

void Compound::draw(Painter* p) const
      {
      foreach(Element* e, elemente) {
            p->translate(e->pos());
            e->draw(p);
            p->translate(-e->pos());
            }
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 offset \a x and \a y are in Point units
*/

void Compound::addElement(Element* e, qreal x, qreal y)
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
//   RubberBand::draw
//---------------------------------------------------------

void RubberBand::draw(Painter* p) const
      {
      p->setPenColor(Color(255, 0, 0));
      p->drawLine(_p1.x(), _p1.y(), _p2.x(), _p2.y());
      }

//---------------------------------------------------------
//   readType
//---------------------------------------------------------

ElementType Element::readType(XmlReader* r, QPointF* dragOffset)
      {
      ElementType type = INVALID;

      while (r->readElement()) {
            if (r->readPoint("dragOffset", dragOffset))
                  ;
            else {
                  type = name2type(QString::fromUtf8((const char*)(r->tag().s())));
                  if (type == INVALID) {
                        r->unknown();
                        break;
                        }
                  }
            if (type != INVALID)
                  break;
            }
      return type;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Element::add(Element*)
      {
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Element::remove(Element*)
      {
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
            case STAFF_TEXT:        return new StaffText(score);
            case INSTRUMENT_CHANGE: return new InstrumentChange(score);
            case NOTEHEAD:          return new NoteHead(score);
            case NOTEDOT:           return new NoteDot(score);
            case TREMOLO:           return new Tremolo(score);
            case LAYOUT_BREAK:      return new LayoutBreak(score);
            case MARKER:            return new Marker(score);
            case JUMP:              return new Jump(score);
            case REPEAT_MEASURE:    return new RepeatMeasure(score);
//            case ICON:              return new Icon(score);
            case NOTE:              return new Note(score);
            case SYMBOL:            return new Symbol(score);
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

            default:
                  break;
            }
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
            case ACCIDENTAL_BRACKET: return "AccidentalBracket";
            default:
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
      return e1->type() > e2->type();
      }
