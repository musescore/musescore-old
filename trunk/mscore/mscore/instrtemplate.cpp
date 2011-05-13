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

#include "instrtemplate.h"
#include "xml.h"
#include "style.h"
#include "sym.h"
#include "drumset.h"
#include "clef.h"
#include "bracket.h"
#include "utils.h"
#include "tablature.h"

QList<InstrumentGroup*> instrumentGroups;
QList<MidiArticulation*> articulation;                // global articulations

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

InstrumentTemplate::InstrumentTemplate()
      {
      staves             = 1;
      clefIdx[0]         = CLEF_G;
      staffLines[0]      = 5;
      smallStaff[0]      = false;
      bracket[0]         = NO_BRACKET;
      bracketSpan[0]     = 1;
      barlineSpan[0]     = 1;
      minPitchA          = 0;
      maxPitchA          = 127;
      minPitchP          = 0;
      maxPitchP          = 127;
      useDrumset         = false;
      drumset            = 0;
      extended           = false;
      tablature          = 0;
      useTablature       = false;
      }

InstrumentTemplate::InstrumentTemplate(const InstrumentTemplate& t)
      {
      init(t);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void InstrumentTemplate::init(const InstrumentTemplate& t)
      {
      trackName  = t.trackName;
      longNames  = t.longNames;
      shortNames = t.shortNames;
      staves     = t.staves;
      extended   = t.extended;

      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]     = t.clefIdx[i];
            staffLines[i]  = t.staffLines[i];
            smallStaff[i]  = t.smallStaff[i];
            bracket[i]     = t.bracket[i];
            bracketSpan[i] = t.bracketSpan[i];
            barlineSpan[i] = t.barlineSpan[i];
            }
      minPitchA  = t.minPitchA;
      maxPitchA  = t.maxPitchA;
      minPitchP  = t.minPitchP;
      maxPitchP  = t.maxPitchP;
      transpose  = t.transpose;
      useDrumset = t.useDrumset;
      if (t.drumset)
            drumset = new Drumset(*t.drumset);
      else
            drumset = 0;
      if (t.tablature)
            tablature = new Tablature(*t.tablature);
      else
            tablature = 0;
      midiActions = t.midiActions;
      channel = t.channel;
      }

