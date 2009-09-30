//=============================================================================
//  MuseScore
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

/**
 \file
 Implementation of classes Chord, LedgerLine, NoteList Stem ans StemSlash.
*/

#include "chord.h"
#include "note.h"
#include "xml.h"
#include "style.h"
#include "segment.h"
#include "text.h"
#include "measure.h"
#include "system.h"
#include "tuplet.h"
#include "hook.h"
#include "slur.h"
#include "arpeggio.h"
#include "score.h"
#include "tremolo.h"
#include "glissando.h"
#include "staff.h"
#include "utils.h"
#include "articulation.h"

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(QPainter& p) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      QPen pen(p.pen());
      pen.setWidthF(lw);
      p.setPen(pen);

      p.drawLine(QLineF(0.0, 0.0, 0.0, stemLen()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Stem::write(Xml& xml) const
      {
      xml.stag("Stem");
      Element::writeProperties(xml);
      if (_userLen.val() != 0.0)
            xml.sTag("userLen", _userLen);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Stem::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "userLen")
                  _userLen = Spatium(e.text().toDouble());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Stem::setVisible(bool f)
      {
      Element::setVisible(f);
      Chord* chord = static_cast<Chord*>(parent());
      if (chord && chord->hook() && chord->hook()->visible() != f)
            chord->hook()->setVisible(f);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Stem::bbox() const
      {
      double w = point(score()->styleS(ST_stemWidth));
      double l = _len + point(_userLen);
      return QRectF(-w * .5, 0, w, l).normalized();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Stem::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      QPointF p(0.0, stemLen());
      grip[0].translate(canvasPos() + p);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Stem::editDrag(int, const QPointF& delta)
      {
      _userLen += Spatium(delta.y() / spatium());
      Chord* c = static_cast<Chord*>(parent());
      if (c->hook())
            c->hook()->move(0.0, delta.y());
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Stem::toDefault()
      {
      _userLen = Spatium(0.0);
      setUserOff(QPointF());
      }

//---------------------------------------------------------
//   StemSlash
//---------------------------------------------------------

StemSlash::StemSlash(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StemSlash::draw(QPainter& p) const
      {
      qreal lw = point(score()->styleS(ST_stemWidth));
      QPen pen(p.pen());
      pen.setWidthF(lw);
      p.setPen(pen);
      p.drawLine(line);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF StemSlash::bbox() const
      {
      double w = point(score()->styleS(ST_stemWidth)) * .5;
      QRectF r(line.p1().x(), line.p2().y(),
         line.p2().x()-line.p1().x(),
         line.p1().y()-line.p2().y()
         );
      return r.adjusted(-w, -w, 2.0 * w, 2.0 * w);
      }

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int Chord::upLine() const
      {
      return upNote()->line();
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int Chord::downLine() const
      {
      return downNote()->line();
      }

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

Chord::Chord(Score* s)
   : ChordRest(s)
      {
      _up            = true;
      _stem          = 0;
      _hook          = 0;
      _stemDirection = AUTO;
      _arpeggio      = 0;
      _tremolo       = 0;
      _glissando     = 0;
      _noteType      = NOTE_NORMAL;
      _stemSlash     = 0;
      _noStem        = false;
      minSpace       = 0.0;
      extraSpace     = 0.0;
      }

Chord::Chord(const Chord& c)
   : ChordRest(c)
      {
      for (ciNote i = c.notes.begin(); i != c.notes.end(); ++i)
            add(new Note(*(i->second)));

      foreach(const LedgerLine* ll, c._ledgerLines) {
            LedgerLine* l = new LedgerLine(*ll);
            l->setParent(this);
            l->setTrack(track());
            _ledgerLines.append(new LedgerLine(*ll));
            }

      _stem          = 0;
      _hook          = 0;
      _glissando     = 0;
      _arpeggio      = 0;
      _stemSlash     = 0;

      _noStem = c._noStem;
      if (c._stem)
            add(new Stem(*(c._stem)));
      if (c._hook)
            add(new Hook(*(c._hook)));
      if (c._glissando)
            add(new Glissando(*(c._glissando)));
      if (c._arpeggio)
            add(new Arpeggio(*(c._arpeggio)));
      if (c._stemSlash) {
            _stemSlash = new StemSlash(*(c._stemSlash));
            _stemSlash->setParent(this);
            _stemSlash->setTrack(track());
            }
      _stemDirection = c._stemDirection;
      _tremolo       = 0;
      _noteType      = c._noteType;
      minSpace       = c.minSpace;
      extraSpace     = c.extraSpace;
      }

//---------------------------------------------------------
//   ~Chord
//---------------------------------------------------------

Chord::~Chord()
      {
      delete _arpeggio;
      delete _tremolo;
      delete _glissando;
      delete _stemSlash;
      delete _stem;
      }

//---------------------------------------------------------
//   setHook
//---------------------------------------------------------

void Chord::setHook(Hook* f)
      {
      delete _hook;
      _hook = f;
      if (_hook) {
            _hook->setParent(this);
            if (_stem)        // should always be true
                  _hook->setVisible(_stem->visible());
            }
      }

//---------------------------------------------------------
//   setStem
//---------------------------------------------------------

void Chord::setStem(Stem* s)
      {
      delete _stem;
      _stem = s;
      if (_stem) {
            _stem->setParent(this);
            _stem->setTrack(track());
            }
      }

//---------------------------------------------------------
//   stemPos
//---------------------------------------------------------

QPointF Chord::stemPos(bool upFlag, bool top) const
      {
      const Note* note = (top ? !upFlag : upFlag) ? downNote() : upNote();
      return note->stemPos(upFlag) + pos() + segment()->pos();
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Chord::setSelected(bool f)
      {
      Element::setSelected(f);
      NoteList* nl = noteList();
      for (iNote in = nl->begin(); in != nl->end(); ++in)
            in->second->setSelected(f);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Chord::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());
      if (e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            notes.add(note);
            if (note->tieFor()) {
                  if (note->tieFor()->endNote())
                        note->tieFor()->endNote()->setTieBack(note->tieFor());
                  }
            }
      else if (e->type() == ARTICULATION)
            articulations.push_back(static_cast<Articulation*>(e));
      else if (e->type() == ARPEGGIO)
            _arpeggio = static_cast<Arpeggio*>(e);
      else if (e->type() == TREMOLO) {
            Tremolo* tr = static_cast<Tremolo*>(e);
            if (tr->twoNotes()) {
                  Duration d = duration();
                  d  = d.shift(-1);
                  if (tr->chord1())
                        tr->chord1()->setDuration(d);
                  if (tr->chord2())
                        tr->chord2()->setDuration(d);
                  }
            _tremolo = tr;
            }
      else if (e->type() == GLISSANDO)
            _glissando = static_cast<Glissando*>(e);
      else if (e->type() == STEM)
            _stem = static_cast<Stem*>(e);
      else if (e->type() == HOOK)
            _hook = static_cast<Hook*>(e);
      else
            printf("Chord::add: unknown element\n");
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Chord::remove(Element* e)
      {
      if (e->type() == NOTE) {
            iNote i = notes.begin();
            for (; i != notes.end(); ++i) {
                  if (i->second == e) {
                        Note* note = (Note*)e;
                        if (note->tieFor()) {
                              if (note->tieFor()->endNote())
                                    note->tieFor()->endNote()->setTieBack(0);
                              }
                        notes.erase(i);
                        break;
                        }
                  }
            if (i == notes.end())
                  printf("Chord::remove() note %p not found!\n", e);
            }
      else if (e->type() == ARTICULATION) {
            if (!articulations.removeOne(static_cast<Articulation*>(e)))
                  printf("Chord::remove(): attribute not found\n");
            }
      else if (e->type() == ARPEGGIO)
            _arpeggio = 0;
      else if (e->type() == TREMOLO) {
            Tremolo* tremolo = static_cast<Tremolo*>(e);
            if (tremolo->twoNotes()) {
                  Duration d = duration();
                  d          = d.shift(1);
                  if (tremolo->chord1())
                        tremolo->chord1()->setDuration(d);
                  if (tremolo->chord2())
                        tremolo->chord2()->setDuration(d);
                  }
            _tremolo = 0;
            }
      else if (e->type() == GLISSANDO)
            _glissando = 0;
      else if (e->type() == STEM)
            _stem = 0;
      else if (e->type() == HOOK)
            _hook = 0;
      else
            printf("Chord::remove: unknown element\n");
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Chord::bbox() const
      {
      QRectF _bbox;
      for (ciNote i = notes.begin(); i != notes.end(); ++i)
            _bbox |= i->second->bbox().translated(i->second->pos());
      foreach(const LedgerLine* l, _ledgerLines)
            _bbox |= l->bbox().translated(l->pos());
      for (ciArticulation i = articulations.begin(); i != articulations.end(); ++i)
            _bbox |= (*i)->bbox().translated((*i)->pos());
      if (_hook)
            _bbox |= _hook->bbox().translated(_hook->pos());
      if (_stem)
            _bbox |= _stem->bbox().translated(_stem->pos());
      if (_arpeggio)
            _bbox |= _arpeggio->bbox().translated(_arpeggio->pos());
      if (_glissando)
            _bbox |= _glissando->bbox().translated(_glissando->pos());
      if (_stemSlash)
            _bbox |= _stemSlash->bbox().translated(_stemSlash->pos());
      if (_tremolo)
            _bbox |= _tremolo->bbox().translated(_tremolo->pos());
      return _bbox;
      }

//---------------------------------------------------------
//   layoutStem1
//    called before layout spacing of notes
//    set hook if necessary to get right note width for next
//       pass
//---------------------------------------------------------

/**
 Layout chord stem and hook.
*/

void Chord::layoutStem1()
      {
      int istaff = staffIdx();

      //-----------------------------------------
      //  process stem
      //-----------------------------------------

      bool hasStem = duration().hasStem() && !(_noStem || measure()->slashStyle(istaff));
      int hookIdx  = hasStem ? duration().hooks() : 0;

      if (hasStem) {
            if (!_stem)
                  setStem(new Stem(score()));
            }
      else
            setStem(0);

      if (hasStem && _noteType == NOTE_ACCIACCATURA) {
            _stemSlash = new StemSlash(score());
            _stemSlash->setMag(mag());
            _stemSlash->setParent(this);
            }
      else {
            delete _stemSlash;
            _stemSlash = 0;
            }

      //-----------------------------------------
      //  process hook
      //-----------------------------------------

      if (hookIdx) {
            if (!up())
                  hookIdx = -hookIdx;
            if (!_hook)
                  setHook(new Hook(score()));
            _hook->setMag(mag());
            _hook->setSubtype(hookIdx);
            }
      else
            setHook(0);
      }

//---------------------------------------------------------
//   addLedgerLine
///   Add a ledger line to a chord.
///   \arg x          center of note head
///   \arg staffIdx   determines the y origin
///   \arg line       vertical position of line
//---------------------------------------------------------

void Chord::addLedgerLine(double x, int staffIdx, int line, int lr)
      {
      double _spatium = spatium();
      double hw       = upNote()->headWidth();
      double hw2      = hw * .5;

      double y = line * _spatium * .5;

      LedgerLine* h   = new LedgerLine(score());
      h->setTrack(staffIdx * VOICES);

      Spatium len(h->len());

      if (_noteType != NOTE_NORMAL)
            len *= score()->style(ST_graceNoteMag).toDouble();
      x -= len.val() * _spatium * .5;

      x += (lr & 1) ? -hw2 : hw2;
      if (lr == 3)
            len += Spatium(hw / spatium());

      h->setParent(this);
      h->setTrack(staffIdx * VOICES);

      //
      // Experimental:
      //  shorten ledger line to avoid collisions with accidentals
      //

      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* n = in->second;
            if (n->line() >= (line-1) && n->line() <= (line+1) && n->accidentalType() != ACC_NONE) {
                  x   += _spatium * .25;
                  len -= Spatium(.25);
                  break;
                  }
            }
      h->setLen(len);
      h->setPos(x, y);
      _ledgerLines.push_back(h);
      }

//---------------------------------------------------------
//   addLedgerLines
//---------------------------------------------------------

void Chord::addLedgerLines(double x, int move)
      {
      int uppos = 1000;
      int ulr   = 0;
      int idx   = staffIdx() + move;

      for (riNote in = notes.rbegin(); in != notes.rend(); ++in) {
            const Note* note = in->second;
            if (note->staffMove() != move)
                  continue;
            int l = note->line();
            if (l >= 0)
                  break;
            for (int i = (uppos+1) & ~1; i < l; i += 2)
                  addLedgerLine(x, idx, i, ulr);
            ulr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            uppos = l;
            }
      for (int i = (uppos+1) & ~1; i <= -2; i += 2)
            addLedgerLine(x, idx, i, ulr);

      int downpos = -1000;
      int dlr = 0;
      for (ciNote in = notes.begin(); in != notes.end(); ++in) {
            const Note* note = in->second;
            if (note->staffMove() != move)
                  continue;
            int l = note->line();
            if (l <= 8)
                  break;
            for (int i = downpos & ~1; i > l; i -= 2)
                  addLedgerLine(x, idx, i, dlr);
            dlr |= (up() ^ note->mirror()) ? 0x1 : 0x2;
            downpos = l;
            }
      for (int i = downpos & ~1; i >= 10; i -= 2)
            addLedgerLine(x, idx, i, dlr);
      }

//-----------------------------------------------------------------------------
//   computeUp
//    rules:
//      single note:
//          All notes beneath the middle line: upward stems
//          All notes on or above the middle line: downward stems
//      two notes:
//          If the interval above the middle line is greater than the interval
//             below the middle line: downward stems
//          If the interval below the middle line is greater than the interval
//             above the middle line: upward stems
//          If the two notes are the same distance from the middle line:
//             stem can go in either direction. but most engravers prefer
//             downward stems
//       > two notes:
//          If the interval of the highest note above the middle line is greater
//             than the interval of the lowest note below the middle line:
//             downward stems
//          If the interval of the lowest note below the middle line is greater
//             than the interval of the highest note above the middle line:
//             upward stem
//          If the highest and the lowest notes are the same distance from
//          the middle line:, use these rules to determine stem direction:
//             - If the majority of the notes are above the middle:
//               downward stems
//             - If the majority of the notes are below the middle:
//               upward stems
//-----------------------------------------------------------------------------

void Chord::computeUp()
      {
      if (_stemDirection != AUTO) {
            _up = _stemDirection == UP;
            return;
            }
      if (_noteType != NOTE_NORMAL) {
            _up = true;
            return;
            }

      int si = staffIdx();
      if (measure()->mstaff(si)->hasVoices) {
            switch(voice()) {
                  case 0:  _up = (score()->style(ST_stemDir1).toDirection() == UP); break;
                  case 1:  _up = (score()->style(ST_stemDir2).toDirection() == UP); break;
                  case 2:  _up = (score()->style(ST_stemDir3).toDirection() == UP); break;
                  case 3:  _up = (score()->style(ST_stemDir4).toDirection() == UP); break;
                  }
            return;
            }

      Note* upnote = upNote();
      if (notes.size() == 1) {
            if (upnote->staffMove() > 0)
                  _up = true;
            else if (upnote->staffMove() < 0)
                  _up = false;
            else
                  _up = upnote->line() > 4;
            return;
            }
      Note* downnote = downNote();
      int ud = upnote->line() - 4;
      int dd = downnote->line() - 4;
      if (-ud == dd) {
            int up = 0;
            for (ciNote in = notes.begin(); in != notes.end(); ++in) {
                  int l = in->second->line();
                  if (l <= 4)
                        --up;
                  else
                        ++up;
                  }
            _up = up > 0;
            }
      _up = dd > -ud;
      }

//---------------------------------------------------------
//   staffMove
//---------------------------------------------------------

int Chord::staffMove() const
      {
      int move = notes.front()->staffMove();
      for (ciNote in = notes.begin(); in != notes.end(); ++in) {
            if (in->second->staffMove() != move)
                  return 0;
            }
      return move;
      }

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
      {
      Note* note = 0;
      for (ciNote in = notes.begin(); in != notes.end(); ++in) {
            if (in->second->selected()) {
                  if (note)
                        return 0;
                  note = in->second;
                  }
            }
      return note;
      }

//---------------------------------------------------------
//   Chord::write
//---------------------------------------------------------

void Chord::write(Xml& xml, int startTick, int endTick) const
      {
      xml.stag("Chord");
      ChordRest::writeProperties(xml);
      if (_noteType != NOTE_NORMAL) {
            switch(_noteType) {
                  case NOTE_INVALID:
                  case NOTE_NORMAL:
                        break;
                  case NOTE_ACCIACCATURA:
                        xml.tagE("acciaccatura");
                        break;
                  case NOTE_APPOGGIATURA:
                        xml.tagE("appoggiatura");
                        break;
     	            case NOTE_GRACE4:
                        xml.tagE("grace4");
                        break;
                  case NOTE_GRACE16:
                        xml.tagE("grace16");
                        break;
                  case NOTE_GRACE32:
                        xml.tagE("grace32");
                        break;
                  }
            }
      if (_noStem)
            xml.tag("noStem", _noStem);
      else if (_stem) {
            if (!_stem->userOff().isNull() || (_stem->userLen().val() != 0.0) || !_stem->visible())
                  _stem->write(xml);
            }
      switch(_stemDirection) {
            case UP:   xml.tag("StemDirection", QVariant("up")); break;
            case DOWN: xml.tag("StemDirection", QVariant("down")); break;
            case AUTO: break;
            }
      ciNote in = notes.begin();
      for (; in != notes.end(); ++in)
            in->second->write(xml, startTick, endTick);
      if (_arpeggio)
            _arpeggio->write(xml);
      if (_glissando)
            _glissando->write(xml);
      if (_tremolo)
            _tremolo->write(xml);
      xml.etag();
      xml.curTick = tick() + ticks();
      }

//---------------------------------------------------------
//   Chord::readNote
//---------------------------------------------------------

void Chord::readNote(QDomElement e, const QList<Tuplet*>& tuplets, const QList<Beam*>& beams)
      {
      Note* note = new Note(score());
      int ptch   = e.attribute("pitch", "-1").toInt();
      int ticks  = e.attribute("ticks", "-1").toInt();
      int tpc    = e.attribute("tpc", "-1").toInt();

      if (ticks != -1) {
            Duration d;
            d.setVal(ticks);
            setDuration(d);
            }

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (tag == "pitch")
                  note->setPitch(i);
            else if (tag == "prefix") {
                  printf("read Note:: prefix: TODO\n");
                  }
            else if (tag == "line")
                  note->setLine(i);
            else if (tag == "Tie") {
                  Tie* _tieFor = new Tie(score());
                  _tieFor->setTrack(track());
                  _tieFor->read(e);
                  _tieFor->setStartNote(note);
                  note->setTieFor(_tieFor);
                  }
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setSubtype(TEXT_FINGERING);
                  f->setTextStyle(TEXT_STYLE_FINGERING);
                  f->read(e);
                  f->setParent(this);
                  note->add(f);
                  }
            else if (tag == "move")
                  note->setStaffMove(i);
            else if (!ChordRest::readProperties(e, tuplets, beams))
                  domError(e);
            }
      if (ptch != -1)
            note->setPitch(ptch);
      if (tpc != -1)
            note->setTpc(tpc);
      add(note);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Chord::read(QDomElement e)
      {
      QList<Tuplet*> tl;
      QList<Beam*> bl;
      read(e, tl, bl);
      convertTicks();
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(QDomElement e, const QList<Tuplet*>& tuplets, const QList<Beam*>& beams)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "Note") {
                  Note* note = new Note(score());
                  note->setParent(this);
                  note->setTrack(track());
                  note->read(e);
                  notes.add(note);
                  }
            else if (tag == "appoggiatura")
                  _noteType = NOTE_APPOGGIATURA;
            else if (tag == "acciaccatura")
                  _noteType = NOTE_ACCIACCATURA;
            else if (tag == "grace4")
                  _noteType = NOTE_GRACE4;
            else if (tag == "grace16")
                  _noteType = NOTE_GRACE16;
            else if (tag == "grace32")
                  _noteType = NOTE_GRACE32;
            else if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (tag == "noStem")
                  _noStem = i;
            else if (tag == "Arpeggio") {
                  _arpeggio = new Arpeggio(score());
                  _arpeggio->setTrack(track());
                  _arpeggio->read(e);
                  _arpeggio->setParent(this);
                  }
            else if (tag == "Glissando") {
                  _glissando = new Glissando(score());
                  _glissando->setTrack(track());
                  _glissando->read(e);
                  _glissando->setParent(this);
                  }
            else if (tag == "Tremolo") {
                  _tremolo = new Tremolo(score());
                  _tremolo->setTrack(track());
                  _tremolo->read(e);
                  _tremolo->setParent(this);
                  }
            else if (tag == "tickOffset")       // obsolete
                  ;
            else if (tag == "Stem") {
                  _stem = new Stem(score());
                  _stem->read(e);
                  add(_stem);
                  }
            else if (!ChordRest::readProperties(e, tuplets, beams))
                  domError(e);
            }
      convertTicks();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Chord::dump() const
      {
      printf("Chord tick %d  len %d\n", tick(), ticks());
      }

//---------------------------------------------------------
//   find
//---------------------------------------------------------

Note* NoteList::find(int pitch) const
      {
      ciNote i = std::multimap<const int, Note*>::find(pitch);
      if (i != end())
            return i->second;
      return 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

NoteList::iterator NoteList::add(Note* n)
      {
      return std::multimap<const int, Note*>::insert(std::pair<const int, Note*> (n->pitch(), n));
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Chord::upPos() const
      {
      return upNote()->pos().y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Chord::downPos() const
      {
      return downNote()->pos().y();
      }

//---------------------------------------------------------
//   centerX
//    return x position for attributes
//---------------------------------------------------------

qreal Chord::centerX() const
      {
      const Note* note = isUp() ? upNote() : downNote();
      qreal x = note->pos().x();
      x += note->headWidth() * .5;
      if (note->mirror()) {
            x += note->headWidth() * (isUp() ? -1.0 : 1.0);
            }
      return x;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Chord::scanElements(void* data, void (*func)(void*, Element*))
      {
      if (_hook)
            func(data, _hook);
      if (_stem)
            func(data, _stem);
      if (_stemSlash)
            func(data, _stemSlash);
      if (_arpeggio)
            func(data, _arpeggio);
      if (_tremolo)
            func(data, _tremolo);
      if (_glissando)
            func(data, _glissando);

      foreach(LedgerLine* h, _ledgerLines)
            func(data, h);

      for (iNote in = notes.begin(); in != notes.end(); ++in)
            in->second->scanElements(data, func);
      ChordRest::scanElements(data, func);
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Chord::setTrack(int val)
      {
      Element::setTrack(val);
      if (_hook)
            _hook->setTrack(val);
      if (_stem)
            _stem->setTrack(val);
      if (_stemSlash)
            _stemSlash->setTrack(val);
      if (_arpeggio)
            _arpeggio->setTrack(val);
      if (_glissando)
            _glissando->setTrack(val);
      if (_tremolo)
            _tremolo->setTrack(val);

      foreach(LedgerLine* h, _ledgerLines)
            h->setTrack(val);

      for (ciNote in = notes.begin(); in != notes.end(); ++in)
            in->second->setTrack(val);
      }

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
   : Line(s, false)
      {
      setLineWidth(score()->styleS(ST_ledgerLineWidth));
      setLen(Spatium(2.0));
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF LedgerLine::canvasPos() const
      {
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = chord()->measure()->system();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Chord::setMag(double val)
      {
      Element::setMag(val);
      foreach (LedgerLine* ll, _ledgerLines)
            ll->setMag(val);
      if (_stem)
            _stem->setMag(val);
      if (_hook)
            _hook->setMag(val);
      if (_stemSlash)
            _stemSlash->setMag(val);
      if (_arpeggio)
            _arpeggio->setMag(val);
      if (_tremolo)
            _tremolo->setMag(val);
      if (_glissando)
            _glissando->setMag(val);
      for (ciNote in = notes.begin(); in != notes.end(); ++in)
            in->second->setMag(val);
      }

//---------------------------------------------------------
//   layoutStem
//---------------------------------------------------------

/**
 Layout chord stem and hook.
*/

void Chord::layoutStem()
      {
      System* s = segment()->measure()->system();
      if (s == 0)       //DEBUG
            return;

      if (_stem) {
            Spatium stemLen;
            QPointF npos;

            int hookIdx      = duration().hooks();
            Note* upnote     = upNote();
            Note* downnote   = downNote();
            double ul        = upnote->line() * .5;
            double dl        = downnote->line() * .5;
            bool shortenStem = score()->styleB(ST_shortenStem);
            Spatium progression(score()->styleS(ST_shortStemProgression));
            Spatium shortest(score()->styleS(ST_shortestStem));

            double normalStemLen = 3.5;
            if (_noteType != NOTE_NORMAL)
                  normalStemLen *= score()->style(ST_graceNoteMag).toDouble();

            if (up()) {
                  double dy  = dl + downnote->stemYoff(true);
                  double sel = ul - normalStemLen;

                  if (shortenStem && (sel < 0.0) && (hookIdx == 0 || !downnote->mirror()))
                        sel -= sel  * progression.val();
                  if (sel > 2.0)
                        sel = 2.0;
                  npos       = downnote->stemPos(true);
                  stemLen    = Spatium(sel - dy);
                  if (-stemLen < shortest)
                        stemLen = -shortest;
                  }
            else {
                  double uy  = ul + upnote->stemYoff(false);
                  double sel = dl + normalStemLen;

                  if (shortenStem && (sel > 4.0) && (hookIdx == 0 || downnote->mirror()))
                        sel -= (sel - 4.0)  * progression.val();
                  if (sel < 2.0)
                        sel = 2.0;
                  npos       = upnote->stemPos(false);
                  stemLen    = Spatium(sel - uy);
                  if (stemLen < shortest)
                        stemLen = shortest;
                  }

            _stem->setLen(point(stemLen));
            _stem->setPos(npos);

            if (_stemSlash) {
                  // TODO: does not work for chords
                  double x = _stem->pos().x();
                  double y = _stem->pos().y();
                  double l = point(stemLen) * .5;
                  y += l;
                  double h2 = l * .5;
                  double w  = upnote->headWidth() * .7;
                  _stemSlash->setLine(QLineF(QPointF(x + w, y + h2), QPointF(x - w, y - h2)));
                  }

            if (hookIdx) {
                  if (!up())
                        hookIdx = -hookIdx;
                  _hook->setSubtype(hookIdx);
                  qreal lw  = point(score()->styleS(ST_stemWidth)) * .5;
                  QPointF p = npos + QPointF(lw, _stem->stemLen());
                  _hook->setPos(p);
                  }
            else
                  setHook(0);
            }
      else
            setHook(0);

      //-----------------------------------------
      //    process tremolo
      //-----------------------------------------

      if (_tremolo)
            _tremolo->layout();
      }

//---------------------------------------------------------
//   layout2
//    Called after horizontal positions of all elements
//    are fixed.
//---------------------------------------------------------

void Chord::layout2()
      {
      double _spatium = spatium();
      foreach(LedgerLine* h, _ledgerLines) {
            //
            // Experimental:
            //    look for colliding ledger lines
            //

            double y = h->y();
            double x = h->x();
            Spatium len(h->len());

            double minDist = _spatium * .2;
            bool found = false;
            double cx  = x + canvasPos().x();
            Segment* s = segment()->prev();
            if (s && s->subtype() == Segment::SegChordRest) {
                  int strack = staffIdx() * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        if (s->element(track)) {
                              Element* e = s->element(track);
                              if (e->type() == CHORD) {
                                    Chord* ch = static_cast<Chord*>(e);
                                    foreach(LedgerLine* ll, *ch->ledgerLines()) {
                                          if (ll->y() == y) {
                                                double d = cx - (ll->canvasPos().x() + ll->len().val()*_spatium) - minDist;
                                                if (d < 0.0) {
                                                      double shorten = -d;
                                                      x   += shorten;
                                                      len -= Spatium(shorten / _spatium);
                                                      ll->setLen(ll->len() - Spatium(shorten / _spatium));
                                                      }
                                                found = true;
                                                break;
                                                }
                                          }
                                    }
                              }
                        if (found)
                              break;
                        }
                  }
            if (found) {
                  h->setLen(len);
                  h->setPos(x, y);
                  }
            }
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Chord::space(double& min, double& extra) const
      {
      min   = minSpace;
      extra = extraSpace;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Chord::layout()
      {
      if (notes.empty())
            return;

      double _spatium  = spatium();

      if (!segment()) {
            //
            // hack for use in palette
            //
            for (iNote in = notes.begin(); in != notes.end(); ++in) {
                  Note* note = in->second;
                  note->layout();

                  double x = 0.0;
                  double y = note->line() * _spatium * .5;
                  note->setPos(x, y);
                  }
            return;
            }

      Note* upnote     = upNote();
      double headWidth = upnote->headWidth();

      //-----------------------------------------
      //  process notes
      //    - position
      //-----------------------------------------

      int minMove = 1;
      int maxMove = -1;

      double lx = 0.0;
      _dotPosX  = 0.0;
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* note = in->second;
            note->layout();

            double x = 0.0;

            int move = note->staffMove();
            if (move < minMove)
                  minMove = move;
            if (move > maxMove)
                  maxMove = move;

            double y = note->line() * _spatium * .5;

            bool stemUp = isUp();
            if (note->staffMove() == -1) {
                  stemUp = false;
                  }
            else if (note->staffMove() == 1) {
                  stemUp = true;
                  }
            if (note->mirror())
                  x += stemUp ? headWidth : - headWidth;

            note->setPos(x, y);
            double xx = x + headWidth;
            if (xx > _dotPosX)
                  _dotPosX = xx;

            Accidental* accidental = note->accidental();
            if (accidental)
                  x = accidental->x() * mag();
            if (x < lx)
                  lx = x;
            }

      //-----------------------------------------
      //  process ledger lines
      //-----------------------------------------

      foreach(const LedgerLine* l, _ledgerLines)
            delete l;
      _ledgerLines.clear();

      //---------------------------------------------------
      //    create ledger lines for notes moved to
      //    upper staff
      //---------------------------------------------------

      double x  = upnote->pos().x();
      if ((up() && !upnote->mirror()) || (!up() && upnote->mirror()))
            x += headWidth;

      addLedgerLines(x, -1);     // notes moved to upper staff
      addLedgerLines(x, 0);
      addLedgerLines(x, 1);      // notes moved to lower staff

      foreach(LedgerLine* l, _ledgerLines)
            l->layout();

      layoutArticulations();

      //-----------------------------------------
      //  Fingering
      //-----------------------------------------

#if 0 // TODO
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* note = in->second;
            QList<Text*>& fingering = note->fingering();
            double x = _spatium * 0.8 + note->headWidth();
            foreach(const Text* f, fingering) {
                  f->setPos(x, 0.0);
                  // TODO: x += _spatium;
                  // if we have two fingerings and move the first,
                  // the second will also change position because their
                  // position in this list changes
                  }
            }
#endif

      if (_arpeggio) {
            double distance   = score()->styleS(ST_ArpeggioNoteDistance).val() * _spatium;
            double headHeight = upnote->headHeight();
            _arpeggio->layout();
            lx -= _arpeggio->width() + distance;
            double y = upNote()->pos().y() - headHeight * .5;
            double h = downNote()->pos().y() - y;
            _arpeggio->setHeight(h);
            _arpeggio->setPos(lx, y);
            }

      extraSpace    = -lx + _extraLeadingSpace.val() * _spatium;
      double mirror = 0.0;
      double hw     = 0.0;
      minSpace      = 0.0;

      if (_glissando)
            extraSpace += _spatium * .5;

      for (ciNote i = notes.begin(); i != notes.end(); ++i) {
            Note* note = i->second;
            double lhw = note->headWidth();
            if (note->mirror() && up() && (mirror < lhw))  // note head on the right side of stem
                  mirror = lhw;
            else if (lhw > hw)
                  hw = lhw;
            }
      minSpace += mirror + hw + _extraTrailingSpace.val() * _spatium;
      if (up() && _hook)
            minSpace += _hook->width();
      extraSpace += point(_extraLeadingSpace);
      }



