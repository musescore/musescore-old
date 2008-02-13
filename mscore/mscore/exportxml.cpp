//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportxml.cpp,v 1.71 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "config.h"
#include "mscore.h"
#include "file.h"
#include "score.h"
#include "rest.h"
#include "chord.h"
#include "sig.h"
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
#include "layout.h"
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
      Slur* slur[MAX_SLURS];
      bool started[MAX_SLURS];
      int findSlur(Slur* s);

   public:
      SlurHandler();
      void doSlurStart(Chord* chord, Notations& notations, Xml& xml);
      void doSlurStop(Chord* chord, Notations& notations, Xml& xml);
      };

//---------------------------------------------------------
//   ExportMusicXml
//---------------------------------------------------------

class ExportMusicXml {
      Score* score;
      Xml xml;
      SlurHandler sh;
      int tick;
      Attributes attr;
      void chord(Chord* chord, int staff, const LyricsList* ll);
      void rest(Rest* chord, int staff);
      void clef(int staff, int clef);
      void timesig(TimeSig* tsig);
      void keysig(int key);
      void barlineLeft(Measure* m);
      void barlineRight(Measure* m);
      void pitch2xml(Note* note, char& c, int& alter, int& octave);
      void lyrics(const LyricsList* ll);

   public:
      ExportMusicXml(Score* s) { score = s; tick = 0; }
      void write(QIODevice* dev);
      void moveToTick(int t);
      void words(Text* text, int staff);
      void hairpin(Hairpin* hp, int staff, int tick);
      void ottava(Ottava* ot, int staff, int tick);
      void pedal(Pedal* pd, int staff, int tick);
      void dynamic(Dynamic* dyn, int staff);
      void symbol(Symbol * sym, int staff);
      void tempoText(TempoText* text, int staff);
      void harmony(Harmony*);
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
      for (int i = 0; i < MAX_SLURS; ++i) {
            slur[i] = 0;
            started[i] = false;
            }
      }

//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//---------------------------------------------------------

int SlurHandler::findSlur(Slur* s)
      {
      for (int i = 0; i < MAX_SLURS; ++i)
            if (slur[i] == s) return i;
      return -1;
      }

//---------------------------------------------------------
//   doSlurStart
//---------------------------------------------------------

