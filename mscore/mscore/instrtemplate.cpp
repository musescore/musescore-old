//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: instrdialog.cpp,v 1.32 2006/03/13 21:35:59 wschweer Exp $
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

QList<InstrumentTemplate*> instrumentTemplates;

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentTemplate::write(Xml& xml) const
      {
      xml.stag("instrument");
      xml.tag("name", name);
      xml.tag("short-name", shortName);
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
      if (minPitch != 0)
            xml.tag("minPitch", minPitch);
      if (maxPitch != 127)
            xml.tag("maxPitch", maxPitch);
      if (transpose)
            xml.tag("transpose", transpose);
      if (useDrumset)
            xml.tag("drumset", useDrumset);
      if (midiProgram)
            xml.tag("midiprogram", midiProgram);
      foreach(const MidiAction& a, midiActions)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentTemplate::read(const QString& g, QDomElement e)
      {
      group  = g;
      staves = 1;
      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]    = 0;
            staffLines[i] = 5;
            smallStaff[i] = false;
            }
      bracket     = -1;
      midiProgram = 0;
      minPitch    = 0;
      maxPitch    = 127;
      transpose   = 0;
      useDrumset  = false;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "name")
                  name = val;
            else if (tag == "short-name")
                  shortName = val;
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
            else if (tag == "minPitch")
                  minPitch = i;
            else if (tag == "maxPitch")
                  maxPitch = i;
            else if (tag == "transpose")
                  transpose = i;
            else if (tag == "drumset")
                  useDrumset = i;
            else if (tag == "midiprogram")
                  midiProgram = i;
            else if (tag == "MidiAction") {
                  MidiAction a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else
                  domError(e);
            }
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

      instrumentTemplates.clear();
      if (!rv) {
            QString s;
            s.sprintf("error reading file %s at line %d column %d: %s\n",
               instrTemplates.toLatin1().data(), line, column, err.toLatin1().data());

            QMessageBox::critical(0, "MuseScore: Read File", s);
            return true;
            }

      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "instrument-group")
                              readInstrumentGroup(ee);
                        else
                              domError(ee);
                        }
                  }
            }
      return true;
      }

