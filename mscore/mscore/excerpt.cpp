//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "excerpt.h"
#include "score.h"
#include "part.h"
#include "xml.h"
#include "staff.h"
#include "box.h"
#include "scoreview.h"
#include "style.h"
#include "page.h"
#include "text.h"
#include "slur.h"
#include "al/sig.h"
#include "al/tempo.h"
#include "measure.h"
#include "rest.h"
#include "stafftype.h"
#include "tuplet.h"
#include "chord.h"
#include "note.h"
#include "lyrics.h"

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Excerpt::read(QDomElement e)
      {
      const QList<Part*>* pl = _score->parts();
      QString name;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag = e.tagName();
            if (tag == "name")
                  name = e.text();
            else if (tag == "title")
                  _title = e.text().trimmed();
            else if (tag == "part") {
                  int partIdx = e.text().toInt();
                  if (partIdx < 0 || partIdx >= pl->size())
                        printf("Excerpt::read: bad part index\n");
                  else
                        _parts.append(pl->at(partIdx));
                  }
            }
      if (_title.isEmpty())
            _title = name.trimmed();
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool Excerpt::operator!=(const Excerpt& e) const
      {
      if (e._score != _score)
            return true;
      if (e._title != _title)
            return true;
      if (e._parts != _parts)
            return true;
      return false;
      }

//---------------------------------------------------------
//   localSetScore
//---------------------------------------------------------

static void localSetScore(void* score, Element* element)
      {
      element->setScore((Score*)score);
      }

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

Score* createExcerpt(const QList<Part*>& parts)
      {
      if (parts.isEmpty())
            return 0;
      QList<int> srcStaves;

      Score* oscore = parts.front()->score();
      Score* score = new Score(oscore->style());
      score->setParentScore(oscore);
      foreach (Part* part, parts) {
            Part* p = new Part(score);
            p->setInstrument(*part->instr());
            int idx = 0;
            foreach(Staff* staff, *part->staves()) {
                  Staff* s = new Staff(score, p, idx);
                  s->setUpdateClefList(true);
                  s->setUpdateKeymap(true);
                  StaffType* st = staff->staffType();
                  if (st->modified())
                        st = new StaffType(*st);
                  s->setStaffType(staff->staffType());
                  int idx = score->staffTypes().indexOf(st);
                  if (idx == -1)
                        score->staffTypes().append(st);

                  s->linkTo(staff);
                  p->staves()->append(s);
                  score->staves().append(s);
                  srcStaves.append(oscore->staffIdx(staff));
                  ++idx;
                  }
            score->appendPart(p);
            }
      cloneStaves(oscore, score, srcStaves);

      //
      // create excerpt title
      //
      MeasureBase* measure = score->first();
      if (!measure || (measure->type() != VBOX)) {
            measure = new VBox(score);
            measure->setTick(0);
            score->addMeasure(measure);
            }
      Text* txt = new Text(score);
      txt->setSubtype(TEXT_INSTRUMENT_EXCERPT);
      txt->setTextStyle(TEXT_STYLE_INSTRUMENT_EXCERPT);
      txt->setText(parts.front()->longName()->getText());
      measure->add(txt);

      //
      // layout score
      //
      score->setPlaylistDirty(true);
      score->rebuildMidiMapping();
      score->updateChannel();

      score->setLayoutAll(true);
      score->addLayoutFlags(LAYOUT_FIX_TICKS | LAYOUT_FIX_PITCH_VELO);
      score->doLayout();
      return score;
      }

//---------------------------------------------------------
//   Tuplet2
//---------------------------------------------------------

