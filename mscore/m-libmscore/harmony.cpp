//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: harmony.cpp 3700 2010-11-10 15:29:43Z wschweer $
//
//  Copyright (C) 2008-2010 Werner Schweer and others
//
//  Some Code inspired by "The JAZZ++ Midi Sequencer"
//  Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
//
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

#include <QtCore/QStack>
#include <QtCore/QFileInfo>

#include "m-al/xml.h"
#include "harmony.h"
#include "pitchspelling.h"
#include "score.h"
#include "system.h"
#include "measure.h"
#include "segment.h"
#include "painter.h"

static const bool useJazzFont = true;     // DEBUG

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(const QString& str)
      {
      static const char* const scaleNames[2][12] = {
            { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
            { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
            };
      keys = 0;
      QStringList sl = str.split(" ", QString::SkipEmptyParts);
      foreach(const QString& s, sl) {
            for (int i = 0; i < 12; ++i) {
                  if (s == scaleNames[0][i] || s == scaleNames[1][i]) {
                        operator+=(i);
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(int a, int b, int c, int d, int e, int f, int g, int h, int i, int k, int l)
      {
      keys = 0;
      if (a >= 0)
            operator+=(a);
      if (b >= 0)
            operator+=(b);
      if (c >= 0)
            operator+=(c);
      if (d >= 0)
            operator+=(d);
      if (e >= 0)
            operator+=(e);
      if (f >= 0)
            operator+=(f);
      if (g >= 0)
            operator+=(g);
      if (h >= 0)
            operator+=(h);
      if (i >= 0)
            operator+=(i);
      if (k >= 0)
            operator+=(k);
      if (l >= 0)
            operator+=(l);
      }

//---------------------------------------------------------
//   rotate
//    rotate 12 Bits
//---------------------------------------------------------

void HChord::rotate(int semiTones)
      {
      while (semiTones > 0) {
            if (keys & 0x800)
                  keys = ((keys & ~0x800) << 1) + 1;
            else
                  keys <<= 1;
            --semiTones;
            }
      while (semiTones < 0) {
            if (keys & 1)
                  keys = (keys >> 1) | 0x800;
            else
                  keys >>= 1;
            ++semiTones;
            }
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString HChord::name(int tpc)
      {
      static const HChord C0(0,3,6,9);
      static const HChord C1(0,3);

      QString buf = tpc2name(tpc, false);
      HChord c(*this);

      int key = tpc2pitch(tpc);

      c.rotate(-key);        // transpose to C

      // special cases
      if (c == C0) {
            buf += "dim";
            return buf;
            }
      if (c == C1) {
            buf += "no5";
            return buf;
            }

      bool seven   = false;
      bool sharp9  = false;
      bool nat11   = false;
      bool sharp11 = false;
      bool nat13   = false;
      bool flat13  = false;

      // minor?
      if (c.contains(3)) {
            if (!c.contains(4))
                  buf += "m";
            else
                  sharp9 = true;
            }

      // 7
      if (c.contains(11)) {
            buf += "Maj7";
            seven = true;
            }
      else if (c.contains(10)) {
            buf += "7";
            seven = true;
            }

      // 4
      if (c.contains(5)) {
            if (!c.contains(4)) {
                  buf += "sus4";
                  }
            else
                  nat11 = true;
            }

      // 5
      if (c.contains(7)) {
            if (c.contains(6))
                  sharp11 = true;
            if (c.contains(8))
                  flat13 = true;
            }
      else {
            if (c.contains(6))
                  buf += "b5";
            if (c.contains(8))
                  buf += "#5";
            }

      // 6
      if (c.contains(9)) {
            if (!seven)
                  buf += "6";
            else
                  nat13 = true;
            }

      // 9
      if (c.contains(1))
            buf += "b9";
      if (c.contains(2))
            buf += "9";
      if (sharp9)
            buf += "#9";

      // 11
      if (nat11)
            buf += "11 ";
      if (sharp11)
            buf += "#11";

      // 13
      if (flat13)
            buf += "b13";
      if (nat13) {
            if (c.contains(1) || c.contains(2) || sharp9 || nat11 || sharp11)
                  buf += "13";
            else
                  buf += "add13";
            }
      return buf;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void HChord::add(const QList<HDegree>& degreeList)
      {
      // convert degrees to semitones
      static const int degreeTable[] = {
            // 1  2  3  4  5  6   7
            // C  D  E  F  G  A   B
               0, 2, 4, 5, 7, 9, 11
            };
      // factor in the degrees
      foreach(const HDegree& d, degreeList) {
            int dv  = degreeTable[(d.value() - 1) % 7] + d.alter();
            int dv1 = degreeTable[(d.value() - 1) % 7];

            if (d.value() == 7 && d.alter() == 0) {
                  // DEBUG: seventh degree is Bb, not B
                  //        except Maj   (TODO)
                  dv -= 1;
                  }

            if (d.type() == ADD)
                  *this += dv;
            else if (d.type() == ALTER) {
                  if (contains(dv1)) {
                        *this -= dv1;
                        *this += dv;
                        }
                  else {
//                        printf("ALTER: chord does not contain degree %d(%d):",
//                           d.value(), d.alter());
//                        print();
                        *this += dv;      // DEBUG: default to add
                        }
                  }
            else if (d.type() == SUBTRACT) {
                  if (contains(dv1))
                        *this -= dv1;
                  }
// printf("  HCHord::added  "); print();
            }
      }

//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

QString Harmony::harmonyName() const
      {
      bool germanNames = score()->styleB(ST_useGermanNoteNames);

      HChord hc = descr() ? descr()->chord : HChord();
      QString s;

      if (!_degreeList.isEmpty()) {
            hc.add(_degreeList);
            // try to find the chord in chordList
            const ChordDescription* newExtension = 0;
            ChordList* cl = score()->style().chordList();
            foreach(const ChordDescription* cd, *cl) {
                  if (cd->chord == hc && !cd->name.isEmpty()) {
                        newExtension = cd;
                        break;
                        }
                  }
            // now determine the chord name
            if (newExtension)
                  s = tpc2name(_rootTpc, germanNames) + newExtension->name;
            else
                  // not in table, fallback to using HChord.name()
                  s = hc.name(_rootTpc);
            //s += " ";
            } // end if (degreeList ...
      else {
            s = tpc2name(_rootTpc, germanNames);
            if (descr()) {
                  //s += " ";
                  s += descr()->name;
                  }
            }
      if (_baseTpc != INVALID_TPC) {
            s += "/";
            s += tpc2name(_baseTpc, germanNames);
            }
      return s;
      }

//---------------------------------------------------------
//   resolveDegreeList
//    try to detect chord number and to eliminate degree
//    list
//---------------------------------------------------------

void Harmony::resolveDegreeList()
      {
      if (_degreeList.isEmpty())
            return;

      HChord hc = descr() ? descr()->chord : HChord();

      hc.add(_degreeList);

// printf("resolveDegreeList: <%s> <%s-%s>: ", _descr->name, _descr->xmlKind, _descr->xmlDegrees);
// hc.print();
// _descr->chord.print();

      // try to find the chord in chordList
      ChordList* cl = score()->style().chordList();
      foreach(const ChordDescription* cd, *cl) {
            if ((cd->chord == hc) && !cd->name.isEmpty()) {
                  _id = cd->id;
                  _degreeList.clear();
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony(Score* score)
   : Text(score)
      {
      setSubtype(HARMONY);
      setTextStyle(TEXT_STYLE_HARMONY);

      _rootTpc   = INVALID_TPC;
      _baseTpc   = INVALID_TPC;
      _id        = -1;
      }

Harmony::Harmony(const Harmony& h)
   : Text(h)
      {
      _rootTpc    = h._rootTpc;
      _baseTpc    = h._baseTpc;
      _id         = h._id;
      _degreeList = h._degreeList;
      }

//---------------------------------------------------------
//   ~Harmony
//---------------------------------------------------------

Harmony::~Harmony()
      {
      foreach(const TextSegment* ts, textList)
            delete ts;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Harmony::read(XmlReader* r)
      {
      // convert table to tpc values
//      static const int table[] = {
//            14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19
//            };

      while (r->readElement()) {
            int i;
            if (r->readInt("base", &i))
                  setBaseTpc(i);
            else if (r->readInt("extension", &i))
                  setId(i);
            else if (r->readInt("root", &i))
                  setRootTpc(i);
            else if (r->tag() == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType = "";
                  while (r->readElement()) {
                        if (r->readInt("degree-value", &degreeValue))
                              ;
                        else if (r->readInt("degree-alter", &degreeAlter))
                              ;
                        else if (r->readString("degree-type", &degreeType))
                              ;
                        else
                              r->unknown();
                        }
                  if (degreeValue <= 0 || degreeValue > 13
                      || degreeAlter < -2 || degreeAlter > 2
                      || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                        }
                  else {
                        if (degreeType == "add")
                              addDegree(HDegree(degreeValue, degreeAlter, ADD));
                        else if (degreeType == "alter")
                              addDegree(HDegree(degreeValue, degreeAlter, ALTER));
                        else if (degreeType == "subtract")
                              addDegree(HDegree(degreeValue, degreeAlter, SUBTRACT));
                        }
                  }
            else if (!Text::readProperties(r))
                  r->unknown();
            }
      render();
      }

//---------------------------------------------------------
//   convertRoot
//    convert something like "C#" into tpc 21
//---------------------------------------------------------

static int convertRoot(const QString& s, bool germanNames)
      {
      int n = s.size();
      if (n < 1)
            return INVALID_TPC;
      int alter = 0;
      if (n > 1) {
            if (s[1].toLower().toAscii() == 'b')
                  alter = -1;
            else if (s[1] == '#')
                  alter = 1;
            }
      int r;
      if (germanNames) {
            switch(s[0].toLower().toAscii()) {
                  case 'c':   r = 0; break;
                  case 'd':   r = 1; break;
                  case 'e':   r = 2; break;
                  case 'f':   r = 3; break;
                  case 'g':   r = 4; break;
                  case 'a':   r = 5; break;
                  case 'h':   r = 6; break;
                  case 'b':
                        if (alter)
                              return INVALID_TPC;
                        r = 6;
                        alter = -1;
                        break;
                  default:
                        return INVALID_TPC;
                  }
            static const int spellings[] = {
               // bb  b   -   #  ##
                  0,  7, 14, 21, 28,  // C
                  2,  9, 16, 23, 30,  // D
                  4, 11, 18, 25, 32,  // E
                 -1,  6, 13, 20, 27,  // F
                  1,  8, 15, 22, 29,  // G
                  3, 10, 17, 24, 31,  // A
                  5, 12, 19, 26, 33,  // B
                  };
            r = spellings[r * 5 + alter + 2];
            }
      else {
            switch(s[0].toLower().toAscii()) {
                  case 'c':   r = 0; break;
                  case 'd':   r = 1; break;
                  case 'e':   r = 2; break;
                  case 'f':   r = 3; break;
                  case 'g':   r = 4; break;
                  case 'a':   r = 5; break;
                  case 'b':   r = 6; break;
                  default:    return INVALID_TPC;
                  }
            static const int spellings[] = {
               // bb  b   -   #  ##
                  0,  7, 14, 21, 28,  // C
                  2,  9, 16, 23, 30,  // D
                  4, 11, 18, 25, 32,  // E
                 -1,  6, 13, 20, 27,  // F
                  1,  8, 15, 22, 29,  // G
                  3, 10, 17, 24, 31,  // A
                  5, 12, 19, 26, 33,  // B
                  };
            r = spellings[r * 5 + alter + 2];
            }
      return r;
      }

//---------------------------------------------------------
//   parseHarmony
//    return ChordDescription
//---------------------------------------------------------

void Harmony::parseHarmony(const QString& ss, int* root, int* base)
      {
      _id = -1;
      QString s = ss.simplified();
      _userName = s;
      int n = s.size();
      if (n < 1)
            return;
      bool germanNames = score()->styleB(ST_useGermanNoteNames);
      int r = convertRoot(s, germanNames);
      if (r == INVALID_TPC) {
            return;
            }
      *root = r;
      int idx = ((n > 1) && ((s[1] == 'b') || (s[1] == '#'))) ? 2 : 1;
      *base = INVALID_TPC;
      int slash = s.indexOf('/');
      if (slash != -1) {
            QString bs = s.mid(slash+1);
            s     = s.mid(idx, slash - idx);
            *base = convertRoot(bs, germanNames);
            }
      else
            s = s.mid(idx).simplified();
      s = s.toLower();
      ChordList* cl = score()->style().chordList();
      foreach(const ChordDescription* cd, *cl) {
            if (cd->name.toLower() == s) {
                  _id = cd->id;
                  return;
                  }
            }
      _userName = s;
      }

//---------------------------------------------------------
//   setHarmony
//---------------------------------------------------------

void Harmony::setHarmony(const QString& s)
      {
      int r, b;
      parseHarmony(s, &r, &b);
      if (_id != -1) {
            setRootTpc(r);
            setBaseTpc(b);
            render();
            }
      else {
            // syntax error, leave text as is
            foreach(const TextSegment* s, textList)
                  delete s;
            textList.clear();

            setRootTpc(INVALID_TPC);
            setBaseTpc(INVALID_TPC);
            _id = -1;
            }
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal Harmony::baseLine() const
      {
      return (textList.isEmpty()) ? Text::baseLine() : 0.0;
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString HDegree::text() const
      {
      if (_type == UNDEF)
            return QString();
      QString degree;
      switch(_type) {
            case UNDEF: break;
            case ADD:         degree += "add"; break;
            case ALTER:       degree += "alt"; break;
            case SUBTRACT:    degree += "sub"; break;
            }
      switch(_alter) {
            case -1:          degree += "b"; break;
            case 1:           degree += "#"; break;
            default:          break;
            }
      return degree + QString("%1").arg(_value);
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind,  const QList<HDegree>& dl)
      {
      QStringList degrees;

      foreach(const HDegree& d, dl)
            degrees.append(d.text());

      QString lowerCaseKind = kind.toLower();
      ChordList* cl = score()->style().chordList();
      foreach(const ChordDescription* cd, *cl) {
            QString k     = cd->xmlKind;
            QStringList d = cd->xmlDegrees;
            if ((lowerCaseKind == k) && (d == degrees)) {
//                  printf("harmony found in db: %s %s -> %d\n", qPrintable(kind), qPrintable(degrees), cd->id);
                  return cd;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind)
      {
      QString lowerCaseKind = kind.toLower();
      ChordList* cl = score()->style().chordList();
      foreach(const ChordDescription* cd, *cl) {
            if (lowerCaseKind == cd->xmlKind)
                  return cd;
            }
      return 0;
      }

//---------------------------------------------------------
//   descr
//---------------------------------------------------------

const ChordDescription* Harmony::descr() const
      {
      return score()->style().chordDescription(_id);
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Harmony::isEmpty() const
      {
      return true; // return textList.isEmpty() && doc()->isEmpty();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Harmony::layout()
      {
      setSubtype(TEXT_CHORD);    // apply style changes

      if (textList.isEmpty()) {
            Text::layout();
            return;
            }
      style().layout(this);
      Measure* m = measure();
      qreal yy = track() < 0 ? 0.0 : m->system()->staff(track() / VOICES)->y();
      qreal xx = 0.0;  // (segment()->tick() < 0) ? 0.0 : m->tick2pos(segment()->tick());

      setPos(ipos() + QPointF(xx, yy));

      QRectF bb;
      foreach(const TextSegment* ts, textList)
            bb |= ts->boundingRect().translated(ts->x, ts->y);
      setbbox(bb);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

#if 0
QPainterPath Harmony::shape() const
      {
      QPainterPath pp;
      pp.addRect(bbox());
      return pp;
      }
#endif

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Harmony::draw(Painter* p) const
      {
      if (textList.isEmpty()) {
            Text::draw(p);
            return;
            }
      foreach(const TextSegment* ts, textList)
            p->drawText(ts->font, QPointF(ts->x, ts->y), ts->text);
      }

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

TextSegment::TextSegment(const QString& s, const Font& f, qreal x, qreal y)
      {
      set(s, f, x, y);
      select = false;
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

qreal TextSegment::width() const
      {
      FontMetricsF fm(font);
      qreal w = 0.0;
      foreach(QChar c, text) {
            w += fm.width(c);
            }
      return w;
      }

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

QRectF TextSegment::boundingRect() const
      {
      FontMetricsF fm(font);
      return fm.boundingRect(text);
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void TextSegment::set(const QString& s, const Font& f, qreal _x, qreal _y)
      {
      font = f;
      x    = _x;
      y    = _y;
      setText(s);
      }

//---------------------------------------------------------
//   readRenderList
//---------------------------------------------------------

static void readRenderList(QString val, QList<RenderAction>& renderList)
      {
      QStringList sl = val.split(" ", QString::SkipEmptyParts);
      foreach(const QString& s, sl) {
            if (s.startsWith("m:")) {
                  QStringList ssl = s.split(":", QString::SkipEmptyParts);
                  if (ssl.size() == 3) {
                        RenderAction a;
                        a.type = RenderAction::RENDER_MOVE;
                        a.movex = ssl[1].toDouble();
                        a.movey = ssl[2].toDouble();
                        renderList.append(a);
                        }
                  }
            else if (s == ":push")
                  renderList.append(RenderAction(RenderAction::RENDER_PUSH));
            else if (s == ":pop")
                  renderList.append(RenderAction(RenderAction::RENDER_POP));
            else if (s == ":n")
                  renderList.append(RenderAction(RenderAction::RENDER_NOTE));
            else if (s == ":a")
                  renderList.append(RenderAction(RenderAction::RENDER_ACCIDENTAL));
            else {
                  RenderAction a(RenderAction::RENDER_SET);
                  a.text = s;
                  renderList.append(a);
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordDescription::read(XmlReader* r)
      {
      while (r->readAttribute()) {
            if (r->tag() == "id")
                  id = r->intValue();
            }
      while (r->readElement()) {
            QString val;

            if (r->readString("name", &name))
                  ;
            else if (r->readString("xml", &xmlKind))
                  ;
            else if (r->readString("degree", &val))
                  xmlDegrees.append(val);
            else if (r->readString("voicing", &val))
                  chord = HChord(val);
            else if (r->readString("render", &val))
                  readRenderList(val, renderList);
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   ~ChordList
//---------------------------------------------------------

ChordList::~ChordList()
      {
      if (isDetached()) {
            QMapIterator<int, ChordDescription*> i(*this);
            while(i.hasNext()) {
                  i.next();
                  delete i.value();
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordList::read(XmlReader* r)
      {
      int fontIdx = 0;
      while (r->readElement()) {
            QString val;
            if (r->tag() == "font") {
                  ChordFont f;
                  while (r->readAttribute()) {
                        if (r->tag() == "family")
                              f.family = r->stringValue();
                        }
                  f.mag = 1.0;
                  qreal d;
                  while (r->readElement()) {
                        if (r->tag() == "sym") {
                              ChordSymbol cs;
                              cs.fontIdx = fontIdx;
                              while (r->readAttribute()) {
                                    if (r->tag() == "name")
                                          cs.name = r->stringValue();
                                    else if (r->tag() == "code")
                                          cs.code = r->intValue();
                                    }
                              symbols.insert(cs.name, cs);
                              r->read();
                              }
                        else if (r->readReal("mag", &d)) {
                              f.mag = d;
                              }
                        else
                              r->unknown();
                        }
                  fonts.append(f);
                  ++fontIdx;
                  }
            else if (r->tag() == "chord") {
                  int id = 0;
                  while (r->readAttribute()) {
                        if (r->tag() == "id")
                              id = r->intValue();
                        }
                  ChordDescription* cd = take(id);
                  if (cd == 0)
                        cd = new ChordDescription();
                  cd->read(r);
                  insert(id, cd);
                  }
            else if (r->readString("renderRoot", &val))
                  readRenderList(val, renderListRoot);
            else if (r->readString("renderBase", &val))
                  readRenderList(val, renderListBase);
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   read
//    read Chord List, return false on error
//---------------------------------------------------------

bool ChordList::read(const QString& /*name*/)
      {
#if 0
      QString path;
      QFileInfo ftest(name);
      if (ftest.isAbsolute())
            path = name;
      else
            path = QString("%1styles/%2").arg(mscoreGlobalShare).arg(name);
      if (name.isEmpty())
            return false;
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly)) {
            return false;
            }
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            return false;
            }
      docName = f.fileName();

      for (XmlReader* e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  // QString version = e.attribute(QString("version"));
                  // QStringList sl = version.split('.');
                  // int _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                  read(e);
                  }
            }
#endif
      return true;
      }

//---------------------------------------------------------
//   renderList
//---------------------------------------------------------

void Harmony::render(const QList<RenderAction>& renderList, qreal& x, qreal& y, int tpc)
      {
      ChordList* chordList = score()->style().chordList();
      QStack<QPointF> stack;
      int fontIdx = 0;
      qreal _spatium = spatium();
      qreal mag = (DPI / PPI) * (_spatium / (SPATIUM20 * DPI));

      foreach(const RenderAction& a, renderList) {
            if (a.type == RenderAction::RENDER_SET) {
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(a.text);
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(a.text);
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_MOVE) {
                  x += a.movex * mag;
                  y += a.movey * mag;
                  }
            else if (a.type == RenderAction::RENDER_PUSH)
                  stack.push(QPointF(x,y));
            else if (a.type == RenderAction::RENDER_POP) {
                  if (!stack.isEmpty()) {
                        QPointF pt = stack.pop();
                        x = pt.x();
                        y = pt.y();
                        }
                  }
            else if (a.type == RenderAction::RENDER_NOTE) {
                  bool germanNames = score()->styleB(ST_useGermanNoteNames);
                  QChar c;
                  int acc;
                  tpc2name(tpc, germanNames, &c, &acc);
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(QString(c));
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(QString(c));
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_ACCIDENTAL) {
                  QChar c;
                  int acc;
                  tpc2name(tpc, false, &c, &acc);
                  if (acc) {
                        TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                        QString s;
                        if (acc == -1)
                              s = "b";
                        else if (acc == 1)
                              s = "#";
                        ChordSymbol cs = chordList->symbol(s);
                        if (cs.isValid()) {
                              ts->font = fontList[cs.fontIdx];
                              ts->setText(QString(cs.code));
                              }
                        else
                              ts->setText(s);
                        textList.append(ts);
                        x += ts->width();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   render
//    construct Chord Symbol
//---------------------------------------------------------

void Harmony::render(const TextStyle* st)
      {
      if (_rootTpc == INVALID_TPC)
            return;

      if (st == 0)
            st = &score()->textStyle(_textStyle);
      ChordList* chordList = score()->style().chordList();

      fontList.clear();
      foreach(ChordFont cf, chordList->fonts) {
            if (cf.family.isEmpty() || cf.family == "default")
                  fontList.append(st->fontPx(spatium() * cf.mag));
            else {
                  Font ff(st->fontPx(spatium() * cf.mag));
                  ff.setFamily(cf.family);
                  fontList.append(ff);
                  }
            }
      if (fontList.isEmpty())
            fontList.append(st->fontPx(spatium()));

      foreach(const TextSegment* s, textList)
            delete s;
      textList.clear();

      qreal x = 0.0, y = 0.0;
      render(chordList->renderListRoot, x, y, _rootTpc);
      if (descr())
            render(descr()->renderList, x, y, 0);
      if (_baseTpc != INVALID_TPC)
            render(chordList->renderListBase, x, y, _baseTpc);
      }

