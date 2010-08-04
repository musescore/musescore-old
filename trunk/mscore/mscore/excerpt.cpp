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

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Excerpt::read(QDomElement e)
      {
#if 0
      const QList<Part*>* pl = score->parts();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag = e.tagName();
            if (tag == "name")
                  _name = e.text();
            else if (tag == "title")
                  _title = e.text();
            else if (tag == "part") {
                  int partIdx = e.text().toInt();
                  if (partIdx < 0 || partIdx >= pl->size())
                        printf("Excerpt::read: bad part index\n");
                  else
                        _parts.append(pl->at(partIdx));
                  }
            }
#endif
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Excerpt::write(Xml& xml) const
      {
#if 0
      xml.stag("Excerpt");
      xml.tag("name", _name);
      xml.tag("title", _title);
      const QList<Part*>* pl = score->parts();
      foreach(Part* const part, _parts)
            xml.tag("part", pl->indexOf(part));
      xml.etag();
#endif
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

#if 0
//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

Score* Score::createExcerpt(Excerpt* excerpt)
      {
      Score* s      = new Score(_style);
      QFileInfo* fi = s->fileInfo();
      QString name  = fileInfo()->path() + "/" + excerpt->name() + ".msc";
      fi->setFile(name);

      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.excerptmode = true;
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      writeExcerpt(excerpt, xml);
      xml << "</museScore>\n";
      buffer.close();
#if 0
      QFile mops("mops");
      mops.open(QIODevice::WriteOnly);
      mops.write(buffer.data());
      mops.close();
#endif
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(buffer.data(), &err, &line, &column)) {
            printf("error reading excerpt data at %d/%d\n<%s>\n",
            line, column, err.toLatin1().data());
            return 0;
            }
      docName = "--";
      QDomElement e = doc.documentElement();
      s->read(e);
      if (!excerpt->title().isEmpty()) {
            MeasureBase* measure = s->first();
            if (!measure || (measure->type() != VBOX)) {
                  measure = new VBox(s);
                  measure->setTick(0);
                  s->addMeasure(measure);
                  }
            Text* txt = new Text(s);
            txt->setSubtype(TEXT_INSTRUMENT_EXCERPT);
            txt->setTextStyle(TEXT_STYLE_INSTRUMENT_EXCERPT);
            txt->setText(excerpt->title());
            measure->add(txt);
            }

      if ((s->parts()->size() == 1)) {
             if (!excerpt->title().isEmpty()) {
                  // remove long instrument name and replace with 10 spaces
                  s->parts()->front()->setLongName(QString("          "));
                  }
            s->parts()->front()->setShortName("");
            }
      //
      // remove all brackets with span <= 1
      //
      foreach (Staff* staff, s->staves()) {
            staff->cleanupBrackets();
            }

      s->renumberMeasures();
      s->setCreated(true);
      s->rebuildMidiMapping();
      s->updateChannel();
      s->addLayoutFlag(LAYOUT_FIX_PITCH_VELO);
      s->doLayout();
      return s;
      }

//---------------------------------------------------------
//   writeExcerpt
//---------------------------------------------------------

void Score::writeExcerpt(Excerpt* excerpt, Xml& xml)
      {
      xml.tag("Division", AL::division);
      xml.tag("Spatium", _spatium / DPMM);
      xml.curTrack  = -1;
      xml.trackDiff = 0;

      _style.save(xml, true);
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (*_textStyles[i] != defaultTextStyleArray[i])
                  _textStyles[i]->write(xml);
            }
      xml.tag("showInvisible", _showInvisible);
      xml.tag("showFrames", _showFrames);
      pageFormat()->write(xml);
      if (rights) {
            xml.stag("rights");
            xml.writeHtml(rights->getHtml());
            xml.etag();
            }
      if (!_movementNumber.isEmpty())
            xml.tag("movement-number", _movementNumber);
      if (!_movementTitle.isEmpty())
            xml.tag("movement-title", _movementTitle);

//      sigmap()->write(xml);
      tempomap()->write(xml);
      foreach(const Part* part, _parts) {
            int idx = excerpt->parts()->indexOf((Part*)part);
            if (idx != -1)
                  part->write(xml);
            }
      int trackOffset[_staves.size()];
      int idx = 0, didx = 0;

      static const int HIDDEN = 100000;

      foreach(Staff* staff, _staves) {
            int i = excerpt->parts()->indexOf(staff->part());
            if (i == -1) {
                  trackOffset[idx] = HIDDEN;
                  ++idx;
                  continue;
                  }
            trackOffset[idx] = (didx - idx) * VOICES;
            ++idx;
            ++didx;
            }

      // to serialize slurs, they get an id; this id is referenced
      // in begin-end elements
      int slurId = 0;

      xml.trackDiff = 0;
      foreach(Element* el, _gel) {
            int staffIdx = el->staffIdx();
            if (el->type() == SLUR) {
                  Slur* slur = static_cast<Slur*>(el);
                  int staffIdx1 = slur->startElement()->staffIdx();
                  int staffIdx2 = slur->endElement()->staffIdx();
                  if (trackOffset[staffIdx1] == HIDDEN || trackOffset[staffIdx2] == HIDDEN)
                        continue;
                  xml.trackDiff = staffIdx; // slur staffIdx is always zero
                  slur->setId(slurId++);
                  }
            else {
                  if (el->track() != -1) {
//                        if (el->type() != VOLTA) {                      // HACK
                              if (trackOffset[staffIdx] == HIDDEN)
                                    continue;
                              xml.trackDiff = trackOffset[staffIdx];
//                              }
                        }
                  }
            el->write(xml);
            }
      xml.trackDiff = 0;
      bool isFirstStaff = true;
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Part* part = staff(staffIdx)->part();
            int idx = excerpt->parts()->indexOf(part);
            if (idx == -1)
                  continue;
            xml.curTick   = 0;
            xml.curTrack  = staffIdx * VOICES;
            xml.trackDiff = trackOffset[staffIdx];
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1 + xml.trackDiff/4));
            for (MeasureBase* m = first(); m; m = m->next()) {
                  if (isFirstStaff || m->type() == MEASURE)
                        m->write(xml, staffIdx, isFirstStaff);
                  if (m->type() == MEASURE)
                        xml.curTick = m->tick() + m->ticks();
                  }
            xml.etag();
            isFirstStaff = false;
            }
      xml.tag("cursorTrack", _is.track());
      }
#endif

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
                  printf("staff type %s\n", qPrintable(st->name()));

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
      score->addLayoutFlag(LAYOUT_FIX_TICKS);
      score->addLayoutFlag(LAYOUT_FIX_PITCH_VELO);
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

