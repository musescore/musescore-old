//=============================================================================
//  MusE Score
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
 MusicXML export.
 */

// TODO: trill lines need to be handled the same way as slurs
// in MuseScore they are measure level elements, while in MusicXML
// they are attached to notes (as ornaments)

//=========================================================
//  LVI FIXME
//
//  Evaluate paramenter handling between the various classes, could be simplified
//=========================================================

#include <math.h>
#include "config.h"
#include "mscore.h"
#include "file.h"
#include "score.h"
#include "rest.h"
#include "chord.h"
#include "al/sig.h"
#include "key.h"
#include "clef.h"
#include "note.h"
#include "segment.h"
#include "xml.h"
#include "beam.h"
#include "staff.h"
#include "part.h"
#include "measure.h"
#include "style.h"
#include "musicxml.h"
#include "slur.h"
#include "hairpin.h"
#include "dynamics.h"
#include "barline.h"
#include "timesig.h"
#include "ottava.h"
#include "pedal.h"
#include "text.h"
#include "tuplet.h"
#include "lyrics.h"
#include "volta.h"
#include "keysig.h"
#include "bracket.h"
#include "arpeggio.h"
#include "repeat.h"
#include "tremolo.h"
#include "trill.h"
#include "zip.h"
#include "harmony.h"
#include "tempotext.h"
#include "sym.h"
#include "pitchspelling.h"
#include "utils.h"
#include "articulation.h"
#include "page.h"
#include "system.h"
#include "element.h"
#include "glissando.h"
#include "navigate.h"
#include "drumset.h"
#include "preferences.h"

static bool isTwoNoteTremolo(Chord* chord);

//---------------------------------------------------------
//   attributes -- prints <attributes> tag when necessary
//---------------------------------------------------------

class Attributes {
      bool inAttributes;

   public:
      Attributes() { start(); }
      void doAttr(Xml& xml, bool attr);
      void start();
      void stop(Xml& xml);
      };

//---------------------------------------------------------
//   doAttr - when necessary change state and print <attributes> tag
//---------------------------------------------------------

void Attributes::doAttr(Xml& xml, bool attr)
      {
      if (!inAttributes && attr) {
            xml.stag("attributes");
            inAttributes = true;
            }
      else if (inAttributes && !attr) {
            xml.etag();
            inAttributes = false;
            }
      }

//---------------------------------------------------------
//   start -- initialize
//---------------------------------------------------------

void Attributes::start()
      {
      inAttributes = false;
      }

//---------------------------------------------------------
//   stop -- print </attributes> tag when necessary
//---------------------------------------------------------

void Attributes::stop(Xml& xml)
      {
      if (inAttributes) {
            xml.etag();
            inAttributes = false;
            }
      }

//---------------------------------------------------------
//   notations -- prints <notations> tag when necessary
//---------------------------------------------------------

class Notations {
      bool notationsPrinted;

