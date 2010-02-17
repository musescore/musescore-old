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
      bracket            = NO_BRACKET;
      minPitchA          = 0;
      maxPitchA          = 127;
      minPitchP          = 0;
      maxPitchP          = 127;
      transposeChromatic = 0;
      transposeDiatonic  = 0;
      useDrumset         = false;
      drumset            = 0;
      extended           = false;
      }

InstrumentTemplate::InstrumentTemplate(const InstrumentTemplate& t)
      {
      trackName  = t.trackName;
      name       = t.name;
      shortName  = t.shortName;
      staves     = t.staves;
      extended   = t.extended;

      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]    = t.clefIdx[i];
            staffLines[i] = t.staffLines[i];
            smallStaff[i] = t.smallStaff[i];
            }
      bracket    = t.bracket;
      minPitchA  = t.minPitchA;
      maxPitchA  = t.maxPitchA;
      minPitchP  = t.minPitchP;
      maxPitchP  = t.maxPitchP;
      transposeChromatic = t.transposeChromatic;
      transposeDiatonic  = t.transposeDiatonic;
      useDrumset = t.useDrumset;
      if (t.drumset)
            drumset = new Drumset(*t.drumset);
      else
            drumset = 0;
      midiActions = t.midiActions;

      foreach(Channel* c, t.channel) {
            Channel* ch = new Channel(*c);
            channel.append(ch);
            }
      }

InstrumentTemplate::~InstrumentTemplate()
      {
      delete drumset;
      foreach(Channel* c, channel)
            delete c;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentTemplate::write(Xml& xml) const
      {
      xml.stag("Instrument");
      xml.tag("name", name);
      xml.tag("short-name",  shortName);
      xml.tag("description", trackName);
      xml.tag("extended", extended);
      if (staves == 1) {
            xml.tag("clef", clefIdx[0]);
            if (staffLines[0] != 5)
                  xml.tag("stafflines", staffLines[0]);
            if (smallStaff[0])
                  xml.tag("smallStaff", smallStaff[0]);
            }
      else {
            xml.tag("staves", staves);
            for (int i = 0; i < staves; ++i) {
                  xml.tag(QString("clef staff=\"%1\"").arg(i), clefIdx[i]);
                  if (staffLines[0] != 5)
                        xml.tag(QString("stafflines staff=\"%1\"").arg(i), staffLines[i]);
                  if (smallStaff[0])
                        xml.tag(QString("smallStaff staff=\"%1\"").arg(i), smallStaff[i]);
                  }
            }
      xml.tag("bracket", bracket);
      if (minPitchA != 0 || maxPitchA != 127)
            xml.tag("aPitchRange", QString("%1-%2").arg(minPitchA).arg(maxPitchA));
      if (minPitchP != 0 || maxPitchP != 127)
            xml.tag("pPitchRange", QString("%1-%2").arg(minPitchP).arg(maxPitchP));
      if (transposeDiatonic)
            xml.tag("transposeDiatonic", transposeDiatonic);
      if (transposeChromatic)
            xml.tag("transposeChromatic", transposeChromatic);
      if (useDrumset) {
            xml.tag("drumset", useDrumset);
            drumset->save(xml);
            }
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const Channel* a, channel)
            a->write(xml);
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
//   read
//---------------------------------------------------------

void InstrumentTemplate::read(QDomElement e)
      {
      bool customDrumset = false;
      staves = 1;
      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]    = 0;
            staffLines[i] = 5;
            smallStaff[i] = false;
            }
      bracket    = -1;
      minPitchA  = 0;
      maxPitchA  = 127;
      minPitchP  = 0;
      maxPitchP  = 127;
      transposeDiatonic  = 0;
      transposeChromatic  = 0;
      useDrumset = false;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "name")
                  name = Xml::htmlToString(e);
            else if (tag == "short-name")
                  shortName = Xml::htmlToString(e);
            else if (tag == "description")
                  trackName = val;
            else if (tag == "extended")
                  extended = true;
            else if (tag == "staves")
                  staves = i;
            else if (tag == "clef") {
                  int idx = e.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  clefIdx[idx] = i;
                  }
            else if (tag == "stafflines") {
                  int idx = e.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  staffLines[idx] = i;
                  }
            else if (tag == "smallStaff") {
                  int idx = e.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  smallStaff[idx] = i;
                  }
            else if (tag == "bracket")
                  bracket = i;
            else if (tag == "aPitchRange")
                  setPitchRange(val, &minPitchA, &maxPitchA);
            else if (tag == "pPitchRange")
                  setPitchRange(val, &minPitchP, &maxPitchP);
            else if (tag == "transposition") {    // obsolete
                  transposeChromatic = i;
                  transposeDiatonic = chromatic2diatonic(i);
                  }
            else if (tag == "transposeChromatic")
                  transposeChromatic = i;
            else if (tag == "transposeDiatonic")
                  transposeDiatonic = i;
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
            else if (tag == "midiAction") {
                  NamedEventList a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else if (tag == "channel") {
                  Channel* a = new Channel();
                  a->read(e);
                  channel.append(a);
                  }
            else if (tag == "Articulation") {
                  MidiArticulation* a = new MidiArticulation;
                  a->read(e);
                  articulation.append(a);
                  }
            else
                  domError(e);
            }
      if (channel.isEmpty()) {
            Channel* a      = new Channel();
            a->chorus       = 0;
            a->reverb       = 0;
            a->name         = "normal";
            a->program      = 0;
            a->bank         = 0;
            a->volume       = 100;
            a ->pan         = 60;
            channel.append(a);
            }
      if (useDrumset) {
            if (channel[0]->bank == 0)
                  channel[0]->bank = 128;
            channel[0]->updateInitList();
            }
      if (trackName.isEmpty())
            trackName = parseInstrName(name);
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
                              group->name = ee.attribute("name");
                              group->extended = ee.attribute("extended", "0").toInt();
                              readInstrumentGroup(group, ee);
                              instrumentGroups.append(group);
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