struct Tuplet2 {
      Tuplet* o;
      Tuplet* n;
      Tuplet2(Tuplet* _o, Tuplet* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   TupletMap
//---------------------------------------------------------

class TupletMap {
      QList<Tuplet2> map;

   public:
      TupletMap() {}
      Tuplet* findNew(Tuplet* o);
      void add(Tuplet* _o, Tuplet* _n) { map.append(Tuplet2(_o, _n)); }
      };

//---------------------------------------------------------
//   findNew
//---------------------------------------------------------

Tuplet* TupletMap::findNew(Tuplet* o)
      {
      foreach(const Tuplet2& t2, map) {
            if (t2.o == o)
                  return t2.n;
            }
      return 0;
      }

//---------------------------------------------------------
//   Slur2
//---------------------------------------------------------

struct Slur2 {
      Slur* o;
      Slur* n;
      Slur2(Slur* _o, Slur* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   SlurMap
//---------------------------------------------------------

class SlurMap {
      QList<Slur2> map;

   public:
      SlurMap() {}
      Slur* findNew(Slur* o);
      void add(Slur* _o, Slur* _n) { map.append(Slur2(_o, _n)); }
      void check();
      };

//---------------------------------------------------------
//   findNew
//---------------------------------------------------------

Slur* SlurMap::findNew(Slur* o)
      {
      foreach(const Slur2& s2, map) {
            if (s2.o == o)
                  return s2.n;
            }
      return 0;
      }

//---------------------------------------------------------
//   check
//---------------------------------------------------------

void SlurMap::check()
      {
      foreach(const Slur2& s2, map) {
            Slur* slur = s2.n;
            if (slur->endElement() == 0) {
                  printf("slur end element missing %p new %p\n", s2.o, s2.n);
                  static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                  delete slur;
                  }
            }
      }

//---------------------------------------------------------
//   Element2
//---------------------------------------------------------

struct Element2 {
      Element* o;
      Element* n;
      Element2(Element* _o, Element* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   ElementMap
//---------------------------------------------------------

class ElementMap {
      QList<Element2> map;

   public:
      ElementMap() {}
      Element* findNew(Element* o) const;
      void add(Element* _o, Element* _n) { map.append(Element2(_o, _n)); }
      };

//---------------------------------------------------------
//   findNew
//---------------------------------------------------------

Element* ElementMap::findNew(Element* o) const
      {
      foreach(const Element2& s2, map) {
            if (s2.o == o)
                  return s2.n;
            }
      return 0;
      }

//---------------------------------------------------------
//   TieMap
//---------------------------------------------------------

class TieMap : public ElementMap {
   public:
      TieMap() {}
      Tie* findNew(Tie* o) const { return static_cast<Tie*>(ElementMap::findNew(o)); }
      void add(Tie* _o, Tie* _n) { ElementMap::add(_o, _n); }
      };

//---------------------------------------------------------
//   cloneStaves
//---------------------------------------------------------

void cloneStaves(Score* oscore, Score* score, const QList<int>& map)
      {
      int tracks = score->nstaves() * VOICES;
      SlurMap slurMap[tracks];
      TieMap tieMap[tracks];

      MeasureBaseList* nmbl = score->measures();
      for(MeasureBase* mb = oscore->measures()->first(); mb; mb = mb->next()) {
            MeasureBase* nmb = 0;
            if (mb->type() == HBOX)
                  nmb = new HBox(score);
            else if (mb->type() == VBOX)
                  nmb = new VBox(score);
            else if (mb->type() == MEASURE) {
                  Measure* m  = static_cast<Measure*>(mb);
                  Measure* nm = new Measure(score);
                  nmb = nm;
                  nm->setTick(m->tick());
                  nm->setLen(m->len());
                  nm->setTimesig(m->timesig());
                  nm->setRepeatCount(m->repeatCount());
                  nm->setRepeatFlags(m->repeatFlags());
                  nm->setIrregular(m->irregular());
                  nm->setNo(m->no());
                  nm->setNoOffset(m->noOffset());
                  nm->setBreakMultiMeasureRest(m->breakMultiMeasureRest());
                  nm->setEndBarLineType(
                     m->endBarLineType(),
                     m->endBarLineGenerated(),
                     m->endBarLineVisible(),
                     m->endBarLineColor());

                  Fraction ts = nm->len();
                  for (int track = 0; track < tracks; ++track) {
                        TupletMap tupletMap;    // tuplets cannot cross measure boundaries
                        int srcTrack = map[track/VOICES] * VOICES + (track % VOICES);
                        for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                              Element* oe = oseg->element(srcTrack);
                              if (oe == 0)
                                    continue;
                              Element* ne = oe->clone();
                              ne->setTrack(track);
                              ne->scanElements(score, localSetScore);
                              ne->setScore(score);
                              Segment* s = nm->getSegment(SegmentType(oseg->subtype()), oseg->tick());
                              if (oe->isChordRest()) {
                                    ChordRest* ocr = static_cast<ChordRest*>(oe);
                                    ChordRest* ncr = static_cast<ChordRest*>(ne);
                                    Tuplet* ot     = ocr->tuplet();
                                    if (ot) {
                                          Tuplet* nt = tupletMap.findNew(ot);
                                          if (nt == 0) {
                                                nt = new Tuplet(*ot);
                                                nt->clear();
                                                nt->setTrack(track);
                                                nt->setScore(score);
                                                nm->add(nt);
                                                tupletMap.add(ot, nt);
                                                }
                                          // nt->add(ncr);
                                          ncr->setTuplet(nt);
                                          }
                                    foreach(Slur* s, ocr->slurFor()) {
                                          Slur* slur = new Slur(score);
                                          slur->setStartElement(ncr);
                                          ncr->addSlurFor(slur);
                                          slurMap[track].add(s, slur);
                                          }
                                    foreach(Slur* s, ocr->slurBack()) {
                                          Slur* slur = slurMap[track].findNew(s);
                                          if (slur) {
                                                slur->setEndElement(ncr);
                                                ncr->addSlurBack(slur);
                                                }
                                          else {
                                                printf("cloneStave: cannot find slur\n");
                                                }
                                          }
                                    foreach(Element* e, oseg->annotations()) {
                                          if (e->generated())
                                                continue;
                                          // if ((e->track() != srcTrack) && !(e->systemFlag() && track == 0))
                                          if (e->track() != srcTrack)
                                                continue;
                                          Element* ne = e->clone();
                                          ne->setTrack(track);
                                          s->add(ne);
                                          }
                                    if ((track % VOICES) == 0) {
                                          int ostaffIdx = srcTrack / VOICES;
                                          Staff* nstaff = score->staff(track/VOICES);
                                          if (!nstaff->useTablature()) {
                                                const QList<Lyrics*>* ll = oseg->lyricsList(ostaffIdx);
                                                if (ll) {
                                                      foreach(Lyrics* ly, *ll) {
                                                            Lyrics* l = ly->clone();
                                                            l->setTrack(track);
                                                            s->add(l);
                                                            }
                                                      }
                                                }
                                          }

                                    if (oe->type() == CHORD) {
                                          Chord* och = static_cast<Chord*>(ocr);
                                          Chord* nch = static_cast<Chord*>(ncr);
                                          int n = och->notes().size();
                                          for (int i = 0; i < n; ++i) {
                                                Note* on = och->notes().at(i);
                                                Note* nn = nch->notes().at(i);
                                                if (on->tieFor()) {
                                                      Tie* tie = new Tie(score);
                                                      nn->setTieFor(tie);
                                                      tie->setStartNote(nn);
                                                      tieMap[track].add(on->tieFor(), tie);
                                                      }
                                                if (on->tieBack()) {
                                                      Tie* tie = tieMap[track].findNew(on->tieBack());
                                                      if (tie) {
                                                            nn->setTieBack(tie);
                                                            tie->setEndNote(nn);
                                                            }
                                                      else {
                                                            printf("cloneStave: cannot find tie\n");
                                                            }
                                                      }
                                                }
                                          }
                                    }
                              s->add(ne);
                              }
                        }
                  }
            foreach(Element* e, *mb->el()) {
                  Element* ne = e->clone();
                  ne->setScore(score);
                  nmb->add(ne);
                  }
            nmbl->add(nmb);
            }
      //DEBUG:
      for (int track = 0; track < tracks; ++track)
            slurMap[track].check();
      }

//---------------------------------------------------------
//   cloneStaff
//---------------------------------------------------------

void cloneStaff(Staff* srcStaff, Staff* dstStaff)
      {
      Score* score  = srcStaff->score();
      dstStaff->linkTo(srcStaff);

      int tracks = score->nstaves() * VOICES;
      SlurMap slurMap[tracks];
      TieMap tieMap[tracks];
      int srcStaffIdx = score->staffIdx(srcStaff);
      int dstStaffIdx = score->staffIdx(dstStaff);

      for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
            int sTrack = srcStaffIdx * VOICES;
            int eTrack = sTrack + VOICES;
            for (int srcTrack = sTrack; srcTrack < eTrack; ++srcTrack) {
                  TupletMap tupletMap;    // tuplets cannot cross measure boundaries
                  int dstTrack = dstStaffIdx * VOICES + (srcTrack - sTrack);
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        Element* oe = seg->element(srcTrack);
                        if (oe == 0)
                              continue;
                        Element* ne = oe->clone();
                        ne->setTrack(dstTrack);
                        seg->add(ne);
                        if (oe->isChordRest()) {
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              ChordRest* ncr = static_cast<ChordRest*>(ne);
                              Tuplet* ot     = ocr->tuplet();
                              if (ot) {
                                    Tuplet* nt = tupletMap.findNew(ot);
                                    if (nt == 0) {
                                          nt = new Tuplet(*ot);
                                          nt->clear();
                                          nt->setTrack(dstTrack);
                                          m->add(nt);
                                          tupletMap.add(ot, nt);
                                          }
                                    ncr->setTuplet(nt);
                                    }
                              foreach(Slur* s, ocr->slurFor()) {
                                    Slur* slur = new Slur(score);
                                    slur->setStartElement(ncr);
                                    ncr->addSlurFor(slur);
                                    slurMap[dstTrack].add(s, slur);
                                    }
                              foreach(Slur* s, ocr->slurBack()) {
                                    Slur* slur = slurMap[dstTrack].findNew(s);
                                    if (slur) {
                                          slur->setEndElement(ncr);
                                          ncr->addSlurBack(slur);
                                          }
                                    else {
                                          printf("cloneStave: cannot find slur\n");
                                          }
                                    }
                              foreach(Element* e, seg->annotations()) {
                                    if (e->generated() || e->systemFlag())
                                          continue;
                                    if (e->track() != srcTrack)
                                          continue;
                                    Element* ne = e->clone();
                                    ne->setTrack(dstTrack);
                                    seg->add(ne);
                                    }
                              if (oe->type() == CHORD) {
                                    Chord* och = static_cast<Chord*>(ocr);
                                    Chord* nch = static_cast<Chord*>(ncr);
                                    int n = och->notes().size();
                                    for (int i = 0; i < n; ++i) {
                                          Note* on = och->notes().at(i);
                                          Note* nn = nch->notes().at(i);
                                          if (on->tieFor()) {
                                                Tie* tie = new Tie(score);
                                                nn->setTieFor(tie);
                                                tie->setStartNote(nn);
                                                tieMap[dstTrack].add(on->tieFor(), tie);
                                                }
                                          if (on->tieBack()) {
                                                Tie* tie = tieMap[dstTrack].findNew(on->tieBack());
                                                if (tie) {
                                                      nn->setTieBack(tie);
                                                      tie->setEndNote(nn);
                                                      }
                                                else {
                                                      printf("cloneStave: cannot find tie\n");
                                                      }
                                                }
                                          }
                                    }
                              }
                        }
                  }
//            foreach(Element* e, *m->el()) {
//                  Element* ne = e->clone();
//                  ne->setScore(score);
//                  nmb->add(ne);
//                  }
            }
      //DEBUG:
//      for (int track = 0; track < tracks; ++track)
//            slurMap[track].check();
      }