   public:
      Notations() { notationsPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   articulations -- prints <articulations> tag when necessary
//---------------------------------------------------------

class Articulations {
      bool articulationsPrinted;

   public:
      Articulations() { articulationsPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   ornaments -- prints <ornaments> tag when necessary
//---------------------------------------------------------

class Ornaments {
      bool ornamentsPrinted;

   public:
      Ornaments() { ornamentsPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   technical -- prints <technical> tag when necessary
//---------------------------------------------------------

class Technical {
      bool technicalPrinted;

   public:
      Technical() { technicalPrinted = false; }
      void tag(Xml& xml);
      void etag(Xml& xml);
      };

//---------------------------------------------------------
//   slur handler -- prints <slur> tags
//---------------------------------------------------------

class SlurHandler {
      const Slur* slur[MAX_NUMBER_LEVEL];
      bool started[MAX_NUMBER_LEVEL];
      int findSlur(const Slur* s) const;

   public:
      SlurHandler();
      void doSlurStart(Chord* chord, Notations& notations, Xml& xml);
      void doSlurStop(Chord* chord, Notations& notations, Xml& xml);
      };

//---------------------------------------------------------
//   glissando handler -- prints <glissando> tags
//---------------------------------------------------------

class GlissandoHandler {
      const Chord* glissChrd[MAX_NUMBER_LEVEL];
      const Chord* slideChrd[MAX_NUMBER_LEVEL];
      int findChord(const Chord* c, int st) const;

   public:
      GlissandoHandler();
      void doGlissandoStart(Chord* chord, Notations& notations, Xml& xml);
      void doGlissandoStop(Chord* chord, Notations& notations, Xml& xml);
      };

//---------------------------------------------------------
//   ExportMusicXml
//---------------------------------------------------------

class ExportMusicXml {
      Score* score;
      Xml xml;
      SlurHandler sh;
      GlissandoHandler gh;
      int tick;
      Attributes attr;
      TextLine* bracket[MAX_BRACKETS];
      int div;
      double millimeters;
      int tenths;

      int findBracket(const TextLine* tl) const;
      void chord(Chord* chord, int staff, const LyricsList* ll, bool useDrumset);
      void rest(Rest* chord, int staff);
      void clef(int staff, int clef);
      void timesig(TimeSig* tsig);
      void keysig(int key, bool visible = true);
      void barlineLeft(Measure* m);
      void barlineRight(Measure* m);
      void pitch2xml(Note* note, char& c, int& alter, int& octave);
      void unpitch2xml(Note* note, char& c, int& octave);
      void lyrics(const LyricsList* ll, const int trk);
      void work(const MeasureBase* measure);
      void calcDivMoveToTick(int t);
      void calcDivisions();
      double getTenthsFromInches(double);
      double getTenthsFromDots(double);

   public:
      ExportMusicXml(Score* s) { score = s; tick = 0; div = 1; tenths = 40;
            millimeters = score->spatium() * tenths / (10 * DPMM);}
      void write(QIODevice* dev);
      void credits(Xml& xml);
      void moveToTick(int t);
      void words(Text* text, int staff);
      void hairpin(Hairpin* hp, int staff, int tick);
      void ottava(Ottava* ot, int staff, int tick);
      void pedal(Pedal* pd, int staff, int tick);
      void textLine(TextLine* tl, int staff, int tick);
      void dynamic(Dynamic* dyn, int staff);
      void symbol(Symbol * sym, int staff);
      void tempoText(TempoText* text, int staff);
      void harmony(Harmony*, Element*);
      };

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Notations::tag(Xml& xml)
      {
      if (!notationsPrinted)
            xml.stag("notations");
      notationsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Notations::etag(Xml& xml)
      {
      if (notationsPrinted)
            xml.etag();
      notationsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Articulations::tag(Xml& xml)
      {
      if (!articulationsPrinted)
            xml.stag("articulations");
      articulationsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Articulations::etag(Xml& xml)
      {
      if (articulationsPrinted)
            xml.etag();
      articulationsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Ornaments::tag(Xml& xml)
      {
      if (!ornamentsPrinted)
            xml.stag("ornaments");
      ornamentsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Ornaments::etag(Xml& xml)
      {
      if (ornamentsPrinted)
            xml.etag();
      ornamentsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Technical::tag(Xml& xml)
      {
      if (!technicalPrinted)
            xml.stag("technical");
      technicalPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Technical::etag(Xml& xml)
      {
      if (technicalPrinted)
            xml.etag();
      technicalPrinted = false;
      }

//---------------------------------------------------------
//   slurHandler
//---------------------------------------------------------

SlurHandler::SlurHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            slur[i] = 0;
            started[i] = false;
            }
      }

//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//---------------------------------------------------------

int SlurHandler::findSlur(const Slur* s) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (slur[i] == s) return i;
      return -1;
      }

//---------------------------------------------------------
//   doSlurStart
//---------------------------------------------------------

void SlurHandler::doSlurStart(Chord* chord, Notations& notations, Xml& xml)
      {
      // search for slur(s) starting at this chord
      foreach(const Slur* s, chord->slurFor()) {
            // check if on slur list (i.e. stop already seen)
            int i = findSlur(s);
            if (i >= 0) {
                  // remove from list and print start
                  slur[i] = 0;
                  started[i] = false;
                  notations.tag(xml);

                  xml.tagE("slur type=\"start\"%s number=\"%d\"", s->slurDirection() == UP ? " placement=\"above\"" : "", i + 1);
                  }
            else {
                  // find free slot to store it
                  i = findSlur(0);
                  if (i >= 0) {
                        slur[i] = s;
                        started[i] = true;
                        notations.tag(xml);
                        xml.tagE("slur type=\"start\" number=\"%d\"", i + 1);
                        }
                  else
                        printf("no free slur slot");
                  }
            }
      }

//---------------------------------------------------------
//   doSlurStop
//---------------------------------------------------------

// Note: a slur may start in a higher voice in the same measure.
// In that case it is not yet started (i.e. on the active slur list)
// when doSlurStop() is executed. Handle this slur as follows:
// - generate stop anyway and put it on the slur list
// - doSlurStart() starts slur but doesn't store it

void SlurHandler::doSlurStop(Chord* chord, Notations& notations, Xml& xml)
      {
      // search for slur(s) stopping at this chord but not on slur list yet
      foreach(const Slur* s, chord->slurBack()) {
            // check if on slur list
            int i = findSlur(s);
            if (i < 0) {
                  // if not, find free slot to store it
                  i = findSlur(0);
                  if (i >= 0) {
                        slur[i] = s;
                        started[i] = false;
                        notations.tag(xml);
                        xml.tagE("slur type=\"stop\" number=\"%d\"", i + 1);
                        }
                  else
                        printf("no free slur slot");
                  }
            }
      // search slur list for already started slur(s) stopping at this chord
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (slur[i] && slur[i]->endElement() == chord) {
                  if (started[i]) {
                        slur[i] = 0;
                        started[i] = false;
                        notations.tag(xml);
                        xml.tagE("slur type=\"stop\" number=\"%d\"", i + 1);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   glissando
//---------------------------------------------------------

// <notations>
//   <slide line-type="solid" number="1" type="start"/>
//   </notations>

// <notations>
//   <glissando line-type="wavy" number="1" type="start"/>
//   </notations>

static void glissando(Glissando * gli, int number, bool start, Notations& notations, Xml& xml)
      {
      int st = gli->subtype();
      switch (st) {
            case 0:
                  notations.tag(xml);
                  xml.tagE("slide line-type=\"solid\" number=\"%d\" type=\"%s\"",
                           number, start ? "start" : "stop");
                  break;
            case 1:
                  notations.tag(xml);
                  xml.tagE("glissando line-type=\"wavy\" number=\"%d\" type=\"%s\"",
                           number, start ? "start" : "stop");
                  break;
            default:
                  printf("unknown glissando subtype %d\n", st);
                  break;
            }
      }

//---------------------------------------------------------
//   GlissandoHandler
//---------------------------------------------------------

GlissandoHandler::GlissandoHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            glissChrd[i] = 0;
            slideChrd[i] = 0;
            }
      }

//---------------------------------------------------------
//   findChord -- get index of chord in chord table for subtype st
//   return -1 if not found
//---------------------------------------------------------

int GlissandoHandler::findChord(const Chord* c, int st) const
      {
      if (st != 0 && st != 1) {
            printf("GlissandoHandler::findChord: unknown glissando subtype %d\n", st);
            return -1;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (st == 0 && slideChrd[i] == c) return i;
            if (st == 1 && glissChrd[i] == c) return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   doGlissandoStart
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStart(Chord* chord, Notations& notations, Xml& xml)
      {
      int st = chord->glissando()->subtype();
      if (st != 0 && st != 1) {
            printf("doGlissandoStart: unknown glissando subtype %d\n", st);
            return;
            }
      // check if on chord list
      int i = findChord(chord, st);
      if (i >= 0) {
            // print error and remove from list
            printf("doGlissandoStart: chord %p already on list\n", chord);
            if (st == 0) slideChrd[i] = 0;
            if (st == 1) glissChrd[i] = 0;
            }
      // find free slot to store it
      i = findChord(0, st);
      if (i >= 0) {
            if (st == 0) slideChrd[i] = chord;
            if (st == 1) glissChrd[i] = chord;
            glissando(chord->glissando(), i + 1, true, notations, xml);
            }
      else
            printf("doGlissandoStart: no free slot");
      }

//---------------------------------------------------------
//   doGlissandoStop
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStop(Chord* chord, Notations& notations, Xml& xml)
      {
      int st = chord->glissando()->subtype();
      if (st != 0 && st != 1) {
            printf("doGlissandoStop: unknown glissando subtype %d\n", st);
            return;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (st == 0 && slideChrd[i] == chord) {
                  slideChrd[i] = 0;
                  glissando(chord->glissando(), i + 1, false, notations, xml);
                  return;
                  }
            if (st == 1 && glissChrd[i] == chord) {
                  glissChrd[i] = 0;
                  glissando(chord->glissando(), i + 1, false, notations, xml);
                  return;
                  }
            }
      printf("doGlissandoStop: glissando chord %p not found\n", chord);
      }

//---------------------------------------------------------
//   directions anchor -- anchor directions at another element or a specific tick
//---------------------------------------------------------

class DirectionsAnchor {
      Element* direct;        // the element containing the direction
      Element* anchor;        // the element it is attached to
      bool     start;         // whether it is attached to start or end
      int      tick;          // the timestamp

   public:
      DirectionsAnchor(Element* a, bool s, int t) { direct = 0; anchor = a; start = s; tick = t; }
      DirectionsAnchor(int t)                     { direct = 0; anchor = 0; start = true; tick = t; }
      Element* getDirect()                        { return direct; }
      Element* getAnchor()                        { return anchor; }
      bool getStart()                             { return start; }
      int getTick()                               { return tick; }
      void setDirect(Element* d)                  { direct = d; }
      };

//---------------------------------------------------------
//   directions handler -- builds list of directives (measure relative elements)
//     associated with elements in segments to enable writing at the correct position
//     in the output stream
//     Note that runtime behaviour is essentially O(n2), as it requires searching
//     through all elements for directions, for every directions followed by searching
//     through all elements for an element with matching start or end time
//     To minimize this, limit the search to the current measure where possible
//---------------------------------------------------------

class DirectionsHandler {
      Score *cs;
      int nextAnchor;
      QList<DirectionsAnchor*> anchors;
      void storeAnchor(DirectionsAnchor* a);

   public:
      DirectionsHandler(Score* s);
      void buildDirectionsList(Part* p, int strack, int etrack);
      void buildDirectionsList(Measure* m, bool dopart, Part* p, int strack, int etrack);
      void handleElement(ExportMusicXml* exp, Element* el, int sstaff, bool start);
      void handleElements(ExportMusicXml* exp, Staff* staff, int mstart, int mend, int sstaff);
      };

//---------------------------------------------------------
//   DirectionsHandler
//---------------------------------------------------------

DirectionsHandler::DirectionsHandler(Score* s)
      {
      cs = s;
      nextAnchor = 0;
//      for (int i = 0; i < MAX_ANCHORS; i++) anchors[i] = 0;
      }

//---------------------------------------------------------
//   storeAnchor
//---------------------------------------------------------

void DirectionsHandler::storeAnchor(DirectionsAnchor* a)
      {
      anchors.append(a);
/*      if (nextAnchor < MAX_ANCHORS)
            anchors[nextAnchor++] = a;
      else
            printf("DirectionsHandler: too many directions\n");
 */
      }

//---------------------------------------------------------
//   handleElement -- handle all directions attached to a specific element
//---------------------------------------------------------

void DirectionsHandler::handleElement(ExportMusicXml* exp, Element* el, int sstaff, bool start)
      {
      int i = 0;
      foreach(DirectionsAnchor* da, anchors) {
            if (da == 0) {
                  ++i;
                  continue;
                  }
            if (da->getAnchor() && da->getAnchor() == el && da->getStart() == start) {
                  Element* dir = da->getDirect();
                  switch(dir->type()) {
                        case SYMBOL:
                              exp->symbol((Symbol *) dir, sstaff);
                              break;
                        case TEMPO_TEXT:
                              exp->tempoText((TempoText*) dir, sstaff);
                              break;
                        case STAFF_TEXT:
                        case TEXT:
                              exp->words((Text*) dir, sstaff);
                              break;
                        case DYNAMIC:
                              exp->dynamic((Dynamic*) dir, sstaff);
                              break;
                        case HAIRPIN:
                              exp->hairpin((Hairpin*) dir, sstaff, da->getTick());
                              break;
                        case OTTAVA:
                              exp->ottava((Ottava*) dir, sstaff, da->getTick());
                              break;
                        case PEDAL:
                              exp->pedal((Pedal*) dir, sstaff, da->getTick());
                              break;
                        case TEXTLINE:
                              exp->textLine((TextLine*) dir, sstaff, da->getTick());
                              break;
                        default:
                              printf("DirectionsHandler::handleElement: direction type %s at tick %d not implemented\n",
                                      Element::name(dir->type()), da->getTick());
                              break;
                        }
                  delete da;
                  anchors[i] = 0;
                  }
            ++i;
            }
      }

//---------------------------------------------------------
//   handleElements -- handle all directions at tick between mstart and mend
//---------------------------------------------------------

void DirectionsHandler::handleElements(ExportMusicXml* /*exp*/, Staff* staff, int mstart, int mend, int sstaff)
      {
      int i = 0;
      foreach(DirectionsAnchor* da, anchors) {
            if (da == 0) {
                  ++i;
                  continue;
                  }
            Element* dir = da->getDirect();
            if (dir && dir->staff() == staff && mstart <= da->getTick() && da->getTick() < mend) {
                  // disabled, not sure if this behaviour is OK
                  // it generates backups/forwards to somewhere
                  // in the middle of notes and rests
                  // note: when re-enabling calcDivisions() also needs
                  // to take these backups/forwards into account
                  /*
                  exp->moveToTick(da->getTick());
                  switch(dir->type()) {
                        case SYMBOL:
                              exp->symbol((Symbol *) dir, sstaff);
                              break;
                        case TEMPO_TEXT:
                              exp->tempoText((TempoText*) dir, sstaff);
                              break;
                        case STAFF_TEXT:
                        case TEXT:
                              exp->words((Text*) dir, sstaff);
                              break;
                        case DYNAMIC:
                              exp->dynamic((Dynamic*) dir, sstaff);
                              break;
                        case HAIRPIN:
                              exp->hairpin((Hairpin*) dir, sstaff, da->getTick());
                              break;
                        case OTTAVA:
                              exp->ottava((Ottava*) dir, sstaff, da->getTick());
                              break;
                        case PEDAL:
                              exp->pedal((Pedal*) dir, sstaff, da->getTick());
                              break;
                        case TEXTLINE:
                              exp->textLine((TextLine*) dir, sstaff, da->getTick());
                              break;
                        default:
                              printf("DirectionsHandler::handleElements: direction type %s at tick %d not implemented\n",
                                      Element::name(dir->type()), da->getTick());
                              break;
                        }
                  */
                  // print warning instead
                  printf("can't export %s in staff %d at tick %d, no note or rest to attach to\n",
                         dir->name(), sstaff, da->getTick());
                  delete da;
                  anchors[i] = 0;
                  }
            ++i;
            }
      }

//---------------------------------------------------------
//   findSpecificMatchInMeasure -- find chord or rest in measure
//     starting or ending at tick
//---------------------------------------------------------

static DirectionsAnchor* findSpecificMatchInMeasure(int tick, Staff* stf, bool start, Measure* m, int strack, int etrack)
      {
      for (int st = strack; st < etrack; ++st) {
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  Element* el = seg->element(st);
                  if (!el)
                        continue;
                  if (el->isChordRest() && el->staff() == stf) {
                        ChordRest* cr = static_cast<ChordRest*>(el);
                        if (   ( start && el->tick() == tick)
                            || (!start && (el->tick() + cr->tickLen()) == tick)) {
                              return new DirectionsAnchor(el, start, tick);
                              }
                        }
                  }
            }
            return 0;
      }

//---------------------------------------------------------
//   findMatchInMeasure -- find chord or rest in measure
//---------------------------------------------------------

static DirectionsAnchor* findMatchInMeasure(int tick, Staff* st, Measure* m, Part* p, int strack, int etrack)
      {
      DirectionsAnchor* da;
      da = findSpecificMatchInMeasure(tick, st, true, m, strack, etrack);
      if (da)
            return da;
      da = findSpecificMatchInMeasure(tick, st, false, m, strack, etrack);
      if (da)
            return da;
      if (st && st->part() == p)
            return new DirectionsAnchor(tick);
      else return 0;
      }

//---------------------------------------------------------
//   findSpecificMatchInPart -- find chord or rest in part
//     starting or ending at tick
//---------------------------------------------------------

static DirectionsAnchor* findSpecificMatchInPart(int tick, Staff* st, bool start, Score* sc, int strack, int etrack)
      {
      for (MeasureBase* mb = sc->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            DirectionsAnchor* da = findSpecificMatchInMeasure(tick, st, start, m, strack, etrack);
            if (da)
                  return da;
            }
      return 0;
      }

//---------------------------------------------------------
//   findMatchInPart -- find chord or rest in part
//     if start is true, try to find a match at start first
//     if start is false, try to find a match at end first
//---------------------------------------------------------

static DirectionsAnchor* findMatchInPart(int tick, Staff* st, bool start, Score* sc, Part* p, int strack, int etrack)
      {
      DirectionsAnchor* da = 0;
      if (start)
            da = findSpecificMatchInPart(tick, st, true, sc, strack, etrack);
      if (da)
            return da;
      da = findSpecificMatchInPart(tick, st, false, sc, strack, etrack);
      if (da)
            return da;
      if (!start)
            da = findSpecificMatchInPart(tick, st, true, sc, strack, etrack);
      if (da)
            return da;
      return (st && st->part() == p) ? new DirectionsAnchor(tick) : 0;
      }

//---------------------------------------------------------
//   buildDirectionsList -- associate directions (measure relative elements)
//     with elements in segments to enable writing at the correct position
//     in the output stream. Called once for every part to handle all part-level elements.
//---------------------------------------------------------

void DirectionsHandler::buildDirectionsList(Part* p, int strack, int etrack)
      {
      // part-level elements stored in the score layout
      foreach(Element* dir, *(cs->gel())) {
            DirectionsAnchor* da = 0;
            switch(dir->type()) {
                  case HAIRPIN:
                  case OTTAVA:
                  case PEDAL:
                  case TEXTLINE:
                        {
                        SLine* sl = (SLine*) dir;
                        da = findMatchInPart(sl->tick(), sl->staff(), true, cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        da = findMatchInPart(sl->tick2(), sl->staff(), false, cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        }
                        break;
                  default:
                        // all others silently ignored
                        // printf("DirectionsHandler::buildDirectionsList: direction type %s not implemented\n",
                        //        Element::name(dir->type()));
                        break;
                  }
            }
      // part-level elements stored in measures
      for (MeasureBase* mb = cs->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            buildDirectionsList(m, true, p, strack, etrack);
            }
      }

//---------------------------------------------------------
//   buildDirectionsList -- associate directions (measure relative elements)
//     with elements in segments to enable writing at the correct position
//     in the output stream. Called once for every measure to handle either
//     part-level or measure-level elements.
//---------------------------------------------------------

void DirectionsHandler::buildDirectionsList(Measure* m, bool dopart, Part* p, int strack, int etrack)
      {
      // loop over all measure relative elements in this measure
      for (ciElement ci = m->el()->begin(); ci != m->el()->end(); ++ci) {
            DirectionsAnchor* da = 0;
            Element* dir = *ci;
            switch(dir->type()) {
                  case DYNAMIC:
                  case SYMBOL:
                  case TEMPO_TEXT:
                  case TEXT:
                  case STAFF_TEXT:
                        if (!dopart) {
                              // LVIFIX 20071110 TODO
                              // Even though they are moved to from measure to vbox, the elements
                              // Title, Subtitle, Poet and Composer show up here. As their staff
                              // is null, that causes a segfault in findMatchInMeasure.
                              if (dir->staff())
                              // END LVIFIX 20071110 TODO
                              da = findMatchInMeasure(dir->tick(), dir->staff(), m, p, strack, etrack);
                              if (da) {
                                    da->setDirect(dir);
                                    storeAnchor(da);
                                    }
                        }
                        break;
                  default:
                        // all others silently ignored
                        // printf("DirectionsHandler::buildDirectionsList: direction type %s not implemented\n",
                        //        Element::name(dir->type()));
                        break;
                  }
            }
      }

// helpers for ::calcDivisions

typedef QList<int> IntVector;
static IntVector integers;
static IntVector primes;

// check if all integers can be divided by d

static bool canDivideBy(int d)
      {
      bool res = true;
      for (int i = 0; i < integers.count(); i++) {
            if ((integers[i] <= 1) || ((integers[i] % d) != 0)) {
                  res = false;
                  }
            }
      return res;
      }

// divide all integers by d

static void divideBy(int d)
      {
      for (int i = 0; i < integers.count(); i++) {
            integers[i] /= d;
            }
      }

static void addInteger(int len)
      {
      if (!integers.contains(len)) {
            integers.append(len);
            }
      }

//---------------------------------------------------------
//   calcDivMoveToTick
//---------------------------------------------------------

void ExportMusicXml::calcDivMoveToTick(int t)
      {
      if (t < tick) {
//            printf("backup %d\n", tick - t);
            addInteger(tick - t);
            }
      else if (t > tick) {
//            printf("forward %d\n", t - tick);
            addInteger(t - tick);
            }
      tick = t;
      }

//---------------------------------------------------------
//  calcDivisions
//---------------------------------------------------------

// Loop over all voices in all staffs and determine a suitable value for divisions.

// Length of time in MusicXML is expressed in "units", which should allow expressing all time values
// as an integral number of units. Divisions contains the number of units in a quarter note.
// MuseScore uses division (480) midi ticks to represent a quarter note, which expresses all note values
// plus triplets and quintuplets as integer values. Solution is to collect all time values required,
// and divide them by the highest common denominator, which is implemented as a series of
// divisions by prime factors. Initialize the list with division to make sure a quarter note can always
// be written as an integral number of units.

/**
 */

void ExportMusicXml::calcDivisions()
      {
      // init
      integers.clear();
      primes.clear();
      integers.append(AL::division);
      primes.append(2);
      primes.append(3);
      primes.append(5);

      const QList<Part*>* il = score->parts();

      for (int idx = 0; idx < il->size(); ++idx) {
            Part* part = il->at(idx);
            tick = 0;

            int staves = part->nstaves();
            int strack = score->staffIdx(part) * VOICES;
            int etrack = strack + staves * VOICES;
/*
            DirectionsHandler dh(score);
            dh.buildDirectionsList(part, strack, etrack);
*/
            for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;
/*
                  dh.buildDirectionsList(m, false, part, strack, etrack);
*/

                  for (int st = strack; st < etrack; ++st) {
                        // sstaff - xml staff number, counting from 1 for this
                        // instrument
                        // special number 0 -> dont show staff number in
                        // xml output (because there is only one staff)

                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;
                        // printf("strack=%d etrack=%d st=%d sstaff=%d\n", strack, etrack, st, sstaff);

                        for (Segment* seg = m->first(); seg; seg = seg->next()) {
                              Element* el = seg->element(st);
                              if (!el)
                                    continue;
                              // must ignore start repeat to prevent spurious backup/forward
                              if (el->type() == BAR_LINE && el->subtype() == START_REPEAT)
                                    continue;

                              if (tick != el->tick()) {
//                                    attr.doAttr(xml, false);
                                    calcDivMoveToTick(el->tick());
                                    }
                              if (el->isChordRest()) {
                                    int l = static_cast<ChordRest*>(el)->ticks();
                                    if (el->type() == CHORD) {
                                          if (isTwoNoteTremolo(static_cast<Chord*>(el)))
                                                l /= 2;
                                          }
                                    // printf("chordrest %d\n", l);
                                    addInteger(l);
                                    tick += l;
                                    }

//                              dh.handleElement(this, el, sstaff, true);
//                              dh.handleElement(this, el, sstaff, false);
                              }
                        if (!((st + 1) % VOICES)) {
                              // sstaff may be 0, which causes a failed assertion (and abort)
                              // in (*i)->staff(ssstaff - 1)
                              // LVIFIX: find exact cause
//                              int ssstaff = sstaff > 0 ? sstaff : sstaff + 1;
                              // printf("st=%d sstaff=%d ssstaff=%d\n", st, sstaff, ssstaff);
//                              dh.handleElements(this, part->staff(ssstaff - 1), m->tick(), m->tick() + m->tickLen(), sstaff);
                              }
                        }
                  // move to end of measure (in case of incomplete last voice)
                  calcDivMoveToTick(m->tick() + m->tickLen());
                  }
            }

      // do it: divide by all primes as often as possible
      for (int u = 0; u < primes.count(); u++) {
            while (canDivideBy(primes[u])) {
                  divideBy(primes[u]);
                  }
            }

      div = AL::division / integers[0];
      // printf("divisions=%d div=%d\n", integers[0], div);
      }

//---------------------------------------------------------
//   defaults
//---------------------------------------------------------

// _spatium = DPMM * (millimeter * 10.0 / tenths);

static void defaults(Xml& xml, Score* s, double& millimeters, const int& tenths)
      {
      xml.stag("defaults");
      xml.stag("scaling");
      xml.tag("millimeters", millimeters);
      xml.tag("tenths", tenths);
      xml.etag();
      PageFormat* pf = s->pageFormat();
      if (pf) pf->writeMusicXML(xml, INCH / millimeters * tenths);
      xml.etag();
      }


//---------------------------------------------------------
//   creditWords
//---------------------------------------------------------

static void creditWords(Xml& xml, double x, double y, int fs, QString just, QString val, QString words)
      {
      xml.stag("credit page=\"1\"");
      QString tagname = QString("credit-words");
      tagname += QString(" default-x=\"%1\"").arg(x);
      tagname += QString(" default-y=\"%1\"").arg(y);
      tagname += QString(" font-size=\"%1\"").arg(fs);
      tagname += " justify=\"" + just + "\"";
      tagname += " valign=\"" + val + "\"";
      xml.tag(tagname, words);
      xml.etag();
      }


//---------------------------------------------------------
//   credits
//---------------------------------------------------------

void ExportMusicXml::credits(Xml& xml)
      {
      // debug
      // printf("credits:\n");
      const MeasureBase* measure = score->measures()->first();
/*
      foreach(const Element* element, *measure->el()) {
            if (element->type() == TEXT) {
                  const Text* text = (const Text*)element;
                  bool mustPrint = true;
                  switch (text->subtype()) {
                        case TEXT_TITLE:
                              printf("title");
                              break;
                        case TEXT_SUBTITLE:
                              printf("subtitle");
                              break;
                        case TEXT_COMPOSER:
                              printf("composer");
                              break;
                        case TEXT_POET:
                              printf("poet");
                              break;
                        case TEXT_TRANSLATOR:
                              printf("translator");
                              break;
                        default:
                              mustPrint = false;
                        }
                  if (mustPrint) printf(" '%s at %f,%f'\n",
                                        text->getText().toUtf8().data(),
                                        text->canvasPos().x(),
                                        text->canvasPos().y()
                                       );
                  }
            }
      if (score->copyright()) printf("copyright '%s'\n", score->copyright()->getText().toUtf8().data());
      printf("end credits\n");
*/

      // determine formatting
      PageFormat* pf = score->pageFormat();
      if (!pf) return;
      //const double t  = 2 * PPI * 10 / 9;
      //const double t  = INCH / millimeters * tenths;
      const double h  = getTenthsFromInches(pf->height());
      const double w  = getTenthsFromInches(pf->width());
      const double lm = getTenthsFromInches(pf->oddLeftMargin);
      const double rm = getTenthsFromInches(pf->oddRightMargin);
//      const double tm = getTenthsFromInches(pf->oddTopMargin);
      const double bm = getTenthsFromInches(pf->oddBottomMargin);
//      printf(" h=%g w=%g lm=%g rm=%g tm=%g bm=%g\n", h, w, lm, rm, tm, bm);

      // write the credits
      // TODO add real font size
      foreach(const Element* element, *measure->el()) {
            if (element->type() == TEXT) {
                  const Text* text = (const Text*)element;
/*
                  printf("x=%g, y=%g fs=%d\n",
                         text->canvasPos().x(),
                         h - text->canvasPos().y(),
                         text->defaultFont().pointSize()
                        );
*/
                  const double ty = h - getTenthsFromDots(text->canvasPos().y());
                  const int fs = text->defaultFont().pointSize();
                  switch (text->subtype()) {
                        case TEXT_TITLE:
                              creditWords(xml, w / 2, ty, fs, "center", "top", text->getText());
                              break;
                        case TEXT_SUBTITLE:
                              creditWords(xml, w / 2, ty, fs, "center", "top", text->getText());
                              break;
                        case TEXT_COMPOSER:
                              creditWords(xml, w - rm, ty, fs, "right", "top", text->getText());
                              break;
                        case TEXT_POET:
                              creditWords(xml, lm, ty, fs, "left", "top", text->getText());
                              break;
                        // case TEXT_TRANSLATOR:
                              break;
                        default:
                              printf("credits: text subtype %s not supported\n",
                                      text->subtypeName().toUtf8().data());
                        }
                  }
            }
      if (score->copyright()) {
            const int fs = score->copyright()->defaultFont().pointSize();
            creditWords(xml, w / 2, bm, fs, "center", "bottom", score->copyright()->getText());
            }
/**/
      }

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

void ExportMusicXml::pitch2xml(Note* note, char& c, int& alter, int& octave)
      {
      static char table1[]  = "FEDCBAG";

      int tick   = note->chord()->tick();
      Staff* i   = note->staff();
      int clef   = i->clefList()->clef(tick);
      int offset = clefTable[clef].yOffset;

      int step   = (note->line() - offset + 700) % 7;
      c          = table1[step];

      int pitch  = note->pitch() - 12;
      octave     = pitch / 12;

      static int table2[7] = { 5, 4, 2, 0, 11, 9, 7 };
      int npitch = table2[step] + (octave + 1) * 12;

      //
      // HACK:
      // On percussion clefs there is no relationship between
      // note->pitch() and note->line()
      // note->line() is determined by drumMap
      //
      if (clef == CLEF_PERC || clef == CLEF_PERC2) {
            alter = 0;
            pitch = line2pitch(note->line(), clef, 0);
            octave = (pitch / 12) - 1;
            }
      else
            alter = note->pitch() - npitch;

      // correct for ottava lines
      int ottava = 0;
      switch (note->ppitch() - note->pitch()) {
            case  24: ottava =  2; break;
            case  12: ottava =  1; break;
            case   0: ottava =  0; break;
            case -12: ottava = -1; break;
            case -24: ottava = -2; break;
            default:  /* printf("pitch2xml() tick=%d pitch()=%d ppitch()=%dd\n",
                             tick, note->pitch(), note->ppitch()) */;
            }
      octave += ottava;
//      printf("pitch2xml() tick=%d offset=%d step=%d pitch()=%d ppitch()=%d npitch=%d alter=%d ottava=%d\n",
//             tick, offset, step, note->pitch(), note->ppitch(), npitch, alter, ottava);

      //deal with Cb and B#
      if (alter > 2) {
            printf("pitch2xml problem: alter %d step %d(line %d) octave %d clef %d(offset %d)\n",
               alter, step, note->line(), octave, clef, offset);
//HACK:
            alter  -= 12;
            octave += 1;
            }
      if (alter < -2) {
            printf("pitch2xml problem: alter %d step %d(line %d) octave %d clef %d(offset %d)\n",
               alter, step, note->line(), octave, clef, offset);
//HACK:
            alter  += 12;
            octave -= 1;
            }
      }

void ExportMusicXml::unpitch2xml(Note* note, char& c, int& octave)
      {
          static char table1[]  = "FEDCBAG";

          int tick   = note->chord()->tick();
          Staff* i   = note->staff();
          int offset = clefTable[i->clefList()->clef(tick)].yOffset;

          int step   = (note->line() - offset + 700) % 7;
          c          = table1[step];

          int tmp = (note->line() - offset);
          octave =(3-tmp+700)/7 + 5 - 100;
      }

//---------------------------------------------------------
//   tick2xml
//    set type + dots depending on tick len
//---------------------------------------------------------

static QString tick2xml(const int ticks, int* dots)
      {
      Duration t;
      t.setVal(ticks);
      *dots = t.dots();
      return t.name();
      }

//---------------------------------------------------------
//   findVolta -- find volta starting in measure m
//---------------------------------------------------------

static Volta* findVolta(Measure* m, bool left)
      {
      foreach(Element* el, *(m->score()->gel())) {
            if (el->type() == VOLTA) {
                  Volta* v = (Volta*) el;
                  if ((left && v->tick() == m->tick())
                      || (!left && v->tick2() == (m->tick() + m->tickLen()))) {
                        return v;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   ending
//---------------------------------------------------------

static void ending(Xml& xml, Volta* v, bool left)
      {
      QString number = "";
      QString type = "";
      foreach(int i, v->endings()) {
            if (!number.isEmpty())
                  number += ", ";
            number += QString("%1").arg(i);
            }
      if (left) {
            type = "start";
            }
      else {
            int st = v->subtype();
            switch (st) {
                  case Volta::VOLTA_OPEN:
                        type = "discontinue";
                        break;
                  case Volta::VOLTA_CLOSED:
                        type = "stop";
                        break;
                  default:
                        printf("unknown volta subtype %d\n", st);
                        type = "unknown";
                        break;
                  }
            }
            xml.tagE("ending number=\"%s\" type=\"%s\"",
                     number.toLatin1().data(),
                     type.toLatin1().data());
      }

//---------------------------------------------------------
//   barlineLeft -- search for and handle barline left
//---------------------------------------------------------

void ExportMusicXml::barlineLeft(Measure* m)
      {
      bool rs = m->repeatFlags() & RepeatStart;
      Volta* volta = findVolta(m, true);
      if (!rs && !volta) return;
      attr.doAttr(xml, false);
      xml.stag(QString("barline location=\"left\""));
      if (rs) {
            xml.tag("bar-style", QString("heavy-light"));
            xml.tagE("repeat direction=\"forward\"");
            }
      if (volta) {
            ending(xml, volta, true);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   barlineRight -- search for and handle barline right
//---------------------------------------------------------

void ExportMusicXml::barlineRight(Measure* m)
      {
      int bst = m->endBarLineType();
      bool visible = m->endBarLineVisible();
      bool needBarStyle = (bst != NORMAL_BAR && bst != START_REPEAT) || !visible;
      Volta* volta = findVolta(m, false);
      if (!needBarStyle && !volta)
            return;
      xml.stag(QString("barline location=\"right\""));
      if (needBarStyle) {
            if(!visible){
                  xml.tag("bar-style", QString("none"));
            }else{
                  switch(bst) {
                        case DOUBLE_BAR:
                              xml.tag("bar-style", QString("light-light"));
                              break;
                        case END_REPEAT:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        case BROKEN_BAR:
                              xml.tag("bar-style", QString("dotted"));
                              break;
                        case END_BAR:
                        case END_START_REPEAT:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        default:
                              printf("ExportMusicXml::bar(): bar subtype %d not supported\n", bst);
                              break;
                        }
                  }
            }
      if (volta)
            ending(xml, volta, false);
      if (bst == END_REPEAT || bst == END_START_REPEAT)
            xml.tagE("repeat direction=\"backward\"");
      xml.etag();
      }

//---------------------------------------------------------
//   moveToTick
//---------------------------------------------------------

void ExportMusicXml::moveToTick(int t)
      {
//      printf("ExportMusicXml::moveToTick(t=%d) tick=%d\n", t, tick);
      if (t < tick) {
//            printf(" -> backup");
            attr.doAttr(xml, false);
            xml.stag("backup");
            xml.tag("duration", (tick - t) / div);
            xml.etag();
            }
      else if (t > tick) {
//            printf(" -> forward");
            attr.doAttr(xml, false);
            xml.stag("forward");
            xml.tag("duration", (t - tick) / div);
            xml.etag();
            }
//      printf("\n");
      tick = t;
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

void ExportMusicXml::timesig(TimeSig* tsig)
      {
      int n  = 0;
      int st = tsig->subtype();
      int z1 = 0;
      int z2 = 0;
      int z3 = 0;
      int z4 = 0;
      tsig->getSig(&n, &z1, &z2, &z3, &z4);
      if (st == TSIG_ALLA_BREVE) {
            // MuseScore calls this 2+2/4, MusicXML 2/2
            n = 2;
            z2 = 0;
            }
      attr.doAttr(xml, true);
      if (st == TSIG_FOUR_FOUR)
            xml.stag("time symbol=\"common\"");
      else if (st == TSIG_ALLA_BREVE)
            xml.stag("time symbol=\"cut\"");
      else
            xml.stag("time");
      QString z = QString("%1").arg(z1);
      if (z2) z += QString("+%1").arg(z2);
      if (z3) z += QString("+%1").arg(z3);
      if (z4) z += QString("+%1").arg(z4);
      xml.tag("beats", z);
      xml.tag("beat-type", n);
      xml.etag();
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void ExportMusicXml::keysig(int key, bool visible)
      {
      attr.doAttr(xml, true);
      if (visible)
            xml.stag("key");
      else
            xml.stag("key print-object=\"no\"");
      xml.tag("fifths", key);
      xml.tag("mode", QString("major"));
      xml.etag();
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

void ExportMusicXml::clef(int staff, int clef)
      {
//      printf("ExportMusicXml::clef(staff=%d, clef=%d)\n", staff, clef);
      attr.doAttr(xml, true);
      if (staff)
            xml.stag(QString("clef number=\"%1\"").arg(staff));
      else
            xml.stag("clef");
      QString sign = clefTable[clef].sign;
      int line   = clefTable[clef].line;
      xml.tag("sign", sign);
      xml.tag("line", line);
      if (clefTable[clef].octChng)
            xml.tag("clef-octave-change", clefTable[clef].octChng);
      xml.etag();
      }

//---------------------------------------------------------
//   tupletStartStop
//---------------------------------------------------------

// LVIFIX: add placement to tuplet support
// <notations>
//   <tuplet type="start" placement="above" bracket="no"/>
// </notations>

static void tupletStartStop(ChordRest* cr, Notations& notations, Xml& xml)
      {
      Tuplet* t = cr->tuplet();
      if (!t) return;
      if (cr == t->elements().front()) {
            notations.tag(xml);
            xml.tagE("tuplet type=\"start\" bracket=\"%s\"", t->hasBracket() ? "yes" : "no");
            }
      if (cr == t->elements().back()) {
            notations.tag(xml);
            xml.tagE("tuplet type=\"stop\"");
            }
      }

//---------------------------------------------------------
//   wavyLineStartStop
//---------------------------------------------------------

static void wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, Xml& xml)
      {
      // search for trill starting at this chord
      foreach(Element* el, *(chord->score()->gel())) {
            if (el->type() == TRILL) {
                  Trill* t = (Trill*) el;
                  if (t->tick() == chord->tick() && t->track() == chord->track()) {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        // mscore only supports wavy-line with trill-mark
                        xml.tagE("trill-mark");
                        xml.tagE("wavy-line type=\"start\"");
                        }
                  else if (t->tick2() == chord->tick()+chord->tickLen() && t->track() == chord->track()) {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("wavy-line type=\"stop\"");
                        }
                  }
            }
      }

//---------------------------------------------------------
//   hasBreathMark - determine if chord has breath-mark
//---------------------------------------------------------

static bool hasBreathMark(Chord* ch)
      {
      Segment* s = ch->segment();
      s = s->next1();
      return (s->subtype() == SegBreath && s->element(ch->track()));
      }

//---------------------------------------------------------
// isTwoNoteTremolo - determine is chord is part of two note tremolo
//---------------------------------------------------------

static bool isTwoNoteTremolo(Chord* chord)
      {
      ChordRest* cr = nextChordRest(chord);
      Chord* nextChord = 0;
      if (cr && cr->type() == CHORD) nextChord = static_cast<Chord*>(cr);
      Tremolo * tr = 0;
      // this chord or next chord may have tremolo, but not both chords
      if (chord && chord->tremolo()) {
            tr = chord->tremolo();
            }
      if (nextChord && nextChord->tremolo()) {
            tr = nextChord->tremolo();
            }
      if (tr) {
            int st = tr->subtype();
            if (st == 3 || st == 4 || st == 5) return true;
            }
      return false;
      }

//---------------------------------------------------------
//   tremoloSingleStartStop
//---------------------------------------------------------

static void tremoloSingleStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, Xml& xml)
      {
//      printf("tremoloSingleStartStop: chord=%p trem=%p nextchord=%p\n", chord, chord->tremolo(), nextChordRest(chord));
      ChordRest* cr = nextChordRest(chord);
      Chord* nextChord = 0;
      if (cr && cr->type() == CHORD) nextChord = static_cast<Chord*>(cr);
      if (nextChord && nextChord->tremolo()) {
            Tremolo * tr = nextChord->tremolo();
            int st = tr->subtype();
            switch (st) {
                  case TREMOLO_1:
                  case TREMOLO_2:
                  case TREMOLO_3:
                        // ignore
                        break;
                  case 3:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"start\"", "1");
                        break;
                  case 4:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"start\"", "2");
                        break;
                  case 5:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"start\"", "3");
                        break;
                  default:
                        printf("unknown tremolo %d\n", st);
                        break;
                  }
            }
      if (chord->tremolo()) {
            Tremolo * tr = chord->tremolo();
            int st = tr->subtype();
            switch (st) {
                  case TREMOLO_1:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"single\"", "1");
                        break;
                  case TREMOLO_2:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"single\"", "2");
                        break;
                  case TREMOLO_3:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"single\"", "3");
                        break;
                  case 3:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"stop\"", "1");
                        break;
                  case 4:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"stop\"", "2");
                        break;
                  case 5:
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tag("tremolo type=\"stop\"", "3");
                        break;
                  default:
                        printf("unknown tremolo %d\n", st);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   chordAttributes
//---------------------------------------------------------

static void chordAttributes(Chord* chord, Notations& notations, Technical& technical, Xml& xml)
      {
      QList<Articulation*>* na = chord->getArticulations();
      // first output the fermatas
      for (ciArticulation ia = chord->getArticulations()->begin(); ia != chord->getArticulations()->end(); ++ia) {
            if ((*ia)->subtype() == UfermataSym) {
                  notations.tag(xml);
                  xml.tagE("fermata type=\"upright\"");
                  }
            else if ((*ia)->subtype() == DfermataSym) {
                  notations.tag(xml);
                  xml.tagE("fermata type=\"inverted\"");
                  }
            }

      // then the attributes whose elements are children of <articulations>
      Articulations articulations;
      for (ciArticulation ia = na->begin(); ia != na->end(); ++ia) {
            switch ((*ia)->subtype()) {
                  case UfermataSym:
                  case DfermataSym:
                        // ignore, already handled
                        break;
                  case SforzatoaccentSym:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("accent");
                        }
                        break;
                  case StaccatoSym:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccato");
                        }
                        break;
                  case UstaccatissimoSym:
                  case DstaccatissimoSym:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("staccatissimo");
                        }
                        break;
                  case TenutoSym:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("tenuto");
                        }
                        break;
                  case DmarcatoSym:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("strong-accent type=\"down\"");
                        }
                        break;
                  case UmarcatoSym:
                        {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE("strong-accent type=\"up\"");
                        }
                        break;
                  case ReverseturnSym:
                  case TurnSym:
                  case TrillSym:
                  case PrallSym:
                  case MordentSym:
                        // ignore, handled with ornaments
                        break;
                  case PlusstopSym:
                  case UpbowSym:
                  case DownbowSym:
                        // ignore, handled with technical
                        break;
                  default:
                        printf("unknown chord attribute %s\n", (*ia)->name().toLatin1().data());
                        break;
                  }
            }
            if (hasBreathMark(chord)) {
                  notations.tag(xml);
                  articulations.tag(xml);
                  xml.tagE("breath-mark");
                  }
            articulations.etag(xml);

      // then the attributes whose elements are children of <ornaments>
      Ornaments ornaments;
      for (ciArticulation ia = na->begin(); ia != na->end(); ++ia) {
            switch ((*ia)->subtype()) {
                  case UfermataSym:
                  case DfermataSym:
                  case SforzatoaccentSym:
                  case StaccatoSym:
                  case UstaccatissimoSym:
                  case DstaccatissimoSym:
                  case TenutoSym:
                  case DmarcatoSym:
                  case UmarcatoSym:
                        // ignore, already handled
                        break;
                  case ReverseturnSym:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-turn");
                        }
                        break;
                  case TurnSym:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("turn");
                        }
                        break;
                  case TrillSym:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("trill-mark");
                        }
                        break;
                  case PrallSym:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("inverted-mordent");
                        }
                        break;
                  case MordentSym:
                        {
                        notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("mordent");
                        }
                        break;
                  case PlusstopSym:
                  case UpbowSym:
                  case DownbowSym:
                        // ignore, handled with technical
                        break;
                  default:
                        printf("unknown chord attribute %s\n", (*ia)->name().toLatin1().data());
                        break;
                  }
            }
            tremoloSingleStartStop(chord, notations, ornaments, xml);
            wavyLineStartStop(chord, notations, ornaments, xml);
            ornaments.etag(xml);

      // and finally the attributes whose elements are children of <technical>
      for (ciArticulation ia = na->begin(); ia != na->end(); ++ia) {
            switch ((*ia)->subtype()) {
                  case PlusstopSym:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("stopped");
                        }
                        break;
                  case UpbowSym:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("up-bow");
                        }
                        break;
                  case DownbowSym:
                        {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE("down-bow");
                        }
                        break;
                  default:
                        // others silently ignored
                        // printf("unknown chord attribute %s\n", (*ia)->name().toLatin1().data());
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   arpeggiate
//---------------------------------------------------------

// <notations>
//   <arpeggiate direction="up"/>
//   </notations>

static void arpeggiate(Arpeggio * arp, bool front, bool back, Xml& xml, Notations& notations)
      {
      int st = arp->subtype();
      switch (st) {
            case 0:
                  notations.tag(xml);
                  xml.tagE("arpeggiate");
                  break;
            case 1:
                  notations.tag(xml);
                  xml.tagE("arpeggiate direction=\"up\"");
                  break;
            case 2:
                  notations.tag(xml);
                  xml.tagE("arpeggiate direction=\"down\"");
                  break;
            case 3:
                  if (front) {
                        notations.tag(xml);
                        xml.tagE("non-arpeggiate type=\"bottom\"");
                        }
                  if (back) {
                        notations.tag(xml);
                        xml.tagE("non-arpeggiate type=\"top\"");
                        }
                  break;
            default:
                  printf("unknown arpeggio subtype %d\n", st);
                  break;
            }
      }

// find the next chord in the same track

static Chord* nextChord(Chord* ch)
      {
      Segment* s = ch->segment();
      s = s->next1();
      while (s) {
            if (s->subtype() == SegChordRest && s->element(ch->track()))
                  break;
            s = s->next1();
            }
      if (s == 0) {
            // printf("no segment for second note of glissando found\n");
            return 0;
            }
      Chord* c = static_cast<Chord*>(s->element(ch->track()));
      if (c == 0 || c->type() != CHORD) {
            // printf("no second note for glissando found, track %d\n", track());
            return 0;
            }
      return c;
      }

//---------------------------------------------------------
//   chord
//---------------------------------------------------------

/**
 Write \a chord on \a staff with lyriclist \a ll.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::chord(Chord* chord, int staff, const LyricsList* ll, bool useDrumset)
      {
//      printf("ExportMusicXml::chord() oldtick=%d\n", tick);
      QList<Note*> nl = chord->notes();
      NoteType gracen = nl.front()->noteType();
      bool grace = (gracen == NOTE_ACCIACCATURA
                 || gracen == NOTE_APPOGGIATURA
                 || gracen == NOTE_GRACE4
                 || gracen == NOTE_GRACE16
                 || gracen == NOTE_GRACE32);
//      printf("notetype=%d grace=%d\n", gracen, grace);
      int tremCorr = 1; // duration correction for two note tremolo
      if (isTwoNoteTremolo(chord)) tremCorr = 2;
      if (!grace) tick += chord->ticks() / tremCorr;
//      printf(" newtick=%d\n", tick);

      PageFormat* pf = score->pageFormat();
      const double pageHeight  = getTenthsFromInches(pf->height());
//      const double pageWidth  = getTenthsFromInches(pf->width());

      foreach(Note* note, nl) {
            QString val;

            attr.doAttr(xml, false);
            QString noteTag = QString("note");

            if (pf && (!converterMode || score->defaultsRead()) ) {
                  double measureX = getTenthsFromDots(chord->measure()->canvasPos().x());
                  double measureY = pageHeight - getTenthsFromDots(chord->measure()->canvasPos().y());
                  double noteX = getTenthsFromDots(note->canvasPos().x());
                  double noteY = pageHeight - getTenthsFromDots(note->canvasPos().y());

                  noteTag += QString(" default-x=\"%1\"").arg(QString::number(noteX - measureX,'f',2));
                  noteTag += QString(" default-y=\"%1\"").arg(QString::number(noteY - measureY,'f',2));
                  }

            if (!note->visible()) {
                  noteTag += QString(" print-object=\"no\"");
                  }

            xml.stag(noteTag);

            if (grace) {
                  if (note->noteType() == NOTE_ACCIACCATURA)
                        xml.tagE("grace slash=\"yes\"");
                  else
                        xml.tagE("grace");
                  }
            if (note != nl.front())
                  xml.tagE("chord");

            char c;
            int alter;
            int octave;
            char buffer[2];

            if (!useDrumset) {
                  pitch2xml(note, c, alter, octave);
                  buffer[0] = c;
                  buffer[1] = 0;
                  // pitch
                  xml.stag("pitch");
                  xml.tag("step", QString(buffer));
                  if (alter)
                        xml.tag("alter", alter);
                  xml.tag("octave", octave);
                  xml.etag();
                  }
            else {
                  // unpitched
                  unpitch2xml(note, c, octave);
                  buffer[0] = c;
                  buffer[1] = 0;
                  xml.stag("unpitched");
                  xml.tag("display-step", QString(buffer));
                  xml.tag("display-octave", octave);
                  xml.etag();
                  }

            // duration
            if (!grace)
                  xml.tag("duration", note->chord()->tickLen() / (div * tremCorr));
            //instrument for unpitched
            if (useDrumset)            
                  xml.tagE(QString("instrument id=\"P%1-I%2\"").arg(score->parts()->indexOf(note->staff()->part()) + 1).arg(note->pitch() + 1));
            if (note->tieBack())
                  xml.tagE("tie type=\"stop\"");
            if (note->tieFor())
                  xml.tagE("tie type=\"start\"");

            // voice
            // for a single-staff part, staff is 0, which needs to be corrected
            // to calculate the correct voice number
            int voice = (staff-1) * VOICES + note->chord()->voice() + 1;
            if (staff == 0)
                voice += VOICES;

            xml.tag("voice", voice);

            // type
            int dots = 0;
            Tuplet* t = note->chord()->tuplet();
            int actNotes = 1;
            int nrmNotes = 1;
            if (t) {
                  actNotes = t->ratio().numerator();
                  nrmNotes = t->ratio().denominator();
                  }

            QString s = tick2xml(note->chord()->tickLen() * actNotes / (nrmNotes * tremCorr), &dots);
            if (s.isEmpty()) {
                  printf("no note type found for ticks %d\n",
                     note->chord()->tickLen());
                  }
            xml.tag("type", s);
            for (int ni = dots; ni > 0; ni--)
                  xml.tagE("dot");

            // accidental
            // Note: in Binchois.xml two accidentals have parentheses which are encoded
            // as editorial="yes". Wikipedia calls this a cautionary accidental.
            // Brackets/parenthese are controlled by the level-display entity (DTD 1.1)
            bool editorial = false;
            int acc        = note->accidentalType();
            if (acc != ACC_NONE) {
                  if (6 <= acc && acc <= 10) {
                        acc -= 5;
                        editorial = true;
                        }
                  else if (11 <= acc && acc <= 15) {
                        acc -= 10;
                        editorial = true;
                        }
                  if (note->accidental()->hasBracket()) {
                        editorial = true;
                        }
                  /*
                        MusicXML accidental names include:
                        sharp,natural, flat, double-sharp, sharp-sharp, flat-flat,
                        natural-sharp, natural-flat, quarter-flat, quarter-sharp,
                        three-quarters-flat, and three-quarters-sharp
                    */
                  QString s;
                  switch(acc) {
                        case ACC_SHARP:   s = "sharp";        break;
                        case ACC_FLAT:    s = "flat";         break;
                        case ACC_SHARP2:  s = "double-sharp"; break;
                        case ACC_FLAT2:   s = "flat-flat";    break;
                        case ACC_NATURAL: s = "natural";      break;
                        case 16: s = "quarter-flat";          break; //flat-slash (alternative)
                        case 19: s = "quarter-flat";          break; //mirrored-flat (recommended by Michael)
                        case 29: s = "quarter-flat";          break; //flat arrow up (alternative)
                        case 33: s = "quarter-flat";          break; //natural arrow down (alternative)
                        case 22: s = "quarter-sharp";         break; //sharp-slash (recommended by Michael)
                        case 27: s = "quarter-sharp";         break; //sharp arrow down (alternative)
                        case 32: s = "quarter-sharp";         break; //natural arrow up (alternative)
                        case 18: s = "three-quarters-flat";   break; //mirrored-flat1 (recommended by Michael)
                        case 21: s = "three-quarters-flat";   break; //flat-flat-slash (alternative)
                        case 30: s = "three-quarters-flat";   break; //flat arrow down (alternative)
                        case 25: s = "three-quarters-sharp";  break; //sharp-slash4 (recommended by Michael)
                        case 26: s = "three-quarters-sharp";  break; //sharp arrow up (alternate)
                        default:
                              printf("unknown accidental %d\n", acc);
                        }
                  if (editorial)
                        xml.tag("accidental editorial=\"yes\"", s);
                  else
                        xml.tag("accidental", s);
                  }

            if (t) {
                  xml.stag("time-modification");
                  xml.tag("actual-notes", actNotes);
                  xml.tag("normal-notes", nrmNotes);
                  xml.etag();
                  }

            // no stem for whole notes and beyond
            if (chord->noStem() || chord->measure()->slashStyle(chord->staffIdx())){
                  xml.tag("stem", QString("none"));
            }
            else if ((note->chord()->tickLen() * actNotes / (nrmNotes * tremCorr)) < (4 * AL::division)) {
                  xml.tag("stem", QString(note->chord()->up() ? "up" : "down"));
            }

            QString noteheadTagname = QString("notehead");
            QColor noteheadColor = note->color();
            if(noteheadColor != preferences.defaultColor)
                  noteheadTagname += " color=\"" + noteheadColor.name() + "\"";
            if (note->headGroup() == 5) {
                  xml.tag(noteheadTagname, "slash");
            } else if (note->headGroup() == 3) {
                  xml.tag(noteheadTagname, "triangle");
            } else if(note->headGroup() == 2) {
                  xml.tag(noteheadTagname, "diamond");
            } else if(note->headGroup() == 1) {
                  xml.tag(noteheadTagname, "x");
            } else if(note->headGroup() == 6) {
                  xml.tag(noteheadTagname, "circle-x");
            } else if(note->headGroup() == 7) {
                  xml.tag(noteheadTagname, "do");
            } else if(note->headGroup() == 8) {
                  xml.tag(noteheadTagname, "re");
            } else if(note->headGroup() == 4) {
                  xml.tag(noteheadTagname, "mi");
            } else if(note->headGroup() == 9) {
                  xml.tag(noteheadTagname, "fa");
            } else if(note->headGroup() == 10) {
                  xml.tag(noteheadTagname, "la");
            } else if(note->headGroup() == 11) {
                  xml.tag(noteheadTagname, "ti");
            } else if (noteheadColor != preferences.defaultColor) {
                  xml.tag(noteheadTagname, "normal");
            }

            // LVIFIX: check move() handling
            if (staff)
                  xml.tag("staff", staff + note->chord()->staffMove());

            //  beaming
            //    <beam number="1">start</beam>
            //    <beam number="1">end</beam>
            //    <beam number="1">continue</beam>
            //    <beam number="1">backward hook</beam>
            //    <beam number="1">forward hook</beam>

            if (note == nl.front() && chord->beam())
                  chord->beam()->writeMusicXml(xml, chord);

            Notations notations;
            Technical technical;
            if (note->tieBack()) {
                  notations.tag(xml);
                  xml.tagE("tied type=\"stop\"");
                  }
            if (note->tieFor()) {
                  notations.tag(xml);
                  xml.tagE("tied type=\"start\"");
                  }

            if (note == nl.front()) {
                  tupletStartStop(chord, notations, xml);
                  sh.doSlurStop(chord, notations, xml);
                  sh.doSlurStart(chord, notations, xml);
                  chordAttributes(chord, notations, technical, xml);
                  }
            foreach (const Element* e, *note->el()) {
                  if (e->type() == TEXT
                      && (e->subtype() == TEXT_FINGERING || e->subtype() == TEXT_STRING_NUMBER)) {
                        Text* f = (Text*)e;
                        notations.tag(xml);
                        technical.tag(xml);
                        QString t = f->getText();
                        if (e->subtype() == TEXT_FINGERING) {
                              // p, i, m, a, c represent the plucking finger
                              if (t == "p" || t == "i"  || t == "m" || t == "a" || t == "c")
                                    xml.tag("pluck", t);
                              else
                                    xml.tag("fingering", t);
                              }
                        else
                              // TEXT_STRING_NUMBER
                              xml.tag("string", t);
                        }
                  else {
                        // TODO
                        }
                  }
            technical.etag(xml);
            if (chord->arpeggio()) {
                  arpeggiate(chord->arpeggio(), note == nl.front(), note == nl.back(), xml, notations);
                  }
            // write glissando (only for last note)
            Chord* ch = nextChord(chord);
//            printf("chord->gliss=%p nextchord=%p gliss=%p\n", chord->glissando(), ch, ch ? ch->glissando() : 0);
            if ((note == nl.back()) && ch && ch->glissando()) {
                  gh.doGlissandoStart(ch, notations, xml);
                  }
            if (chord->glissando()) {
                  gh.doGlissandoStop(chord, notations, xml);
                  }
            notations.etag(xml);
//            printf("chord %p track %d\n", chord, chord->track());
            // write lyrics (only for first note)
            if ((note == nl.front()) && ll)
                  lyrics(ll, chord->track());
            xml.etag();
            }
      }

//---------------------------------------------------------
//   rest
//---------------------------------------------------------

/**
 Write \a rest on \a staff.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::rest(Rest* rest, int staff)
      {
      static char table2[]  = "CDEFGAB";
//      printf("ExportMusicXml::rest() oldtick=%d", tick);
      attr.doAttr(xml, false);

      QString noteTag = QString("note");
      if(! rest->visible() ){
          noteTag += QString(" print-object=\"no\"");
      }
      xml.stag(noteTag);

      double yOffsSp = rest->userOff().y() / rest->spatium();                    // y offset in spatium (negative = up)
      int    yOffsSt = - 2 * int(yOffsSp > 0.0 ? yOffsSp + 0.5 : yOffsSp - 0.5); // same rounded to int (positive = up)
//      printf(" yshift=%g", yOffsSp);
//      printf(" (up %d steps)", yOffsSt);
      int clef = rest->staff()->clefList()->clef(rest->tick());
      int po = clefTable[clef].pitchOffset;
      po -= 4;          // pitch middle staff line (two lines times two steps lower than top line)
      po += yOffsSt;    // rest "pitch"
      int oct = po / 7; // octave
      int stp = po % 7; // step
//      printf("\nclef=%d line=%d po=%d stp=%d oct=%d", clef, clefTable[clef].line, po, stp, oct);

      // Either <rest/>
      // or <rest><display-step>F</display-step><display-octave>5</display-octave></rest>
      if (yOffsSt == 0) {
            xml.tagE("rest");
            }
      else {
            xml.stag("rest");
            xml.tag("display-step", QString(QChar(table2[stp])));
            xml.tag("display-octave", oct - 1);
            xml.etag();
            }

      Duration d = rest->duration();
      int tickLen = rest->tickLen();
      if (d.type() == Duration::V_MEASURE){
            // to avoid forward since rest->ticklen=0 in this case.
            tickLen = rest->measure()->tickLen();
            }
      tick += tickLen;
//      printf(" tickLen=%d newtick=%d\n", tickLen, tick);

      xml.tag("duration", tickLen / div);

      // for a single-staff part, staff is 0, which needs to be corrected
      // to calculate the correct voice number
      int voice = (staff-1) * VOICES + rest->voice() + 1;
      if (staff == 0)
            voice += VOICES;
      xml.tag("voice", voice);

      // do not output a "type" element for whole measure rest
      if (d.type() != Duration::V_MEASURE) {
            QString s = d.name();
            int dots  = rest->dots();
            xml.tag("type", s);
            for (int i = dots; i > 0; i--)
                  xml.tagE("dot");
            }

      if (rest->tuplet()) {
            Tuplet* t = rest->tuplet();
            xml.stag("time-modification");
            xml.tag("actual-notes", t->ratio().numerator());
            xml.tag("normal-notes", t->ratio().denominator());
            xml.etag();
            }

      if (staff)
            xml.tag("staff", staff);

      Notations notations;
      tupletStartStop(rest, notations, xml);
      notations.etag(xml);

      xml.etag();
      }

//---------------------------------------------------------
//   directionTag
//---------------------------------------------------------

static void directionTag(Xml& xml, Attributes& attr, Element* el = 0)
      {
      attr.doAttr(xml, false);
      QString tagname = QString("direction");
      if (el) {
/*
            printf("directionTag() spatium=%g\nelem tp=%d st=%d (%s,%s) x=%g y=%g w=%g h=%g userOff.y=%g\n",
                   el->spatium(),
                   el->type(), el->subtype(),
                   el->name(), el->subtypeName().toUtf8().data(),
                   el->x(), el->y(),
                   el->width(), el->height(),
                   el->userOff().y());
*/
            if (el->type() == HAIRPIN || el->type() == OTTAVA || el->type() == TEXTLINE) {
                  SLine* sl = static_cast<const SLine*>(el);
//                  printf("slin segsz=%d", sl->lineSegments().size());
                  if (sl->lineSegments().size() > 0) {
                        LineSegment* seg = sl->lineSegments().at(0);
/*
                        printf(" x=%g y=%g w=%g h=%g cpx=%g cpy=%g userOff.y=%g\n",
                               seg->x(), seg->y(),
                               seg->width(), seg->height(),
                               seg->canvasPos().x(), seg->canvasPos().y(),
                               seg->userOff().y());
*/
                        System* sys = 0;
                        QPointF pnt = sl->tick2pos(0, el->tick(), el->staffIdx(), &sys);
                        if (sys) {
                              QRectF bb = sys->staff(el->staffIdx())->bbox();
/*
                              printf("syst x=%g y=%g cpx=%g cpy=%g\n",
                                     sys->pos().x(),  sys->pos().y(),
                                     sys->canvasPos().x(),
                                     sys->canvasPos().y()
                                    );
                              printf("staf x=%g y=%g w=%g h=%g\n",
                                     bb.x(), bb.y(),
                                     bb.width(), bb.height());
*/
                              // for the line type elements the reference point is vertically centered
                              // actual position info is in the segments
                              // compare the segment's canvas ypos with the staff's center height
                              if (seg->canvasPos().y() < sys->canvasPos().y() + bb.y() + bb.height() / 2)
                                     tagname += " placement=\"above\"";
                              else
                                     tagname += " placement=\"below\"";
                              }
                         }
                  }
            Element* pel = el->parent();
            if (pel) {
/*
                  printf("prnt tp=%d st=%d (%s,%s) x=%g y=%g w=%g h=%g userOff.y=%g\n",
                         pel->type(), pel->subtype(),
                         pel->name(), pel->subtypeName().toUtf8().data(),
                         pel->x(), pel->y(),
                         pel->width(), pel->height(),
                         pel->userOff().y());
*/
                  }
            // printf("\n");
            if (pel && pel->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(pel);
                  System* sys = m->system();
                  QRectF bb = sys->staff(el->staffIdx())->bbox();
/*
                  printf("syst x=%g y=%g cpx=%g cpy=%g\n",
                         sys->pos().x(),  sys->pos().y(),
                         sys->canvasPos().x(),
                         sys->canvasPos().y()
                        );
                  printf("staf x=%g y=%g w=%g h=%g\n",
                         bb.x(), bb.y(),
                         bb.width(), bb.height());
                  // element is above the staff if center of bbox is above center of staff
                  printf("center diff=%g\n", el->y() + el->height() / 2 - bb.y() - bb.height() / 2);
*/
                  if (el->y() + el->height() / 2 < bb.y() + bb.height() / 2)
                        tagname += " placement=\"above\"";
                  else
                        tagname += " placement=\"below\"";
                  }
//            printf("\n");
            }
      xml.stag(tagname);
      }

//---------------------------------------------------------
//   directionETag
//---------------------------------------------------------

static void directionETag(Xml& xml, int staff, int offs = 0)
      {
      if (offs)
            xml.tag("offset", offs);
      if (staff)
            xml.tag("staff", staff);
      xml.etag();
      }

//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

static void partGroupStart(Xml& xml, int number, int bracket)
      {
      xml.stag(QString("part-group type=\"start\" number=\"%1\"").arg(number));
      QString br = "";
      switch(bracket) {
            case BRACKET_NORMAL:
                  br = "bracket";
                  break;
            case BRACKET_AKKOLADE:
                  br = "brace";
                  break;
            default:
                  printf("bracket subtype %d not understood\n", bracket);
                  }
      if (br != "")
            xml.tag("group-symbol", br);
      xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

// a line containing only a note and zero or more dots
QRegExp metro("^[\\xe100\\xe101\\xe104-\\xe109][\\xe10a\\xe10b\\.]?$");
// a note, zero or more dots, zero or more spaces, an equals sign, zero or more spaces
QRegExp metroPlusEquals("[\\xe100\\xe101\\xe104-\\xe109][\\xe10a\\xe10b\\.]? ?= ?");
// a parenthesis open, zero or more spaces at end of line
QRegExp leftParen("\\( ?$");
// zero or more spaces, an equals sign, zero or more spaces at end of line
QRegExp equals(" ?= ?$");

static bool findUnitAndDots(QString words, QString& unit, int& dots)
      {
      unit = "";
      dots = 0;
//      printf("findUnitAndDots('%s') slen=%d", qPrintable(words), words.length());
      if (!metro.exactMatch(words)) { /* printf("\n"); */ return false; }
      switch (words.at(0).unicode()) {
            case 0xe100: unit = "breve"; break;
            case 0xe101: unit = "whole"; break;
            case 0xe104: unit = "half"; break;
            case 0xe105: unit = "quarter"; break;
            case 0xe106: unit = "eighth"; break;
            case 0xe107: unit = "16th"; break;
            case 0xe108: unit = "32nd"; break;
            case 0xe109: unit = "64th"; break;
            default: printf("findUnitAndDots: unknown char '%s'(0x%0xd)\n",
                            qPrintable(words.mid(0, 1)), words.at(0).unicode());
            }
      for (int i = 1; i < words.length(); ++i)
            switch (words.at(i).unicode()) {
                  case '.':    // fall through
                  case 0xe10a: ++dots; break;
                  case 0xe10b: ++dots; ++dots; break;
                  default: printf("findUnitAndDots: unknown char '%s'(0x%0xd)\n",
                                  qPrintable(words.mid(i, 1)), words.at(i).unicode());
                  }
//      printf(" unit='%s' dots=%d\n", qPrintable(unit), dots);
      return true;
      }

static bool findMetronome(QString words,
                          QString& wordsLeft,  // words left of metronome
                          bool& hasParen,      // parenthesis
                          QString& metroLeft,  // left part of metronome
                          QString& metroRight, // right part of metronome
                          QString& wordsRight  // words right of metronome
                         )
      {
//      printf("findMetronome('%s') slen=%d", qPrintable(words), words.length());
      wordsLeft  = "";
      hasParen   = false;
      metroLeft  = "";
      metroRight = "";
      wordsRight = "";
      int pos = metroPlusEquals.indexIn(words);
      if (pos != -1) {
            int len = metroPlusEquals.matchedLength();
/*
            printf(" mpos=%d mlen=%d\n",
                   pos, len
                  );
*/
            if (words.length() > pos + len) {
                  QString s1 = words.mid(0, pos);    // string to the left of metronome
                  QString s2 = words.mid(pos, len);  // first note and equals sign
                  QString s3 = words.mid(pos + len); // string to the right of equals sign
/*
                  printf("found metronome: '%s'%s'%s'",
                         qPrintable(s1),
                         qPrintable(s2),
                         qPrintable(s3)
                        );
*/
                  // determine if metronome has parentheses
                  // left part of string must end with parenthesis plus optional spaces
                  // right part of string must have parenthesis (but not in first pos)
                  int lparen = leftParen.indexIn(s1);
                  int rparen = s3.indexOf(")");
                  hasParen = (lparen != -1 && rparen > 0);
//                  printf(" lparen=%d rparen=%d hasP=%d", lparen, rparen, hasParen);
                  if (hasParen) wordsLeft = s1.mid(0, lparen);
                  else wordsLeft = s1;
                  int equalsPos = equals.indexIn(s2);
                  if (equalsPos != -1) metroLeft = s2.mid(0, equalsPos);
                  else printf("\ncan't find equals in s2\n");
                  if (hasParen) {
                        metroRight = s3.mid(0, rparen);
                        wordsRight = s3.mid(rparen + 1, s3.length() - rparen - 1);
                        }
                  else {
                        metroRight = s3;
                        }
/*
                  printf(" '%s'%s'%s'%s'\n",
                         qPrintable(wordsLeft),
                         qPrintable(metroLeft),
                         qPrintable(metroRight),
                         qPrintable(wordsRight)
                        );
*/
//                  bool res;
                  QString unit;
                  int dots;
                  return findUnitAndDots(metroLeft, unit, dots);
/*
                  printf("findUnitAndDots res=%d unit='%s' dots=%d\n",
                         res1, qPrintable(unit), dots);
                  res2 = findUnitAndDots(metroRight, unit, dots);
                  printf("findUnitAndDots res=%d unit='%s' dots=%d\n",
                         res2, qPrintable(unit), dots);
                  return res1;
*/
                  }
            }
      return false;
      }

static void wordsMetrome(Xml& xml, Text* text)
      {
      QString wordsLeft;  // words left of metronome
      bool hasParen;      // parenthesis
      QString metroLeft;  // left part of metronome
      QString metroRight; // right part of metronome
      QString wordsRight; // words right of metronome
      if (findMetronome(text->getText(), wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {
            if (wordsLeft != "") {
                  xml.stag("direction-type");
                  xml.tag("words", wordsLeft);
                  xml.etag();
                  }
            xml.stag("direction-type");
            xml.stag(QString("metronome parentheses=\"%1\"").arg(hasParen ? "yes" : "no"));
            QString unit;
            int dots;
            findUnitAndDots(metroLeft, unit, dots);
            xml.tag("beat-unit", unit);
            while (dots > 0) {
                  xml.tagE("beat-unit-dot");
                  --dots;
                  }
            if (findUnitAndDots(metroRight, unit, dots)) {
                  xml.tag("beat-unit", unit);
                  while (dots > 0) {
                        xml.tagE("beat-unit-dot");
                        --dots;
                        }
                  }
            else
                  xml.tag("per-minute", metroRight);
            xml.etag();
            xml.etag();
            if (wordsRight != "") {
                  xml.stag("direction-type");
                  xml.tag("words", wordsRight);
                  xml.etag();
                  }
            }
      else {
            xml.stag("direction-type");
            xml.tag("words", text->getText());
            xml.etag();
            }
      }

void ExportMusicXml::tempoText(TempoText* text, int staff)
      {
//      printf("ExportMusicXml::tempoText(TempoText='%s')\n", qPrintable(text->getText()));
      attr.doAttr(xml, false);
      xml.stag(QString("direction placement=\"%1\"").arg((text->parent()->y()-text->y() < 0.0) ? "below" : "above"));
      wordsMetrome(xml, text);
      int offs = text->mxmlOff();
      if (offs)
            xml.tag("offset", offs);
      if (staff)
            xml.tag("staff", staff);
      xml.tagE(QString("sound tempo=\"%1\"").arg(QString::number(text->tempo()*60.0)));
      xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportMusicXml::words(Text* text, int staff)
      {
/*
      printf("ExportMusicXml::words userOff.x=%f userOff.y=%f xoff=%g yoff=%g text='%s'\n",
             text->userOff().x(), text->userOff().y(), text->xoff(), text->yoff(),
             text->getText().toUtf8().data());
*/
      directionTag(xml, attr, text);
      if (text->subtypeName() == "RehearsalMark") {
            xml.stag("direction-type");
            xml.tag("rehearsal", text->getText());
            xml.etag();
            }
      else
            wordsMetrome(xml, text);
      directionETag(xml, staff, text->mxmlOff());
      }

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportMusicXml::hairpin(Hairpin* hp, int staff, int tick)
      {
      directionTag(xml, attr, hp);
      xml.stag("direction-type");
      if (hp->tick() == tick)
            xml.tagE("wedge type=\"%s\"", hp->subtype() ? "diminuendo" : "crescendo");
      else
            xml.tagE("wedge type=\"stop\"");
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   ottava
// <octave-shift type="down" size="8" relative-y="14"/>
// <octave-shift type="stop" size="8"/>
//---------------------------------------------------------

void ExportMusicXml::ottava(Ottava* ot, int staff, int tick)
      {
      int st = ot->subtype();
      directionTag(xml, attr, ot);
      xml.stag("direction-type");
      if (ot->tick() == tick) {
            const char* sz = 0;
            const char* tp = 0;
            switch(st) {
                  case 0:
                        sz = "8";
                        tp = "down";
                        break;
                  case 1:
                        sz = "15";
                        tp = "down";
                        break;
                  case 2:
                        sz = "8";
                        tp = "up";
                        break;
                  case 3:
                        sz = "15";
                        tp = "up";
                        break;
                  default:
                        printf("ottava subtype %d not understood\n", st);
                  }
            if (sz && tp)
                  xml.tagE("octave-shift type=\"%s\" size=\"%s\"", tp, sz);
            }
      else {
            if (st == 0 || st == 2)
                  xml.tagE("octave-shift type=\"stop\" size=\"8\"");
            else if (st == 1 || st == 3)
                  xml.tagE("octave-shift type=\"stop\" size=\"15\"");
            else
                  printf("ottava subtype %d not understood\n", st);
            }
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

void ExportMusicXml::pedal(Pedal* pd, int staff, int tick)
      {
      directionTag(xml, attr, pd);
      xml.stag("direction-type");
      if (pd->tick() == tick)
            xml.tagE("pedal type=\"start\" line=\"yes\"");
      else
            xml.tagE("pedal type=\"stop\" line=\"yes\"");
      xml.etag();
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   findBracket -- get index of bracket in bracket table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findBracket(const TextLine* tl) const
      {
      for (int i = 0; i < MAX_BRACKETS; ++i)
            if (bracket[i] == tl) return i;
      return -1;
      }

//---------------------------------------------------------
//   textLine
//---------------------------------------------------------

void ExportMusicXml::textLine(TextLine* tl, int staff, int tick)
      {
      QString rest;
      QPointF p;

      QString lineEnd = "none";
      QString type;
      int offs;
      int n = 0;
      if (tl->tick() == tick) {
            QString lineType;
            switch (tl->lineStyle()) {
                  case Qt::SolidLine:
                        lineType = "solid";
                        break;
                  case Qt::DashLine:
                        lineType = "dashed";
                        break;
                  case Qt::DotLine:
                        lineType = "dotted";
                        break;
                  default:
                        lineType = "solid";
                  }
            rest += QString(" line-type=\"%1\"").arg(lineType);
            p = tl->lineSegments().first()->userOff();
            offs = tl->mxmlOff();
            type = "start";
            }
      else {
            if (tl->endHook()) {
                  double h = tl->endHookHeight().val();
                  if (h < 0.0) {
                        lineEnd = "up";
                        h *= -1.0;
                        }
                  else {
                        lineEnd = "down";
                        }
                  rest += QString(" end-length=\"%1\"").arg(h * 10);
                  }
            p = tl->lineSegments().last()->userOff2();
            offs = tl->mxmlOff2();
            type = "stop";
            }

      n = findBracket(tl);
      if (n >= 0)
            bracket[n] = 0;
      else {
            n = findBracket(0);
            bracket[n] = tl;
            }

      if (p.x() != 0)
            rest += QString(" default-x=\"%1\"").arg(p.x() * 10 / tl->spatium());
      if (p.y() != 0)
            rest += QString(" default-y=\"%1\"").arg(p.y() * -10 / tl->spatium());

      directionTag(xml, attr, tl);
      if (tl->beginText() && tl->tick() == tick) {
            xml.stag("direction-type");
            xml.tag("words", tl->beginText()->getText());
            xml.etag();
            }
      xml.stag("direction-type");
      xml.tagE(QString("bracket type=\"%1\" number=\"%2\" line-end=\"%3\"%4").arg(type, QString::number(n + 1), lineEnd, rest));
      xml.etag();
      if (offs)
            xml.tag("offset", offs);
      if (staff)
            xml.tag("staff", staff);
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   dynamic
//---------------------------------------------------------

// In MuseScore dynamics are essentially user-defined texts, therefore the ones
// supported by MusicXML need to be filtered out. Everything not recognized
// as MusicXML dynamics is written as words.

void ExportMusicXml::dynamic(Dynamic* dyn, int staff)
      {
      QString t = dyn->getText();
      directionTag(xml, attr, dyn);
      xml.stag("direction-type");
      if (t == "p" || t == "pp" || t == "ppp" || t == "pppp" || t == "ppppp" || t == "pppppp"
       || t == "f" || t == "ff" || t == "fff" || t == "ffff" || t == "fffff" || t == "ffffff"
       || t == "mp" || t == "mf" || t == "sf" || t == "sfp" || t == "sfpp" || t == "fp"
       || t == "rf" || t == "rfz" || t == "sfz" || t == "sffz" || t == "fz") {
            xml.stag("dynamics");
            xml.tagE(t.toLatin1().data());
            xml.etag();
            }
      else if (t == "m" || t == "z") {
            xml.stag("dynamics");
            xml.tag("other-dynamics", t);
            xml.etag();
            }
      else
            xml.tag("words", t);
      xml.etag();
      directionETag(xml, staff, dyn->mxmlOff());
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

// TODO: remove dependency on symbol name and replace by a more stable interface
// changes in sym.cpp r2494 broke MusicXML export of pedals (again)

void ExportMusicXml::symbol(Symbol* sym, int staff)
      {
      QString name = symbols[sym->sym()].name();
      const char* mxmlName = "";
      if (name == "pedal ped")
            mxmlName = "pedal type=\"start\"";
      else if (name == "pedalasterisk")
            mxmlName = "pedal type=\"stop\"";
      else {
            printf("ExportMusicXml::symbol(): %s not supported", name.toLatin1().data());
            return;
            }
      directionTag(xml, attr, sym);
      xml.stag("direction-type");
      xml.tagE(mxmlName);
      xml.etag();
      directionETag(xml, staff, sym->mxmlOff());
      }

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

void ExportMusicXml::lyrics(const LyricsList* ll, const int trk)
      {
      for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
            if (*i) {
//                  printf("lyric trk %d\n", (*i)->track());
                  if ((*i)->track() == trk) {
                        xml.stag(QString("lyric number=\"%1\"").arg((*i)->no() + 1));
                        int syl   = (*i)->syllabic();
                        QString s = "";
                        switch(syl) {
                              case Lyrics::SINGLE: s = "single"; break;
                              case Lyrics::BEGIN:  s = "begin";  break;
                              case Lyrics::END:    s = "end";    break;
                              case Lyrics::MIDDLE: s = "middle"; break;
                              default:
                                    printf("unknown syllabic %d\n", syl);
                              }
                        xml.tag("syllabic", s);
                        xml.tag("text", (*i)->getText());
                        if((*i)->endTick() > 0)
                              xml.tagE("extend");
                        xml.etag();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   directionJump -- write jump
//---------------------------------------------------------

// LVIFIX: TODO coda and segno should be numbered uniquely

static void directionJump(Xml& xml, Jump* jp)
      {
      int jtp = jp->jumpType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (jtp == JUMP_DC) {
            if (jp->getText() == "")
                  words = "D.C.";
            else
                  words = jp->getText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == JUMP_DC_AL_FINE) {
            if (jp->getText() == "")
                  words = "D.C. al Fine";
            else
                  words = jp->getText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == JUMP_DC_AL_CODA) {
            if (jp->getText() == "")
                  words = "D.C. al Coda";
            else
                  words = jp->getText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == JUMP_DS_AL_CODA) {
            if (jp->getText() == "")
                  words = "D.S. al Coda";
            else
                  words = jp->getText();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == JUMP_DS_AL_FINE) {
            if (jp->getText() == "")
                  words = "D.S. al Fine";
            else
                  words = jp->getText();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == JUMP_DS) {
            words = "D.S.";
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else
            printf("jump type=%d not implemented\n", jtp);
      if (sound != "") {
            xml.stag("direction placement=\"above\"");
            xml.stag("direction-type");
            if (type != "") xml.tagE(type);
            if (words != "") xml.tag("words", words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   directionMarker -- write marker
//---------------------------------------------------------

static void directionMarker(Xml& xml, Marker* m)
      {
      int mtp = m->markerType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (mtp == MARKER_CODA) {
            type = "coda";
            if (m->label() == "")
                  sound = "coda=\"1\"";
            else
                  // LVIFIX hack: force label to "coda" to match to coda label
                  // sound = "coda=\"" + m->label() + "\"";
                  sound = "coda=\"coda\"";
            }
      else if (mtp == MARKER_SEGNO) {
            type = "segno";
            if (m->label() == "")
                  sound = "segno=\"1\"";
            else
                  sound = "segno=\"" + m->label() + "\"";
            }
      else if (mtp == MARKER_FINE) {
            words = "Fine";
            sound = "fine=\"yes\"";
            }
      else if (mtp == MARKER_TOCODA) {
            if (m->getText() == "")
                  words = "To Coda";
            else
                  words = m->getText();
            if (m->label() == "")
                  sound = "tocoda=\"1\"";
            else
                  sound = "tocoda=\"" + m->label() + "\"";
            }
      else
            printf("marker type=%d not implemented\n", mtp);
      if (sound != "") {
            xml.stag("direction placement=\"above\"");
            xml.stag("direction-type");
            if (type != "") xml.tagE(type);
            if (words != "") xml.tag("words", words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }

//---------------------------------------------------------
//  repeatAtMeasureStart -- write repeats at begin of measure
//---------------------------------------------------------

static void repeatAtMeasureStart(Xml& xml, Attributes& attr, Measure* m)
      {
      // loop over all measure relative elements in this measure
      // looking for JUMPS and MARKERS
      for (ciElement ci = m->el()->begin(); ci != m->el()->end(); ++ci) {
            Element* dir = *ci;
            int tp = dir->type();
            if (tp == JUMP) {
                  // note: all jumps are handled at measure stop
                  /*
                  Jump* jp = (Jump*) dir;
                  printf("repeatAtMeasureStart: jump st=%d jumpType=%d"
                         " getText=%s jumpTo=%s playUntil=%s continueAt=%s\n",
                         jp->subtype(), jp->jumpType(),
                         jp->getText().toLatin1().data(),
                         jp->jumpTo().toLatin1().data(),
                         jp->playUntil().toLatin1().data(),
                         jp->continueAt().toLatin1().data());
                  */
                  }
            else if (tp == MARKER) {
                  // filter out the markers at measure start
                  Marker* m = (Marker*) dir;
                  int mtp = m->markerType();
                  /*
                  printf("repeatAtMeasureStart: marker st=%d markerType=%d"
                         " getText=%s label=%s\n",
                         m->subtype(), mtp,
                         m->getText().toLatin1().data(),
                         m->label().toLatin1().data());
                  */
                  if (   mtp == MARKER_SEGNO
                      || mtp == MARKER_CODA
                     ) {
                        attr.doAttr(xml, false);
                        directionMarker(xml, m);
                        }
                  }
            }
      }

//---------------------------------------------------------
//  repeatAtMeasureStop -- write repeats at end of measure
//---------------------------------------------------------

static void repeatAtMeasureStop(Xml& xml, Measure* m)
      {
      // loop over all measure relative elements in this measure
      // looking for JUMPS and MARKERS
      for (ciElement ci = m->el()->begin(); ci != m->el()->end(); ++ci) {
            Element* dir = *ci;
            int tp = dir->type();
            if (tp == JUMP) {
                  // all jumps are handled at measure stop
                  Jump* jp = (Jump*) dir;
                  directionJump(xml, jp);
                  }
            else if (tp == MARKER) {
                  // filter out the markers at measure stop
                  Marker* m = (Marker*) dir;
                  int mtp = m->markerType();
                  if (   mtp == MARKER_FINE
                      || mtp == MARKER_TOCODA
                     ) {
                        directionMarker(xml, m);
                        }
                  }
            }
      }

//---------------------------------------------------------
//  work -- write the <work> element
//  note that order must be work-number, work-title
//  also write <movement-number> and <movement-title>
//  data is taken from the score metadata instead of the Text elements
//---------------------------------------------------------

void ExportMusicXml::work(const MeasureBase* /*measure*/)
      {
      if (!(score->workTitle().isEmpty() && score->workNumber().isEmpty())) {
            xml.stag("work");
            if (!score->workNumber().isEmpty())
                  xml.tag("work-number", score->workNumber());
            if (!score->workTitle().isEmpty())
                  xml.tag("work-title", score->workTitle());
            xml.etag();
            }
      if (!score->movementNumber().isEmpty())
            xml.tag("movement-number", score->movementNumber());
      if (!score->movementTitle().isEmpty())
            xml.tag("movement-title", score->movementTitle());
      }


//---------------------------------------------------------
//   elementRighter // used for harmony order
//---------------------------------------------------------

static bool elementRighter(const Element* e1, const Element* e2)
      {
      return e1->x() < e2->x();
      }

//---------------------------------------------------------
//  measureStyle -- write measure-style
//---------------------------------------------------------

// this is done at the first measure of a multi-meaure rest
// note: the measure count is stored in the last measure
// see measure.h _multiMeasure

static void measureStyle(Xml& xml, Attributes& attr, Measure* m)
      {
//      printf("measureStyle() multiMeasure=%d", m->multiMeasure());
      if (m->multiMeasure() > 0) {
            attr.doAttr(xml, true);
            xml.stag("measure-style");
            xml.tag("multiple-rest", m->multiMeasure());
            xml.etag();
            }
//      printf("\n");
      }

//---------------------------------------------------------
//  write
//---------------------------------------------------------

/**
 Write the score to \a dev in MusicXML format.
 */

void ExportMusicXml::write(QIODevice* dev)
      {

      calcDivisions();

/*
printf("gel contains:\n");
foreach(Element* el, *(score->gel())) {
      printf("%p type=%d(%s) tick=%d track=%d",
             el, el->type(), Element::name(el->type()), el->tick(), el->track());
      if (el->type() == OTTAVA) {
           Ottava * o = (Ottava *) el;
           printf(" tick2=%d", o->tick2());
           }
      if (el->type() == SLUR) {
           Slur * s = (Slur *) el;
           printf(" tick2=%d track2=%d", s->tick2(), s->track2());
           }
      if (el->type() == VOLTA) {
           Volta * v = (Volta *) el;
           printf(" tick2=%d text=%s endings=", v->tick2(), v->text().toLatin1().data());
           foreach(int i, v->endings()) printf(" %d", i);
           }
      printf("\n");
      }
*/

      for (int i = 0; i < MAX_BRACKETS; ++i)
            bracket[i] = 0;

      xml.setDevice(dev);
      xml.setCodec("utf8");
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml << "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 2.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
      xml.stag("score-partwise");

      const MeasureBase* measure = score->measures()->first();
      work(measure);

      // LVI TODO: write these text elements as credit-words
      // use meta data here instead
      xml.stag("identification");
      for (int i = 0; i < score->numberOfCreators(); ++i) {
/*
            printf("creator type='%s' text='%s'\n",
                   score->getCreator(i)->crType().toUtf8().data(),
                   score->getCreator(i)->crText().toUtf8().data()
                  );
*/
            const MusicXmlCreator* crt = score->getCreator(i);
            xml.tag(QString("creator type=\"%1\"").arg(crt->crType()), crt->crText());
            }
      if (!score->mxmlRights().isEmpty())
            xml.tag("rights", score->mxmlRights());
      xml.stag("encoding");
      if (debugMode) {
            xml.tag("software", QString("MuseScore 0.7.0"));
            xml.tag("encoding-date", QString("2007-09-10"));
            }
      else {
            xml.tag("software", QString("MuseScore ") + QString(VERSION));
            xml.tag("encoding-date", QDate::currentDate().toString(Qt::ISODate));
            }
      xml.etag();
      if (!score->source().isEmpty())
            xml.tag("source", score->source());
      xml.etag();

      // to keep most regression testfiles simple, write defaults and credits
      // in convertermode only when already present in the input file
      if (!converterMode || score->defaultsRead())
            defaults(xml, score, millimeters, tenths);
      if (!converterMode || score->creditsRead())
            credits(xml);

      xml.stag("part-list");
      const QList<Part*>* il = score->parts();
      int staffCount = 0;                       // count sum of # staves in parts
      int partGroupEnd[MAX_PART_GROUPS];        // staff where part group ends
      for (int i = 0; i < MAX_PART_GROUPS; i++)
            partGroupEnd[i] = -1;
      for (int idx = 0; idx < il->size(); ++idx) {
            Part* part = il->at(idx);
/*
            printf("part %d nstaves=%d (%d..%d)\n",
                   idx+1, part->nstaves(), staffCount, staffCount+part->nstaves()-1);
            for (int i = 0; i < part->nstaves(); i++) {
                  Staff* st = part->staff(i);
                  if (st) {
                        printf(" staff %d brLevels=%d\n", i+1, st->bracketLevels());
                        for (int j = 0; j < st->bracketLevels(); j++) {
                              // if (st->bracket(j) >= 0)
                                    printf("  j=%d br=%d brSpan=%d\n", j, st->bracket(j), st->bracketSpan(j));
                              }
                        }
                  }
*/
            for (int i = 0; i < part->nstaves(); i++) {
                  Staff* st = part->staff(i);
                  if (st) {
                        for (int j = 0; j < st->bracketLevels(); j++) {
                              if (st->bracket(j) != NO_BRACKET) {
                                    if (i == 0) {
                                          // OK, found bracket in first staff of part
                                          // filter out implicit brackets
                                          if (st->bracketSpan(j) != part->nstaves()) {
                                                // find part group number
                                                int number = 0;
                                                for (; number < MAX_PART_GROUPS; number++)
                                                      if (partGroupEnd[number] == -1)
                                                            break;
                                                if (number < MAX_PART_GROUPS) {
                                                      // printf("start number=%d end=%d\n", number + 1, idx + st->bracketSpan(j));
                                                      partGroupStart(xml, number + 1, st->bracket(j));
                                                      partGroupEnd[number] = idx + st->bracketSpan(j);
                                                      }
                                                else
                                                      printf("no free part group number\n");
                                                }
                                          }
                                    else {
                                          // bracket in other staff not supported in MusicXML
                                          printf("bracket starting in staff %d not supported\n", i + 1);
                                          }
                                    }
                              }
                        }
                  }

            xml.stag(QString("score-part id=\"P%1\"").arg(idx+1));
            xml.tag("part-name", part->longName()->getText());
            if(part->shortName() && ! part->shortName()->getText().isEmpty ())
                xml.tag("part-abbreviation", part->shortName()->getText());
            
            if (part->useDrumset()) {
                  Drumset* drumset = part->drumset();
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead >= 0) {
                              xml.stag(QString("score-instrument id=\"P%1-I%2\"").arg(idx+1).arg(i + 1));
                              xml.tag("instrument-name", di.name);
                              xml.etag();
                              }
                        }
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead >= 0) {
                              xml.stag(QString("midi-instrument id=\"P%1-I%2\"").arg(idx+1).arg(i + 1));
                              xml.tag("midi-channel", part->midiChannel() + 1);
                              xml.tag("midi-program", part->midiProgram() + 1);
                              xml.tag("midi-unpitched", i + 1);
                              xml.etag();
                              }
                        }
                  }
            else {
                  xml.stag(QString("score-instrument id=\"P%1-I%2\"").arg(idx+1).arg(3));
                  xml.tag("instrument-name", part->longName()->getText());
                  xml.etag();
      
                  xml.stag(QString("midi-instrument id=\"P%1-I%2\"").arg(idx+1).arg(3));
                  xml.tag("midi-channel", part->midiChannel() + 1);
                  xml.tag("midi-program", part->midiProgram() + 1);
                  xml.etag();
                  }
            xml.etag();
            staffCount += part->nstaves();
            for (int i = MAX_PART_GROUPS - 1; i >= 0; i--) {
                  int end = partGroupEnd[i];
                  if (end >= 0) {
                        if (staffCount >= end) {
                              // printf("end %d\n", i + 1);
                              xml.tagE("part-group type=\"stop\" number=\"%d\"", i + 1);
                              partGroupEnd[i] = -1;
                              }
                        }
                  }
            }
      xml.etag();

      staffCount = 0;

      for (int idx = 0; idx < il->size(); ++idx) {
            Part* part = il->at(idx);
            tick = 0;
            xml.stag(QString("part id=\"P%1\"").arg(idx+1));

            int staves = part->nstaves();
            int strack = score->staffIdx(part) * VOICES;
            int etrack = strack + staves * VOICES;

            DirectionsHandler dh(score);
            dh.buildDirectionsList(part, strack, etrack);

            int measureNo = 1;          // number of next regular measure
            int irregularMeasureNo = 1; // number of next irregular measure
            int pickupMeasureNo = 1;    // number of next pickup measure

            for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;
                  PageFormat* pf = score->pageFormat();


                  // printf("measureNo=%d\n", measureNo);
                  // pickup and other irregular measures need special care
                  QString measureTag = "measure number=";
                  if ((irregularMeasureNo + measureNo) == 2 && m->irregular()) {
                        measureTag += "\"0\" implicit=\"yes\"";
                        pickupMeasureNo++;
                        }
                  else if (m->irregular())
                        measureTag += QString("\"X%1\" implicit=\"yes\"").arg(irregularMeasureNo++);
                  else
                        measureTag += QString("\"%1\"").arg(measureNo++);
                  if (!converterMode || score->defaultsRead())
                        measureTag += QString(" width=\"%1\"").arg(QString::number(m->bbox().width() / DPMM / millimeters * tenths,'f',2));
                  xml.stag(measureTag);

                  int currentSystem = NoSystem;
                  Measure* previousMeasure = 0;

                  for (MeasureBase* currentMeasureB = m->prev(); currentMeasureB; currentMeasureB = currentMeasureB->prev()){
                      if (currentMeasureB->type() == MEASURE) {
                          previousMeasure = (Measure*) currentMeasureB;
                          break;
                          }
                      }

                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4)
                        currentSystem = TopSystem;
                  else if ((measureNo > 2 && int(m->canvasPos().x() / DPI / pf->width()) != int(previousMeasure->canvasPos().x() / DPI / pf->width())))    // TODO: MeasureBase
                        currentSystem = NewPage;
                  else if (previousMeasure &&
                        m->canvasPos().y() > (previousMeasure->canvasPos().y()))  // TODO: MeasureBase
                        currentSystem = NewSystem;

                  if (currentSystem != NoSystem) {
                      if (!converterMode || score->defaultsRead()) {
                          const double pageWidth  = getTenthsFromInches(pf->width());
                          const double lm = getTenthsFromInches(pf->oddLeftMargin);
                          const double rm = getTenthsFromInches(pf->oddRightMargin);
                          const double tm = getTenthsFromInches(pf->oddTopMargin);

                          if (currentSystem == TopSystem)
                              xml.stag("print");
                          else if (currentSystem == NewSystem)
                              xml.stag("print new-system=\"yes\"");
                          else if (currentSystem == NewPage)
                              xml.stag("print new-page=\"yes\"");

                          // System Layout
                          // Put the system print suggestions only for the first part in a score...
                          if (idx == 0) {
                              // Find the right margin of the system.
                              double systemLM = getTenthsFromDots(m->canvasPos().x() - m->system()->page()->canvasPos().x()) - lm;
                              double systemRM = pageWidth - rm - (getTenthsFromDots(m->system()->bbox().width()) + lm);

                              xml.stag("system-layout");
                              xml.stag("system-margins");
                              xml.tag("left-margin", QString("%1").arg(QString::number(systemLM,'f',2)));
                              xml.tag("right-margin", QString("%1").arg(QString::number(systemRM,'f',2)) );
                              xml.etag();

                              if (currentSystem == NewPage || currentSystem == TopSystem)
                                  xml.tag("top-system-distance", QString("%1").arg(QString::number(getTenthsFromDots(m->canvasPos().y()) - tm,'f',2)) );
                              else if (currentSystem == NewSystem)
                                  xml.tag("system-distance", QString("%1").arg(QString::number(getTenthsFromDots(m->canvasPos().y() - previousMeasure->canvasPos().y() - previousMeasure->bbox().height()),'f',2)));

                              xml.etag();
                              }

                          // Staff layout elements.
                          for (int staffIdx = (staffCount == 0) ? 1 : 0; staffIdx < staves; staffIdx++) {
                              xml.stag(QString("staff-layout number=\"%1\"").arg(staffIdx + 1));
                              xml.tag("staff-distance", QString("%1").arg(QString::number(getTenthsFromDots(mb->score()->point(mb->system()->staff(staffCount + staffIdx - 1)->distance())),'f',2)));
                              xml.etag();
                              }

                          xml.etag();
                          } // if (!converterMode ...

                      else {
                          if (currentSystem == NewSystem)
                              xml.tagE("print new-system=\"yes\"");
                          else if (currentSystem == NewPage)
                              xml.tagE("print new-page=\"yes\"");
                          } // if (!converterMode ...
                      } // if (currentSystem ...

                  attr.start();
                  dh.buildDirectionsList(m, false, part, strack, etrack);

                  // barline left must be the first element in a measure
                  barlineLeft(m);

                  // output attributes with the first actual measure (pickup or regular)
                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                        attr.doAttr(xml, true);
                        xml.tag("divisions", AL::division / div);
                        }
                  // output attributes at start of measure: key, time
                  KeySig* ksig = 0;
                  TimeSig* tsig = 0;
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        if (seg->tick() > m->tick())
                              break;
                        Element* el = seg->element(strack);
                        if (!el)
                              continue;
                        if (el->type() == KEYSIG)
                              ksig = (KeySig*) el;
                        else if (el->type() == TIMESIG)
                              tsig = (TimeSig*) el;
                        }
                  if (ksig) {
                        // output only keysig changes, not generated keysigs
                        // at line beginning
                        int ti = ksig->tick();
                        //TODO_K
                        KeyList* kl = score->staff(strack/VOICES)->keymap();
                        KeySigEvent key = kl->key(ti);
                        ciKeyList ci = kl->find(ti);
                        if (ci != kl->end()) {
                              keysig(key.accidentalType(), ksig->visible());
                              }
                        }
                  else if (tick == 0)
                        // always write a keysig at tick = 0
                        keysig(0);
                  if (tsig) {
                        // int z, n;
                        // score->sigmap()->timesig(tsig->tick(), z, n);
                        timesig(tsig);
                        }
                  // output attributes with the first actual measure (pickup or regular) only
                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                        if (staves > 1)
                              xml.tag("staves", staves);
                        }
                  // output attribute at start of measure: clef
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
//                        printf("segment %s %s at tick %d\n",
//                               seg->name(), seg->subtypeName().toUtf8().data(), seg->tick());
                        if (seg->tick() > m->tick())
                              break;
                        Element* el = seg->element(strack);
                        if (!el)
                              continue;
                        if (el->type() == CLEF)
                              for (int st = strack; st < etrack; st += VOICES) {
                                    // sstaff - xml staff number, counting from 1 for this
                                    // instrument
                                    // special number 0 -> dont show staff number in
                                    // xml output (because there is only one staff)

                                    int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                                    sstaff /= VOICES;
                                    // printf("strack=%d etrack=%d st=%d sstaff=%d\n", strack, etrack, st, sstaff);
                                    el = seg->element(st);
                                    if (el && el->type() == CLEF) {
                                          // output only clef changes, not generated clefs
                                          // at line beginning
                                          int ti = el->tick();
                                          int ct = ((Clef*)el)->subtype();
//                                          printf("exportxml: clef ti=%d ct=%d\n", ti, ct);
                                          ClefList* cl = score->staff(st/VOICES)->clefList();
                                          ciClefEvent ci = cl->find(ti);
                                          if (ci != cl->end()) {
                                                clef(sstaff, ct);
                                                }
                                          }
                                    }
                        }
                   // output attributes with the first actual measure (pickup or regular) only
                   if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                      for (int i = 0; i < staves; i++) {
                        Staff* st = part->staff(i);
                        if(st->lines() != 5){
                            if (staves > 1)
                                xml.stag(QString("staff-details number=\"%1\"").arg(i+1));
                            else
                                xml.stag("staff-details");
                            xml.tag("staff-lines", st->lines());
                            xml.etag();
                          }
                        }
                        if (part->transpose().chromatic) {
                          xml.stag("transpose");
                          xml.tag("diatonic",  part->transpose().diatonic);
                          xml.tag("chromatic", part->transpose().chromatic);
                          xml.etag();
                        }
                      }

                  // output attribute at start of measure: measure-style
                  measureStyle(xml, attr, m);

                  // MuseScore limitation: repeats are always in the first part
                  // and are implicitly placed at either measure start or stop
                  if (idx == 0)
                        repeatAtMeasureStart(xml, attr, m);

                  for (int st = strack; st < etrack; ++st) {
                        // sstaff - xml staff number, counting from 1 for this
                        // instrument
                        // special number 0 -> dont show staff number in
                        // xml output (because there is only one staff)

                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;
                        // printf("strack=%d etrack=%d st=%d sstaff=%d\n", strack, etrack, st, sstaff);

                        for (Segment* seg = m->first(); seg; seg = seg->next()) {
                              Element* el = seg->element(st);
                              if (!el)
                                    continue;
                              // must ignore start repeat to prevent spurious backup/forward
                              if (el->type() == BAR_LINE && el->subtype() == START_REPEAT)
                                    continue;

                              // look for harmony element for this tick position
                              if (el->isChordRest()) {
                                    QList<Element*> list;
                                    ChordRest* cr = nextChordRest(static_cast<ChordRest*>(el));
                                    int endTick = el->tick();
                                    if (cr) {
                                        endTick = cr->tick();
                                        printf("CR %d %d\n", endTick, el->tick());
                                        }
                                    foreach(Element* he, *m->el()) {
                                    printf("staff %d %d tick %d\n", he->staffIdx(), sstaff, he->tick());
                                          if ((he->type() == HARMONY) && (he->staffIdx() == sstaff)
                                             && (he->tick() >= el->tick()) && (he->tick() <= endTick)) {
                                                list << he;
                                                }
                                          }

                                    qSort(list.begin(), list.end(), elementRighter);

                                    foreach (Element* hhe, list){
                                          attr.doAttr(xml, false);
                                          harmony((Harmony*)hhe, el);
                                          }
                                    }

                              // generate backup or forward to the start time of the element
                              // but not for breath, which has the same start time as the
                              // previous note, while tick is already at the end of that note
                              if (tick != el->tick()) {
                                    attr.doAttr(xml, false);
                                    if (el->type() != BREATH) moveToTick(el->tick());
                                    }
/*
                              if (el->isChordRest()) {
                                    printf("isChordRest oldtick=%d", tick);
                                    tick += static_cast<ChordRest*>(el)->ticks();
                                    printf(" newtick=%d\n", tick);
                                    }
*/
                              dh.handleElement(this, el, sstaff, true);
                              switch (el->type()) {
                                    case CLEF:
                                          {
                                          // output only clef changes, not generated clefs
                                          // at line beginning
                                          // also ignore clefs at the start o a measure,
                                          // these have already been output
                                          int ti = el->tick();
                                          int ct = ((Clef*)el)->subtype();
                                          ClefList* cl = score->staff(st/VOICES)->clefList();
                                          ciClefEvent ci = cl->find(ti);
                                          if (ci != cl->end() && el->tick() != m->tick()) {
                                                clef(sstaff, ct);
                                                }
                                          }
                                          break;
                                    case KEYSIG:
                                          {
                                          /* ignore
                                          // output only keysig changes, not generated keysigs
                                          // at line beginning
                                          int ti = el->tick();
                                          int key = score->keymap->key(el->tick());
                                          KeyList* kl = score->keymap;
                                          ciKeyList ci = kl->find(ti);
                                          if (ci != kl->end()) {
                                                keysig(key);
                                                }
                                          */
                                          }
                                          break;
                                    case TIMESIG:
                                          {
                                          /* ignore
                                          // output only for staff 0
                                          if (st == 0) {
                                                int z, n;
                                                score->sigmap()->timesig(el->tick(), z, n);
                                                timesig(z, n);
                                                }
                                          */
                                          }
                                          break;
                                    case CHORD:
                                          {
                                          // note: in MuseScore there is one lyricslist for each staff
                                          // MusicXML associates lyrics with notes in a specific voice
                                          // (too) simple solution: output lyrics only for the first voice
                                          const LyricsList* ll = 0;
                                          /* if ((st % VOICES) == 0) */ ll = seg->lyricsList(st / VOICES);
                                          chord((Chord*)el, sstaff, ll, part->useDrumset());
                                          break;
                                          }
                                    case REST:
                                          rest((Rest*)el, sstaff);
                                          break;
                                    case BAR_LINE:
                                          // Following must be enforced (ref MusicXML barline.dtd):
                                          // If location is left, it should be the first element in the measure;
                                          // if location is right, it should be the last element.
                                          // implementation note: START_REPEAT already written by barlineLeft()
                                          // any bars left should be "middle"
                                          // TODO: print barline only if middle
                                          // if (el->subtype() != START_REPEAT)
                                          //       bar((BarLine*) el);
                                          break;
                                    case BREATH:
                                          // ignore, already exported as note articulation
                                          break;

                                    default:
                                          printf("ExportMusicXml::write unknown segment type %s\n", el->name());
                                          break;
                                    }
                              dh.handleElement(this, el, sstaff, false);
                              }
                        attr.stop(xml);
                        if (!((st + 1) % VOICES)) {
                              // sstaff may be 0, which causes a failed assertion (and abort)
                              // in (*i)->staff(ssstaff - 1)
                              // LVIFIX: find exact cause
                              int ssstaff = sstaff > 0 ? sstaff : sstaff + 1;
                              // printf("st=%d sstaff=%d ssstaff=%d\n", st, sstaff, ssstaff);
                              dh.handleElements(this, part->staff(ssstaff - 1), m->tick(), m->tick() + m->tickLen(), sstaff);
                              }
                        }
                  // move to end of measure (in case of incomplete last voice)
//                  printf("end of measure\n");
                  moveToTick(m->tick() + m->tickLen());
                  if (idx == 0)
                        repeatAtMeasureStop(xml, m);
                  // note: don't use "m->repeatFlags() & RepeatEnd" here, because more
                  // barline types need to be handled besides repeat end ("light-heavy")
                  barlineRight(m);
                  xml.etag();
                  }
            staffCount += staves;
            xml.etag();
            }

      xml.etag();
      }

//---------------------------------------------------------
//   saveXml
//    return false on error
//---------------------------------------------------------

/**
 Save Score as MusicXML file \a name.

 Return false on error.
 */

bool Score::saveXml(const QString& name)
      {
      QFile f(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;
      ExportMusicXml em(this);
      em.write(&f);
      return f.error() == QFile::NoError;
      }


//---------------------------------------------------------
//   saveMxl
//    return false on error
//---------------------------------------------------------

/**
 Save Score as compressed MusicXML file \a name.

 Return false on error.
 */

// META-INF/container.xml:
// <?xml version="1.0" encoding="UTF-8"?>
// <container>
//     <rootfiles>
//         <rootfile full-path="testHello.xml"/>
//     </rootfiles>
// </container>

bool Score::saveMxl(const QString& name)
      {
//      printf("Score::saveMxl(%s)\n", name.toUtf8().data());

      Zip::ErrorCode ec;
      Zip uz;

      ec = uz.createArchive(name);
      if (ec != Zip::Ok) {
            printf("Cannot create zipfile '%s'\n", name.toUtf8().data());
            return false;
            }
      QFileInfo fi(name);
      QDateTime dt;
      if (debugMode)
            dt = QDateTime(QDate(2007, 9, 10), QTime(12, 0));
      else
            dt = QDateTime::currentDateTime();
      QString fn = fi.completeBaseName() + ".xml";

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      Xml xml;
      xml.setDevice(&cbuf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(fn));
      xml.etag();
      xml.etag();
      xml.etag();
//      printf("bufsize=%d\n", cbuf.data().size());
//      printf("data=%s\n", cbuf.data().data());
      cbuf.seek(0);
      ec = uz.createEntry("META-INF/container.xml", cbuf, dt);
      if (ec != Zip::Ok) {
            printf("Cannot add container.xml to zipfile '%s'\n", name.toUtf8().data());
            return false;
            }

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      ExportMusicXml em(this);
      em.write(&dbuf);
//      printf("bufsize=%d\n", dbuf.data().size());
//      printf("data=%s\n", dbuf.data().data());
      dbuf.seek(0);
      ec = uz.createEntry(fn, dbuf, dt);
      if (ec != Zip::Ok) {
            printf("Cannot add %s to zipfile '%s'\n", fn.toUtf8().data(), name.toUtf8().data());
            return false;
            }

      ec = uz.closeArchive();
      if (ec != Zip::Ok) {
            printf("Cannot close zipfile '%s'\n", name.toUtf8().data());
            return false;
            }

      return true;
      }

double ExportMusicXml::getTenthsFromInches(double inches){
    return inches * INCH / millimeters * tenths;
}

double ExportMusicXml::getTenthsFromDots(double dots){
    return dots / DPMM / millimeters * tenths;
}


//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

void ExportMusicXml::harmony(Harmony* h, Element* e)
      {
      int rootTpc = h->rootTpc();
      if (rootTpc != INVALID_TPC) {
    	    double rx = h->userOff().x()*10;
    	    QString relative;
    	    if (rx > 0){
    	    	relative = QString(" relative-x=\"%1\"").arg(QString::number(rx,'f',2));
    	    }
            if (h->frameWidth() > 0.0)
                  xml.stag(QString("harmony print-frame=\"yes\"").append(relative));
            else
                  xml.stag(QString("harmony print-frame=\"no\"").append(relative));
            xml.stag("root");
            xml.tag("root-step", tpc2stepName(rootTpc));
            int alter = tpc2alter(rootTpc);
            if (alter)
                  xml.tag("root-alter", alter);
            xml.etag();

            if (!h->xmlKind().isEmpty()) {
                  xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), h->xmlKind());
                  QStringList l = h->xmlDegrees();
                  if (!l.isEmpty()) {
                        foreach(QString tag, l) {
                              xml.stag("degree");
                              int alter = 0;
                              int idx = 3;
                              if (tag[idx] == '#') {
                                    alter = 1;
                                    ++idx;
                                    }
                              else if (tag[idx] == 'b') {
                                    alter = -1;
                                    ++idx;
                                    }
                              xml.tag("degree-value", tag.mid(idx));
                              xml.tag("degree-alter", alter);     // finale insists on this even if 0
                              if (tag.startsWith("add"))
                                    xml.tag("degree-type", "add");
                              else if (tag.startsWith("sub"))
                                    xml.tag("degree-type", "subtract");
                              else if (tag.startsWith("alt"))
                                    xml.tag("degree-type", "alter");
                              xml.etag();
                              }
                        }
                  }
            else {
                  if (h->extensionName() == 0)
                        xml.tag("kind", "");
                  else
                        xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), "");
                  }

            int baseTpc = h->baseTpc();
            if (baseTpc != INVALID_TPC) {
                  xml.stag("bass");
                  xml.tag("bass-step", tpc2stepName(baseTpc));
                  int alter = tpc2alter(baseTpc);
                  if (alter) {
                        xml.tag("bass-alter", alter);
                        }
                  xml.etag();
                  }
            if(e->tick() < h->tick())
                  xml.tag("offset", (h->tick() - e->tick()) / div);
            xml.etag();
            }
      else {
            //
            // export an unrecognized Chord
            // which may contain arbitrary text
            //
            xml.stag("direction");
            xml.stag("direction-type");
            xml.tag("words default-y=\"100\"", h->getText());
            xml.etag();
            xml.etag();
            }
#if 0
      xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), extension);
      for (int i = 0; i < h->numberOfDegrees(); i++) {
            HDegree hd = h->degree(i);
            int tp = hd.type();
            if (tp == ADD || tp == ALTER || tp == SUBTRACT) {
                  xml.stag("degree");
                  xml.tag("degree-value", hd.value());
                  xml.tag("degree-alter", hd.alter());
                  switch (tp) {
                        case ADD:
                              xml.tag("degree-type", "add");
                              break;
                        case ALTER:
                              xml.tag("degree-type", "alter");
                              break;
                        case SUBTRACT:
                              xml.tag("degree-type", "subtract");
                              break;
                        default:
                              break;
                        }
                  xml.etag();
                  }
            }
#endif
      }


