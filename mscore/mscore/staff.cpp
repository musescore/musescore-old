//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: staff.cpp,v 1.11 2006/03/28 14:58:58 wschweer Exp $
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

#include "globals.h"
#include "staff.h"
#include "part.h"
#include "clef.h"
#include "xml.h"
#include "score.h"
#include "bracket.h"
#include "key.h"
#include "segment.h"

//---------------------------------------------------------
//   isTopSplit
//---------------------------------------------------------

bool Staff::isTopSplit() const
      {
      return _part->nstaves() > 1 && isTop();
      }

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

QString Staff::trackName() const
      {
      return _part->trackName();
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

const QTextDocument& Staff::longName() const
      {
      return _part->longName();
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

const QTextDocument& Staff::shortName() const
      {
      return _part->shortName();
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Staff::midiChannel() const
      {
      return _part->midiChannel();
      }

//---------------------------------------------------------
//   midiProgram
//---------------------------------------------------------

int Staff::midiProgram() const
      {
      return _part->midiProgram();
      }

//---------------------------------------------------------
//   volume
//---------------------------------------------------------

int Staff::volume() const
      {
      return _part->volume();
      }

//---------------------------------------------------------
//   reverb
//---------------------------------------------------------

int Staff::reverb() const
      {
      return _part->reverb();
      }

//---------------------------------------------------------
//   chorus
//---------------------------------------------------------

int Staff::chorus() const
      {
      return _part->chorus();
      }

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Score* s, Part* p, int rs)
      {
      _score  = s;
      _rstaff = rs;
      _part   = p;
      _clef   = new ClefList;
      _bracket = NO_BRACKET;
      _bracketSpan = 0;
      _keymap = new KeyList;
      (*_keymap)[0] = 0;
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      delete _clef;
      delete _keymap;
      }

//---------------------------------------------------------
//   Staff::key
//---------------------------------------------------------

int Staff::key(int tick) const
      {
      return _clef->clef(tick);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Staff::write(Xml& xml) const
      {
      xml.stag("Staff");
      _clef->write(xml, "cleflist");
      _keymap->write(xml, "keylist");
      if (_bracket != NO_BRACKET) {
            xml.tagE("bracket type=\"%d\" span=\"%d\"", _bracket, _bracketSpan);
            }
      xml.etag("Staff");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "cleflist")
                  _clef->read(node, _score);
            else if (tag == "keylist")
                  _keymap->read(node, _score);
            else if (tag == "bracket") {
                  _bracket = e.attribute("type", "-1").toInt();
                  _bracketSpan = e.attribute("span", "0").toInt();
                  }
            else
                  printf("Mscore:Staff: unknown tag %s\n", tag.toLatin1().data());
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void StaffList::remove(Staff* p)
      {
      if (removeAll(p) < 1)
            printf("StaffList::remove(%p): not found\n", p);
      }

//---------------------------------------------------------
//   changeKeySig
//
// change key signature at tick into subtype st for all staves
// in response to gui command (drop keysig on measure or keysig)
//
// FIXME: redo does not work
//---------------------------------------------------------

void Staff::changeKeySig(int tick, int st)
      {
      int ot = _keymap->key(tick);
// printf("changeKeySig %p tick %d st %d ot %d\n",
//         this, tick, st, ot);
      if (ot == st)
            return;                 // no change

      int oval = -1000;
      iKeyEvent ki = _keymap->find(tick);
      if (ki != _keymap->end())
            oval = ki->second;

      _score->undoOp(UndoOp::ChangeKeySig, this, tick, oval, st);
      (*_keymap)[tick] = st;

      Measure* m = _score->tick2measure(tick);
      if (!m) {
            printf("measure for tick %d not found!\n", tick);
            return;
            }

      //---------------------------------------------
      //    if the next keysig has the same subtype
      //    then its unnecessary and must be removed
      //---------------------------------------------

      for (; m; m = m->next()) {
            bool found = false;
            for (Segment* segment = m->first(); segment; segment = segment->next()) {
                  if (segment->segmentType() == Segment::SegKeySig) {
                        //
                        // we assume keySigs are only in first track (voice 0)
                        //
                        int track = idx() * VOICES;
                        KeySig* e = (KeySig*)segment->element(track);
                        if (e) {
                              int etick = segment->tick();
                              if (etick >= tick && e->subtype() == st) {
                                    _score->undoOp(UndoOp::RemoveElement, e);
                                    (*segment->elist())[track] = 0;
                                    m->cmdRemoveEmptySegment(segment);

                                    if (etick > tick) {
                                          _score->undoOp(UndoOp::ChangeKeySig, this, etick, st, -1000);
                                          iKeyEvent ik = _keymap->find(etick);
                                          if (ik == _keymap->end()) {
                                                printf("   NOT FOUND tick %d\n", etick);
                                                abort();
                                                }
                                          _keymap->erase(ik);
                                          }
                                    }
                              found = true;
                              break;
                              }
                        }
                  }
            if (found)
                  break;
            }

      //---------------------------------------------
      // insert new keysig symbols
      //---------------------------------------------

      m = _score->tick2measure(tick);
      KeySig* keysig = new KeySig(_score);
      keysig->setStaff(this);
      keysig->setTick(tick);
      keysig->setSubtype(st);

      Segment::SegmentType stype = Segment::segmentType(KEYSIG);
      Segment* s = m->findSegment(stype, tick);
      if (!s) {
            s = m->createSegment(stype, tick);
            _score->undoOp(UndoOp::AddElement, s);
            }
      keysig->setParent(s);
      _score->undoOp(UndoOp::AddElement, keysig);
      _score->layout();
      }

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return _score->staff(this);
      }

