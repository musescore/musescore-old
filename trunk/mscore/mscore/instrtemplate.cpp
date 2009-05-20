//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

QList<InstrumentTemplate*> instrumentTemplates;

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

InstrumentTemplate::InstrumentTemplate()
      {
      drumset = 0;
      }

InstrumentTemplate::InstrumentTemplate(const InstrumentTemplate& t)
      {
      group      = t.group;
      trackName  = t.trackName;
      name       = t.name;
      shortName  = t.shortName;
      staves     = t.staves;

      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]    = t.clefIdx[i];
            staffLines[i] = t.staffLines[i];
            smallStaff[i] = t.smallStaff[i];
            }
      bracket    = t.bracket;
      minPitchA  =  t.minPitchA;
      maxPitchA  =  t.maxPitchA;
      minPitchP  =  t.minPitchP;
      maxPitchP  =  t.maxPitchP;
      transpose  =  t.transpose;
      useDrumset =  t.useDrumset;
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
      if (drumset)
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
      if (transpose)
            xml.tag("transposition", transpose);
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

void InstrumentTemplate::read(const QString& g, QDomElement e)
      {
      bool customDrumset = false;
      group  = g;
      staves = 1;
      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]    = 0;
            staffLines[i] = 5;
            smallStaff[i] = false;
            }
      bracket    = -1;
      minPitchA   = 0;
      maxPitchA   = 127;
      minPitchP   = 0;
      maxPitchP   = 127;
      transpose  = 0;
      useDrumset = false;

#ifdef Q_WS_MAC
      QFont font("MScore1 20");
#else
      QFont font("MScore1");
#endif
#if 0       // TODOX
      double extraMag = 1.0;
      double mag = _spatium * extraMag / (SPATIUM20 * DPI);
      font.setPointSizeF(12.0 * mag);     // TODO: get from style
#endif
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
            else if (tag == "transposition")
                  transpose = i;
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

static void readInstrumentGroup(QDomElement e)
      {
      QString group = e.attribute("name");

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "instrument") {
                  InstrumentTemplate* t = new InstrumentTemplate;
                  t->read(group, e);
                  instrumentTemplates.push_back(t);
                  }
            else if (tag == "ref") {
                  QString name = e.text();
                  InstrumentTemplate* ttt = 0;
                  foreach(InstrumentTemplate* tt, instrumentTemplates) {
                        if (tt->trackName == name) {
                              ttt = tt;
                              break;
                              }
                        }
                  if (ttt) {
                        InstrumentTemplate* t = new InstrumentTemplate(*ttt);
                        t->group = group;
                        instrumentTemplates.push_back(t);
                        }
                  else
                        printf("instrument reference not found <%s>\n", qPrintable(name));
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

      foreach(InstrumentTemplate* t, instrumentTemplates)
            delete t;
      instrumentTemplates.clear();

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
                        if (tag == "instrument-group" || tag == "InstrumentGroup")
                              readInstrumentGroup(ee);
                        else if (tag == "Articulation") {
                              //TODO
                              }
                        else
                              domError(ee);
                        }
                  }
            }
      return true;
      }