InstrumentTemplate::~InstrumentTemplate()
      {
      delete drumset;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentTemplate::write(Xml& xml) const
      {
      xml.stag("Instrument");
      foreach(StaffName sn, longNames)
            xml.tag(QString("longName pos=\"%1\"").arg(sn.pos), sn.name);
      foreach(StaffName sn, shortNames)
            xml.tag(QString("shortName pos=\"%1\"").arg(sn.pos), sn.name);
      xml.tag("description", trackName);
      xml.tag("extended", extended);
      if (tablature)
            tablature->write(xml);
      if (staves == 1) {
            xml.tag("clef", clefTable[clefIdx[0]].tag);
            if (staffLines[0] != 5)
                  xml.tag("stafflines", staffLines[0]);
            if (smallStaff[0])
                  xml.tag("smallStaff", smallStaff[0]);
            }
      else {
            xml.tag("staves", staves);
            for (int i = 0; i < staves; ++i) {
                  xml.tag(QString("clef staff=\"%1\"").arg(i), clefTable[clefIdx[i]].tag);
                  if (staffLines[i] != 5)
                        xml.tag(QString("stafflines staff=\"%1\"").arg(i), staffLines[i]);
                  if (smallStaff[i])
                        xml.tag(QString("smallStaff staff=\"%1\"").arg(i), smallStaff[i]);
                  xml.tag(QString("bracket staff=\"%1\"").arg(i), bracket[i]);
                  xml.tag(QString("bracketSpan staff=\"%1\"").arg(i), bracketSpan[i]);
                  xml.tag(QString("barlineSpan staff=\"%1\"").arg(i), barlineSpan[i]);
                  }
            }
      if (minPitchA != 0 || maxPitchA != 127)
            xml.tag("aPitchRange", QString("%1-%2").arg(minPitchA).arg(maxPitchA));
      if (minPitchP != 0 || maxPitchP != 127)
            xml.tag("pPitchRange", QString("%1-%2").arg(minPitchP).arg(maxPitchP));
      if (transpose.diatonic)
            xml.tag("transposeDiatonic", transpose.diatonic);
      if (transpose.chromatic)
            xml.tag("transposeChromatic", transpose.chromatic);
      if (useDrumset) {
            xml.tag("drumset", useDrumset);
            drumset->save(xml);
            }
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const Channel& a, channel)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   setPitchRange
//---------------------------------------------------------

void InstrumentTemplate::setPitchRange(const QString& s, char* a, char* b) const
      {
      QStringList sl = s.split("-");
      if (sl.size() != 2) {
            *a = 0;
            *b = 127;
            return;
            }
      *a = sl[0].toInt();
      *b = sl[1].toInt();
      }

//---------------------------------------------------------
//   parseInstrName
//---------------------------------------------------------

static QString parseInstrName(const QString& name)
      {
      QString sName;
      QDomDocument dom;
      int line, column;
      QString err;
      if (!dom.setContent(name, false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("error: %s\n", qPrintable(error));
            printf("   data:<%s>\n", qPrintable(name));
            return QString();
            }

      for (QDomNode e = dom.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
                  QDomElement el = ee.toElement();
                  QString tag(el.tagName());
                  if (tag == "symbol") {
                        QString name = el.attribute(QString("name"));
                        if (name == "flat")
                              sName += "b";
                        else if (name == "sharp")
                              sName += "#";
                        }
                  QDomText t = ee.toText();
                  if (!t.isNull())
                        sName += t.data();
                  }
            }
      return sName;
      }

//---------------------------------------------------------
//   readStaffIdx
//---------------------------------------------------------

static int readStaffIdx(QDomElement e)
      {
      int idx = e.attribute("staff", "1").toInt() - 1;
      if (idx >= MAX_STAVES)
            idx = MAX_STAVES-1;
      return idx;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentTemplate::read(QDomElement e)
      {
      bool customDrumset = false;
      staves = 1;
      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]     = CLEF_INVALID;
            staffLines[i]  = -1;
            smallStaff[i]  = false;
            bracket[i]     = NO_BRACKET;
            bracketSpan[i] = 0;
            barlineSpan[i] = 0;
            }
      minPitchA  = 0;
      maxPitchA  = 127;
      minPitchP  = 0;
      maxPitchP  = 127;
      transpose.diatonic  = 0;
      transpose.chromatic  = 0;
      useDrumset = false;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "name" || tag == "longName") {               // "name" is obsolete
                  int pos = e.attribute("pos", "0").toInt();
                  QString longName = Xml::htmlToString(e);
                  longNames.append(StaffName(longName, pos));
                  }
            else if (tag == "short-name" || tag == "shortName") {   // "short-name" is obsolete
                  int pos = e.attribute("pos", "0").toInt();
                  QString shortName = Xml::htmlToString(e);
                  shortNames.append(StaffName(shortName, pos));
                  }
            else if (tag == "description")
                  trackName = val;
            else if (tag == "extended")
                  extended = true;
            else if (tag == "staves") {
                  staves = i;
                  bracketSpan[0] = staves;
                  barlineSpan[0] = staves;
                  }
            else if (tag == "clef") {
                  int idx = readStaffIdx(e);
                  bool ok;
                  int i = val.toInt(&ok);
                  if (!ok) {
                        ClefType ct = Clef::clefType(val);
                        clefIdx[idx] = ct;
                        }
                  else
                        clefIdx[idx] = ClefType(i);
                  }
            else if (tag == "stafflines") {
                  int idx = readStaffIdx(e);
                  staffLines[idx] = i;
                  }
            else if (tag == "smallStaff") {
                  int idx = readStaffIdx(e);
                  smallStaff[idx] = i;
                  }
            else if (tag == "bracket") {
                  int idx = readStaffIdx(e);
                  bracket[idx] = i;
                  }
            else if (tag == "bracketSpan") {
                  int idx = readStaffIdx(e);
                  bracketSpan[idx] = i;
                  }
            else if (tag == "barlineSpan") {
                  int idx = readStaffIdx(e);
                  barlineSpan[idx] = i;
                  }
            else if (tag == "Tablature") {
                  tablature = new Tablature;
                  tablature->read(e);
                  }
            else if (tag == "aPitchRange")
                  setPitchRange(val, &minPitchA, &maxPitchA);
            else if (tag == "pPitchRange")
                  setPitchRange(val, &minPitchP, &maxPitchP);
            else if (tag == "transposition") {    // obsolete
                  transpose.chromatic = i;
                  transpose.diatonic = chromatic2diatonic(i);
                  }
            else if (tag == "transposeChromatic")
                  transpose.chromatic = i;
            else if (tag == "transposeDiatonic")
                  transpose.diatonic = i;
            else if (tag == "drumset")
                  useDrumset = i;
            else if (tag == "Drum") {
                  // if we see on of this tags, a custom drumset will
                  // be created
                  if (drumset == 0)
                        drumset = new Drumset(*smDrumset);
                  if (!customDrumset) {
                        drumset->clear();
                        customDrumset = true;
                        }
                  drumset->load(e);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else if (tag == "channel") {
                  Channel a;
                  a.read(e);
                  channel.append(a);
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  articulation.append(a);
                  }
            else if (tag == "stafftype") {
                  if (val == "tablature")
                        useTablature = true;
                  else {
                        fprintf(stderr, "unknown stafftype <%s>\n", qPrintable(val));
                        domError(e);
                        }
                  }
            else if (tag == "init") {
                  InstrumentTemplate* ttt = searchTemplate(val);
                  if (ttt) {
// printf("Instrument template init <%s> from <%s>\n", qPrintable(trackName), qPrintable(ttt->trackName));
                        init(*ttt);
                        }
                  else
                        printf("Instrument template <%s> not found\n", qPrintable(val));
                  }
            else
                  domError(e);
            }
      //
      // check bar line spans
      //
      int barLine = 0;
      for (int i = 0; i < staves; ++i) {
            int bls = barlineSpan[i];
            if (barLine) {
                  if (bls)
                        barlineSpan[i] = 0;
                  }
            else {
                  if (bls == 0) {
                        bls = 1;
                        barlineSpan[i] = 1;
                        }
                  barLine = bls;
                  }
            --barLine;
            }
      for (int i = 0; i < MAX_STAVES; ++i) {
            if (clefIdx[i] == CLEF_INVALID)
                  clefIdx[i] = CLEF_G;
            if (staffLines[i] == -1)
                  staffLines[i] = 5;
            }
      if (channel.isEmpty()) {
            Channel a;
            a.chorus       = 0;
            a.reverb       = 0;
            a.name         = "normal";
            a.program      = 0;
            a.bank         = 0;
            a.volume       = 100;
            a.pan         = 60;
            channel.append(a);
            }
      if (useDrumset) {
            if (channel[0].bank == 0)
                  channel[0].bank = 128;
            channel[0].updateInitList();
            }
      if (trackName.isEmpty() && !longNames.isEmpty())
            trackName = parseInstrName(longNames[0].name);
      }