void SlurHandler::doSlurStart(Chord* chord, Notations& notations, Xml& xml)
      {
      // search for slur(s) starting at this chord
      foreach(Element* el, *(chord->score()->gel())) {
            if (el->type() == SLUR) {
                  Slur* s = (Slur*) el;
                  if (s->startsAt(chord->tick(), chord->track())) {
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
      foreach(Element* el, *(chord->score()->gel())) {
            if (el->type() == SLUR) {
                  Slur* s = (Slur*) el;
                  if (s->endsAt(chord->tick(), chord->track())) {
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
                  }
            }
      // search slur list for already started slur(s) stopping at this chord
      for (int i = 0; i < MAX_SLURS; ++i) {
            if (slur[i] && slur[i]->endsAt(chord->tick(), chord->track())) {
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

//---------------------------------------------------------
//   LVI FIXME:
//   - replace fix-sized anchors[] by something dynamically sized
//---------------------------------------------------------

class DirectionsHandler {
      Score *cs;
      static const int MAX_ANCHORS = 50;
      int nextAnchor;
      DirectionsAnchor* anchors[MAX_ANCHORS];
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
      for (int i = 0; i < MAX_ANCHORS; i++) anchors[i] = 0;
      }

//---------------------------------------------------------
//   storeAnchor
//---------------------------------------------------------

void DirectionsHandler::storeAnchor(DirectionsAnchor* a)
      {
      if (nextAnchor < MAX_ANCHORS) anchors[nextAnchor++] = a;
      else printf("DirectionsHandler: too many directions");
      }

//---------------------------------------------------------
//   handleElement -- handle all directions attached to a specific element
//---------------------------------------------------------

void DirectionsHandler::handleElement(ExportMusicXml* exp, Element* el, int sstaff, bool start)
      {
      for (int i = 0; i < MAX_ANCHORS; i++)
            if (DirectionsAnchor* da = anchors[i])
                  if (da->getAnchor() && da->getAnchor() == el && da->getStart() == start) {
                        Element* dir = da->getDirect();
                        switch(dir->type()) {
                              case SYMBOL:
                                    exp->symbol((Symbol *) dir, sstaff);
                                    break;
                              case TEMPO_TEXT:
                                    exp->tempoText((TempoText*) dir, sstaff);
                                    break;
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
                              default:
                                    printf("DirectionsHandler::handleElement: direction type %s at tick %d not implemented\n",
                                            elementNames[dir->type()], da->getTick());
                                    break;
                              }
                        delete anchors[i];
                        anchors[i] = 0;
                        }
      }

//---------------------------------------------------------
//   handleElements -- handle all directions at tick between mstart and mend
//---------------------------------------------------------

void DirectionsHandler::handleElements(ExportMusicXml* exp, Staff* staff, int mstart, int mend, int sstaff)
      {
      for (int i = 0; i < MAX_ANCHORS; i++)
            if (DirectionsAnchor* da = anchors[i]) {
                  Element* dir = da->getDirect();
                  if (dir && dir->staff() == staff && mstart <= da->getTick() && da->getTick() < mend) {
                        exp->moveToTick(da->getTick());
                        switch(dir->type()) {
                              case SYMBOL:
                                    exp->symbol((Symbol *) dir, sstaff);
                                    break;
                              case TEMPO_TEXT:
                                    exp->tempoText((TempoText*) dir, sstaff);
                                    break;
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
                              default:
                                    printf("DirectionsHandler::handleElements: direction type %s at tick %d not implemented\n",
                                            elementNames[dir->type()], da->getTick());
                                    break;
                              }
                        delete anchors[i];
                        anchors[i] = 0;
                        }
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
                  if (el->isChordRest() && el->staff() == stf)
                        if (   ( start && el->tick() == tick)
                            || (!start && (el->tick() + el->tickLen()) == tick)) {
                              return new DirectionsAnchor(el, start, tick);
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
      if (st->part() == p)
            return new DirectionsAnchor(tick);
      else return 0;
      }

//---------------------------------------------------------
//   findSpecificMatchInPart -- find chord or rest in part
//     starting or ending at tick
//---------------------------------------------------------

static DirectionsAnchor* findSpecificMatchInPart(int tick, Staff* st, bool start, Score* sc, int strack, int etrack)
      {
      for (MeasureBase* mb = sc->mainLayout()->first(); mb; mb = mb->next()) {
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
//---------------------------------------------------------

static DirectionsAnchor* findMatchInPart(int tick, Staff* st, Score* sc, Part* p, int strack, int etrack)
      {
      DirectionsAnchor* da;
      da = findSpecificMatchInPart(tick, st, true, sc, strack, etrack);
      if (da)
            return da;
      da = findSpecificMatchInPart(tick, st, false, sc, strack, etrack);
      if (da)
            return da;
      return (st->part() == p) ? new DirectionsAnchor(tick) : 0;
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
                        {
                        Hairpin* hp = (Hairpin*) dir;
                        da = findMatchInPart(hp->tick(), hp->staff(), cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        da = findMatchInPart(hp->tick2(), hp->staff(), cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        }
                        break;
                  case OTTAVA:
                        {
                        Ottava* ot = (Ottava*) dir;
                        da = findMatchInPart(ot->tick(), ot->staff(), cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        da = findMatchInPart(ot->tick2(), ot->staff(), cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        }
                        break;
                  case PEDAL:
                        {
                        Pedal* pd = (Pedal*) dir;
                        da = findMatchInPart(pd->tick(), pd->staff(), cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        da = findMatchInPart(pd->tick2(), pd->staff(), cs, p, strack, etrack);
                        if (da) {
                              da->setDirect(dir);
                              storeAnchor(da);
                              }
                        }
                        break;
                  default:
                        // all others silently ignored
                        // printf("DirectionsHandler::buildDirectionsList: direction type %s not implemented\n",
                        //        elementNames[dir->type()]);
                        break;
                  }
            }
      // part-level elements stored in measures
      for (MeasureBase* mb = cs->mainLayout()->first(); mb; mb = mb->next()) {
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
                        //        elementNames[dir->type()]);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

void ExportMusicXml::pitch2xml(Note* note, char& c, int& alter, int& octave)
      {
      static char table1[]  = "FEDCBAG";

      int tick   = note->chord()->tick();
      Staff* i   = note->staff();
      int offset = clefTable[i->clef()->clef(tick)].yOffset;

      int step   = (note->line() - offset + 700) % 7;
      c          = table1[step];

      int pitch  = note->pitch() - 12;
      octave     = pitch / 12;

      static int table2[7] = { 5, 4, 2, 0, 11, 9, 7 };
      int npitch = table2[step] + (octave + 1) * 12;
      alter      = note->pitch() - npitch;

      if (alter > 2) {
            printf("pitch2xml problem: alter %d step %d(line %d) octave %d\n",
               alter, step, note->line(), octave);
//HACK:
            alter  -= 12;
            octave += 1;
            }
      }

//---------------------------------------------------------
//   tick2xml
//    set type + dots depending on tick len
//---------------------------------------------------------

static QString tick2xml(const int ticks, int& dots)
      {
      QString type;
      dots = 0;
      if (ticks == 16*division)       // 4/1
            type = "long";
      else if (ticks == 8*division)        // 2/1
            type = "breve";
      else if (ticks == 6*division) {        // 1/1 + 1/2  (does this exist???)
            type = "whole";
            dots  = 1;
            }
      else if (ticks == 5*division)        // HACK
            type = "whole";
      else if (ticks == 4*division)        // 1/1
            type = "whole";
      else if (ticks == 3*division) {        // 1/2
            dots = 1;
            type = "half";
            }
      else if (ticks == 2*division)        // 1/2
            type = "half";
      else if (ticks == division + division/2) {
            dots = 1;
            type = "quarter";
            }
      else if (ticks == division)          // quarternote
            type = "quarter";
      else if (ticks == division/2 + division/4) {
            dots = 1;
            type = "eighth";
            }
      else if (ticks == division/2)        // 1/8
            type = "eighth";
      else if (ticks == division/4 + division/8) {
            dots = 1;
            type = "16th";
            }
      else if (ticks == division/4)        // 1/16
            type = "16th";
      else if (ticks == division/8 + division/16) {
            dots = 1;
            type = "32nd";
            }
      else if (ticks == division/8)        // 1/32
            type = "32nd";
      else if (ticks == division/16 + division/32) {
            dots = 1;
            type = "64th";
            }
      else if (ticks == division/16)       // 1/64
            type = "64th";
      else if (ticks == division/32 + division/64) {
            dots = 1;
            type = "128th";
            }
      else if (ticks == division/32)       // 1/128
            type = "128th";
      else if (ticks == division/64 + division/128) {
            dots = 1;
            type = "256th";
            }
      else if (ticks == division/64)       // 1/128
            type = "256th";
      else {
            fprintf(stderr, "tick2xml: invalid note len %d (%d, %d)\n",
               ticks, ticks/division, ticks%division);
            }
      return type;
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
      bool needBarStyle = (bst != NORMAL_BAR && bst != START_REPEAT);
      Volta* volta = findVolta(m, false);
      if (!needBarStyle && !volta)
            return;
      xml.stag(QString("barline location=\"right\""));
      if (needBarStyle) {
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
      if (t < tick) {
            attr.doAttr(xml, false);
            xml.stag("backup");
            xml.tag("duration", tick - t);
            xml.etag();
            }
      else if (t > tick) {
            attr.doAttr(xml, false);
            xml.stag("forward");
            xml.tag("duration", t - tick);
            xml.etag();
            }
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

void ExportMusicXml::keysig(int key)
      {
      attr.doAttr(xml, true);
      xml.stag("key");
      xml.tag("fifths", key);
      xml.tag("mode", QString("major"));
      xml.etag();
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

void ExportMusicXml::clef(int staff, int clef)
      {
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
      if (cr == t->elements()->front()) {
            notations.tag(xml);
            xml.tagE("tuplet type=\"start\" bracket=\"%s\"", t->hasLine() ? "yes" : "no");
            }
      if (cr == t->elements()->back()) {
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
//   chordAttributes
//---------------------------------------------------------

static void chordAttributes(Chord* chord, Notations& notations, Technical& technical, Xml& xml)
      {
      QList<NoteAttribute*>* na = chord->getAttributes();
      // first output the fermatas
      for (ciAttribute ia = chord->getAttributes()->begin(); ia != chord->getAttributes()->end(); ++ia) {
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
      for (ciAttribute ia = na->begin(); ia != na->end(); ++ia) {
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
            articulations.etag(xml);
      // then the attributes whose elements are children of <ornaments>
      Ornaments ornaments;
      for (ciAttribute ia = na->begin(); ia != na->end(); ++ia) {
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
            if (chord->tremolo()) {
                  Tremolo * tr = chord->tremolo();
                  int st = tr->subtype();
                  switch (st) {
                        case TREMOLO_1:
                              notations.tag(xml);
                              ornaments.tag(xml);
                              xml.tag("tremolo", "1");
                              break;
                        case TREMOLO_2:
                              notations.tag(xml);
                              ornaments.tag(xml);
                              xml.tag("tremolo", "2");
                              break;
                        case TREMOLO_3:
                              notations.tag(xml);
                              ornaments.tag(xml);
                              xml.tag("tremolo", "3");
                              break;
                        default:
                              printf("unknown tremolo %d\n", st);
                              break;
                        }
                  }
            wavyLineStartStop(chord, notations, ornaments, xml);
            ornaments.etag(xml);
      // and finally the attributes whose elements are children of <technical>
      for (ciAttribute ia = na->begin(); ia != na->end(); ++ia) {
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
// </notations>

static void arpeggiate(Arpeggio * arp, Xml& xml)
      {
      int st = arp->subtype();
      switch (st) {
            case 0:
                  xml.tagE("arpeggiate");
                  break;
            case 1:
                  xml.tagE("arpeggiate direction=\"up\"");
                  break;
            case 2:
                  xml.tagE("arpeggiate direction=\"down\"");
                  break;
            default:
                  printf("unknown arpeggio subtype %d\n", st);
                  break;
            }
      }

//---------------------------------------------------------
//   chord
//---------------------------------------------------------

/**
 Write \a chord on \a staff with lyriclist \a ll.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::chord(Chord* chord, int staff, const LyricsList* ll)
      {
      NoteList* nl = chord->noteList();

      for (iNote i = nl->begin(); i != nl->end(); ++i) {
            QString val;
            Note* note = i->second;

            attr.doAttr(xml, false);
            xml.stag("note");

            if (i != nl->begin())
                  xml.tagE("chord");
            char c;
            int alter;
            int octave;
            pitch2xml(note, c, alter, octave);

          // pitch
            xml.stag("pitch");
            char buffer[2];
            buffer[0] = c;
            buffer[1] = 0;
            xml.tag("step", QString(buffer));
            if (alter)
                  xml.tag("alter", alter);
            xml.tag("octave", octave);
            xml.etag();

          // duration
            xml.tag("duration", note->chord()->tickLen());

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
                  actNotes = t->actualNotes();
                  nrmNotes = t->normalNotes();
                  }

            if (t) {
                  xml.stag("time-modification");
                  xml.tag("actual-notes", actNotes);
                  xml.tag("normal-notes", nrmNotes);
                  xml.etag();
                  }

            QString s = tick2xml(note->chord()->tickLen() * actNotes / nrmNotes, dots);
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
            int acc        = note->accidentalSubtype();
            if (acc != ACC_NONE) {
                  if (6 <= acc && acc <= 10) {
                        acc -= 5;
                        editorial = true;
                        }
                  else if (11 <= acc && acc <= 15) {
                        acc -= 10;
                        editorial = true;
                        }
                  QString s;
                  switch(acc) {
                        case ACC_SHARP:   s = "sharp";        break;
                        case ACC_FLAT:    s = "flat";         break;
                        case ACC_SHARP2:  s = "double-sharp"; break;
                        case ACC_FLAT2:   s = "flat-flat";    break;
                        case ACC_NATURAL: s = "natural";      break;
                        default:
                              printf("unknown accidental %d\n", acc);
                        }
                  if (editorial)
                        xml.tag("accidental editorial=\"yes\"", s);
                  else
                        xml.tag("accidental", s);
                  }

            if (note->tieBack())
                  xml.tagE("tie type=\"stop\"");
            if (note->tieFor())
                  xml.tagE("tie type=\"start\"");

            // no stem for whole notes and beyond
            if (note->chord()->tickLen() < 4*division)
                  xml.tag("stem", QString(note->chord()->isUp() ? "up" : "down"));
            // LVIFIX: check move() handling
            if (staff)
                  xml.tag("staff", staff + note->staffMove());

            //  beaming
            //    <beam number="1">start</beam>
            //    <beam number="1">end</beam>
            //    <beam number="1">continue</beam>
            //    <beam number="1">backward hook</beam>
            //    <beam number="1">forward hook</beam>

            if (i == nl->begin() && chord->beam()) {
                  QString s = chord->beam()->xmlType(chord);
                  xml.tag("beam number=\"1\"", s);
                  }

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

            if (i == nl->begin()) {
                  tupletStartStop(chord, notations, xml);
                  sh.doSlurStop(chord, notations, xml);
                  sh.doSlurStart(chord, notations, xml);
                  chordAttributes(chord, notations, technical, xml);
                  }
            foreach (const Element* e, *note->el()) {
                  if (e->type() == TEXT && e->subtype() == TEXT_FINGERING) {
                        Text* f = (Text*)e;
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tag("fingering", f->getText());
                        }
                  else {
                        // TODO
                        }
                  }
            technical.etag(xml);
            if (chord->arpeggio()) {
                  notations.tag(xml);
                  arpeggiate(chord->arpeggio(), xml);
                  }
            notations.etag(xml);
            // write lyrics (only for first note)
            if ((i == nl->begin()) && ll)
                  lyrics(ll);
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
      attr.doAttr(xml, false);
      xml.stag("note");
      xml.tagE("rest");

      xml.tag("duration", rest->tickLen());
      // for a single-staff part, staff is 0, which needs to be corrected
      // to calculate the correct voice number
      int voice = (staff-1) * VOICES + rest->voice() + 1;
      if (staff == 0)
            voice += VOICES;
      xml.tag("voice", voice);
      int dots = 0;
      Tuplet* t = rest->tuplet();
      int actNotes = 1;
      int nrmNotes = 1;
      if (t) {
            actNotes = t->actualNotes();
            nrmNotes = t->normalNotes();
            }
      QString s = tick2xml(rest->tickLen() * actNotes / nrmNotes, dots);
      if (s.isEmpty()) {
            printf("no rest type found for ticks %d at %d in measure %d\n",
               rest->tickLen(), rest->tick(), rest->measure()->no()+1);
            }
      xml.tag("type", s);

      for (int i = dots; i > 0; i--)
            xml.tagE("dot");

      if (t) {
            xml.stag("time-modification");
            xml.tag("actual-notes", actNotes);
            xml.tag("normal-notes", nrmNotes);
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
      if (el)
            xml.stag(QString("direction placement=\"%1\"").arg((el->userOff().y() > 0.0) ? "below" : "above"));
      else
            xml.stag("direction");
      xml.stag("direction-type");
      }

//---------------------------------------------------------
//   directionETag
//---------------------------------------------------------

static void directionETag(Xml& xml, int staff, int offs = 0)
      {
      xml.etag();
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

void ExportMusicXml::tempoText(TempoText* text, int staff)
      {
      attr.doAttr(xml, false);
      xml.stag(QString("direction placement=\"%1\"").arg((text->userOff().y() > 0.0) ? "below" : "above"));
      xml.stag("direction-type");
      xml.tag("words", text->getText());
      xml.etag();
      int offs = text->mxmlOff();
      if (offs)
            xml.tag("offset", offs);
      if (staff)
            xml.tag("staff", staff);
      xml.tagE("sound tempo=\"%d\"", (int) text->tempo());
      xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportMusicXml::words(Text* text, int staff)
      {
      directionTag(xml, attr, text);
      if (text->subtypeName() == "RehearsalMark")
            xml.tag("rehearsal", text->getText());
      else
            xml.tag("words", text->getText());
      directionETag(xml, staff, text->mxmlOff());
      }

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportMusicXml::hairpin(Hairpin* hp, int staff, int tick)
      {
      directionTag(xml, attr, hp);
      if (hp->tick() == tick)
            xml.tagE("wedge type=\"%s\"", hp->subtype() ? "diminuendo" : "crescendo");
      else
            xml.tagE("wedge type=\"stop\"");
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
      if (ot->tick() == tick) {
            char* sz = 0;
            char* tp = 0;
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
      directionETag(xml, staff);
      }

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

void ExportMusicXml::pedal(Pedal* pd, int staff, int tick)
      {
      directionTag(xml, attr, pd);
      if (pd->tick() == tick)
            xml.tagE("pedal type=\"start\" line=\"yes\"");
      else
            xml.tagE("pedal type=\"stop\" line=\"yes\"");
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
      directionETag(xml, staff, dyn->mxmlOff());
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

void ExportMusicXml::symbol(Symbol* sym, int staff)
      {
      QString name = symbols[sym->sym()].name();
      char* mxmlName = "";
      if (name == "pedal ped") mxmlName = "pedal type=\"start\"";
      else if (name == "pedalasterisk") mxmlName = "pedal type=\"stop\"";
      if (mxmlName == "") {
            printf("ExportMusicXml::symbol(): %s not supported", name.toLatin1().data());
            return;
      }
      directionTag(xml, attr, sym);
      xml.tagE(mxmlName);
      directionETag(xml, staff, sym->mxmlOff());
      }

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

void ExportMusicXml::lyrics(const LyricsList* ll)
      {
      for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
            if (*i) {
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
                  xml.etag();
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
//---------------------------------------------------------

static void work(Xml& xml, const MeasureBase* measure)
      {
      xml.stag("work");
      foreach(const Element* element, *measure->el()) {
            if (element->type() == TEXT) {
                  const Text* text = (const Text*)element;
                  if (text->subtype() == TEXT_SUBTITLE)
                        xml.tag("work-number", text->getText());
                  }
            }
      foreach(const Element* element, *measure->el()) {
            if (element->type() == TEXT) {
                  const Text* text = (const Text*)element;
                  if (text->subtype() == TEXT_TITLE)
                        xml.tag("work-title", text->getText());
                  }
            }
      xml.etag();
      }

//---------------------------------------------------------
//  write
//---------------------------------------------------------

/**
 Write the score to \a dev in MusicXML format.
 */

void ExportMusicXml::write(QIODevice* dev)
      {

/*
printf("gel contains:\n");
foreach(Element* el, *(score->gel())) {
      printf("%p type=%d(%s) tick=%d track=%d",
             el, el->type(), elementNames[el->type()], el->tick(), el->track());
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

      xml.setDevice(dev);
      xml << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
      xml << "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 2.0 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
      xml.stag("score-partwise");

      const MeasureBase* measure = score->mainLayout()->first();
      work(xml, measure);

      xml.stag("identification");
      foreach(const Element* element, *measure->el()) {
            if (element->type() == TEXT) {
                  const Text* text = (const Text*)element;
                  switch (text->subtype()) {
                        case TEXT_COMPOSER:
                              xml.tag("creator type=\"composer\"", text->getText());
                              break;
                        case TEXT_POET:
                              xml.tag("creator type=\"poet\"", text->getText());
                              break;
                        case TEXT_TRANSLATOR:
                              xml.tag("creator type=\"creator\"", text->getText());
                              break;
                        }
                  }
            }

      if (score->rights)
            xml.tag("rights", score->rights->toPlainText());
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
      xml.etag();

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
            xml.tag("part-name", part->longName().toPlainText());

            xml.stag(QString("score-instrument id=\"P%1-I%2\"").arg(idx+1).arg(3));
            xml.tag("instrument-name", part->longName().toPlainText());
            xml.etag();

            xml.stag(QString("midi-instrument id=\"P%1-I%2\"").arg(idx+1).arg(3));
            xml.tag("midi-channel", part->midiChannel() + 1);
            xml.tag("midi-program", part->midiProgram() + 1);
            xml.etag();

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
            for (MeasureBase* mb = score->mainLayout()->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;
                  // printf("measureNo=%d\n", measureNo);
                  // pickup and other irregular measures need special care
                  if ((irregularMeasureNo + measureNo) == 2 && m->irregular()) {
                        xml.stag("measure number=\"0\" implicit=\"yes\"");
                        pickupMeasureNo++;
                        }
                  else if (m->irregular()) {
                        xml.stag(QString("measure number=\"X%1\" implicit=\"yes\"").arg(irregularMeasureNo++));
                        }
                  else {
                        xml.stag(QString("measure number=\"%1\"").arg(measureNo++));
                        }

                  if (m->prev() && ((Measure*)m->prev())->lineBreak())  // TODO: MeasureBase
                        xml.tagE("print new-system=\"yes\"");
                  if (measureNo > 2 && ((Measure*)m->prev())->pageBreak())    // TODO: MeasureBase
                        xml.tagE("print new-page=\"yes\"");

                  attr.start();
                  dh.buildDirectionsList(m, false, part, strack, etrack);

                  // barline left must be the first element in a measure
                  barlineLeft(m);

                  // output attributes with the first actual measure (pickup or regular)
                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                        attr.doAttr(xml, true);
                        xml.tag("divisions", division);
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
                        KeyList* kl = score->staff(0)->keymap();
                        int key = kl->key(ti);
                        ciKeyEvent ci = kl->find(ti);
                        if (ci != kl->end()) {
                              keysig(key);
                              }
                        }
                  else if (tick == 0)
                        // always write a keysig at tick = 0
                        keysig(0);
                  if (tsig) {
                        int z, n;
                        score->sigmap->timesig(tsig->tick(), z, n);
                        timesig(tsig);
                        }
                  // output attributes with the first actual measure (pickup or regular) only
                  if ((irregularMeasureNo + measureNo + pickupMeasureNo) == 4) {
                        if (staves > 1)
                              xml.tag("staves", staves);
                        }
                  // output attribute at start of measure: clef
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
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
                                          ClefList* cl = score->staff(st/VOICES)->clef();
                                          ciClefEvent ci = cl->find(ti);
                                          if (ci != cl->end()) {
                                                clef(sstaff, ct);
                                                }
                                          }
                                    }
                        }

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
                                    foreach(Element* he, *m->el()) {
                                          if ((he->type() == HARMONY) && (he->staffIdx() == sstaff)
                                             && (he->tick() == el->tick())) {
                                                attr.doAttr(xml, false);
                                                harmony((Harmony*)he);
                                                }
                                          }
                                    }

                              if (tick != el->tick()) {
                                    attr.doAttr(xml, false);
                                    moveToTick(el->tick());
                                    }
                              if (el->isChordRest())
                                    tick += el->tickLen();

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
                                          ClefList* cl = score->staff(st/VOICES)->clef();
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
                                          ciKeyEvent ci = kl->find(ti);
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
                                                score->sigmap->timesig(el->tick(), z, n);
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
                                          if ((st % VOICES) == 0) ll = seg->lyricsList(st / VOICES);
                                          chord((Chord*)el, sstaff, ll);
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

                                    // LVIFIX TODO: support breath mark, which is stored in a separate segment by mscore,
                                    // and is note/notations/articulations/breathmark in MusicXML. It is not clear if
                                    // the breath mark applies to the previous or next note. Furthermore, MusicXML supports
                                    // only one type of breath mark while mscore has two different types.
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
                  moveToTick(m->tick() + m->tickLen());
                  if (idx == 0)
                        repeatAtMeasureStop(xml, m);
                  // note: don't use "m->repeatFlags() & RepeatEnd" here, because more
                  // barline types need to be handled besides repeat end ("light-heavy")
                  barlineRight(m);
                  xml.etag();
                  }
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
//      printf("Score::saveMxl(%s)\n", name.toLatin1().data());

      Zip::ErrorCode ec;
      Zip uz;

      ec = uz.createArchive(name);
      if (ec != Zip::Ok) {
            printf("Cannot create zipfile '%s'\n", name.toLatin1().data());
            return false;
            }

      QFileInfo fi(name);
      QDateTime dt;
      if (debugMode)
            dt = QDateTime(QDate(2007, 9, 10), QTime(12, 0));
      else
            dt = QDateTime::currentDateTime();
      QString fn = fi.baseName() + ".xml";
//      printf("fn=%s\n", fn.toLatin1().data());

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
            printf("Cannot add container.xml to zipfile '%s'\n", name.toLatin1().data());
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
            printf("Cannot add %s to zipfile '%s'\n", fn.toLatin1().data(), name.toLatin1().data());
            return false;
            }

      ec = uz.closeArchive();
      if (ec != Zip::Ok) {
            printf("Cannot close zipfile '%s'\n", name.toLatin1().data());
            return false;
            }

      return true;
      }

//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

void ExportMusicXml::harmony(Harmony* h)
      {
      static char* stepTable[] = {
//          "C",  "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
            "C",  "D",  "D", "E",  "E", "F", "G",  "G", "A",  "A", "B",  "B"
            };
      static int alterTable[] = {
            0,     -1,   0,  -1,    0,   0,   -1,   0,  -1,    0,   -1,   0
            };

      xml.stag("harmony print-frame=\"no\"");
      int root = h->root();
      const char* extension = "";
      if (root > 0) {
            xml.stag("root");
            xml.tag("root-step", stepTable[root - 1]);
            int alter = alterTable[root - 1];
            if (alter)
                  xml.tag("root-alter", alter);
            xml.etag();
            for (unsigned int i = 0; ; ++i) {
                  if (chordExtensions[i].idx == -1)
                        break;
                  if (chordExtensions[i].idx == h->extension()) {
                        extension = chordExtensions[i].xmlName;
                        break;
                        }
                  }
            }
      xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), extension);
      xml.etag();
      }