//---------------------------------------------------------
//   readInstrumentGroup
//---------------------------------------------------------

static void readInstrumentGroup(InstrumentGroup* group, QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "instrument") {
                  InstrumentTemplate* t = new InstrumentTemplate;
                  group->instrumentTemplates.append(t);
                  t->read(e);
                  }
            else if (tag == "ref") {
                  InstrumentTemplate* ttt = searchTemplate(e.text());
                  if (ttt) {
                        InstrumentTemplate* t = new InstrumentTemplate(*ttt);
                        group->instrumentTemplates.append(t);
                        }
                  else
                        printf("instrument reference not found <%s>\n", qPrintable(e.text()));
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   loadInstrumentTemplates
//---------------------------------------------------------

bool loadInstrumentTemplates(const QString& instrTemplates)
      {
      QFile qf(instrTemplates);
      if (!qf.open(QIODevice::ReadOnly))
            return false;

      QDomDocument doc;
      int line, column;
      QString err;

      bool rv = doc.setContent(&qf, false, &err, &line, &column);

      docName = qf.fileName();
      qf.close();

      foreach(InstrumentGroup* g, instrumentGroups) {
            foreach(InstrumentTemplate* t, g->instrumentTemplates)
                  delete t;
            delete g;
            }
      instrumentGroups.clear();

      if (!rv) {
            QString s;
            s.sprintf("error reading file %s at line %d column %d: %s\n",
               instrTemplates.toLatin1().data(), line, column, err.toLatin1().data());

            QMessageBox::critical(0, "MuseScore: Read Template File", s);
            return true;
            }

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "instrument-group" || tag == "InstrumentGroup") {
                              InstrumentGroup* group = new InstrumentGroup;
                              instrumentGroups.append(group);
                              group->name = ee.attribute("name");
                              group->extended = ee.attribute("extended", "0").toInt();
                              readInstrumentGroup(group, ee);
                              }
                        else if (tag == "Articulation") {
                              MidiArticulation* a = new MidiArticulation;
                              a->read(ee);
                              articulation.append(a);
                              }
                        else
                              domError(ee);
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   searchTemplate
//---------------------------------------------------------

InstrumentTemplate* searchTemplate(const QString& name)
      {
      foreach(InstrumentGroup* g, instrumentGroups) {
            foreach(InstrumentTemplate* it, g->instrumentTemplates) {
// printf("<%s><%s>\n", qPrintable(name), qPrintable(it->trackName));
                  if (it->trackName == name)
                        return it;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   populateInstrumentList
//---------------------------------------------------------

void populateInstrumentList(QTreeWidget* instrumentList, bool extended)
      {
      instrumentList->clear();
      // TODO: memory leak
      foreach(InstrumentGroup* g, instrumentGroups) {
            if (!extended && g->extended)
                  continue;
            InstrumentTemplateListItem* group = new InstrumentTemplateListItem(g->name, instrumentList);
            group->setFlags(Qt::ItemIsEnabled);
            foreach(InstrumentTemplate* t, g->instrumentTemplates) {
                  if (!extended && t->extended)
                        continue;
                  new InstrumentTemplateListItem(t, group);
                  }
            }
      }


