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
#include "style.h"
#include "sym.h"

QList<InstrumentTemplate*> instrumentTemplates;

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

InstrumentTemplate::InstrumentTemplate()
      {
      QTextOption to = name.defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(QTextOption::NoWrap);
      name.setUseDesignMetrics(true);
      name.setDefaultTextOption(to);
      shortName.setUseDesignMetrics(true);
      shortName.setDefaultTextOption(to);
      name.setDefaultFont(defaultTextStyleArray[TEXT_STYLE_INSTRUMENT_LONG].font());
      shortName.setDefaultFont(defaultTextStyleArray[TEXT_STYLE_INSTRUMENT_SHORT].font());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentTemplate::write(Xml& xml) const
      {
      xml.stag("Instrument");
      xml.tag("name", name.toPlainText());            // TODO
      xml.tag("short-name", shortName.toPlainText()); // TODO
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
            xml.tag("transposition", transpose);
      if (useDrumset)
            xml.tag("drumset", useDrumset);
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const Channel* a, channel)
            a->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentTemplate::read(const QString& g, QDomElement de)
      {
      group  = g;
      staves = 1;
      for (int i = 0; i < MAX_STAVES; ++i) {
            clefIdx[i]    = 0;
            staffLines[i] = 5;
            smallStaff[i] = false;
            }
      bracket    = -1;
      minPitch   = 0;
      maxPitch   = 127;
      transpose  = 0;
      useDrumset = false;


      double extraMag = 1.0;
      double mag = _spatium * extraMag / (SPATIUM20 * DPI);
      QFont font("MScore1");
      font.setPointSizeF(12.0 * mag);     // TODO: get from style

      for (QDomNode e = de.firstChildElement(); !e.isNull(); e = e.nextSibling()) {
            QDomElement de = e.toElement();
            QString tag(de.tagName());
            QString val(de.text());
            int i = val.toInt();

            if (tag == "name") {
                  QString n;
                  QTextCursor cursor(&name);
                  QTextCharFormat f = cursor.charFormat();
                  QTextCharFormat sf(f);
                  sf.setFont(font);

                  for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
                        QDomElement de1 = ee.toElement();
                        QString tag(de1.tagName());
                        if (tag == "symbol") {
                              QString name = de1.attribute(QString("name"));
                              if (name == "flat") {
                                    n += "b";
                                    cursor.insertText(QString(0xe10d), sf);
                                    }
                              else if (name == "sharp") {
                                    n += "#";
                                    cursor.insertText(QString(0xe10c), sf);
                                    }
                              }
                        QDomText t = ee.toText();
                        if (!t.isNull()) {
                              n += t.data();
                              cursor.insertText(t.data(), f);
                              }
                        }
                  trackName = n;
                  }
            else if (tag == "short-name") {
                  QTextCursor cursor(&shortName);
                  QTextCharFormat f = cursor.charFormat();
                  QTextCharFormat sf(f);
                  sf.setFont(font);

                  for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
                        QDomElement de1 = ee.toElement();
                        QString tag(de1.tagName());
                        if (tag == "symbol") {
                              QString name = de1.attribute(QString("name"));
                              if (name == "flat")
                                    cursor.insertText(QString(0xe10d), sf);
                              else if (name == "sharp")
                                    cursor.insertText(QString(0xe10c), sf);
                              }
                        QDomText t = ee.toText();
                        if (!t.isNull()) {
                              cursor.insertText(t.data(), f);
                              }
                        }
                  }
            else if (tag == "staves")
                  staves = i;
            else if (tag == "clef") {
                  int idx = de.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  clefIdx[idx] = i;
                  }
            else if (tag == "stafflines") {
                  int idx = de.attribute("staff", "1").toInt() - 1;
                  if (idx >= MAX_STAVES)
                        idx = MAX_STAVES-1;
                  staffLines[idx] = i;
                  }
            else if (tag == "smallStaff") {
                  int idx = de.attribute("staff", "1").toInt() - 1;
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
            else if (tag == "transposition")
                  transpose = i;
            else if (tag == "drumset")
                  useDrumset = i;
            else if (tag == "Drum") {
                  // read instrument of drumset
                  }
            else if (tag == "midiAction") {
                  NamedEventList a;
                  a.read(de);
                  midiActions.append(a);
                  }
            else if (tag == "channel") {
                  Channel* a = new Channel();
                  a->read(de);
                  channel.append(a);
                  }
            else
                  domError(de);
            }
      if (channel.isEmpty()) {
            Channel* a      = new Channel();
            a->chorus       = 0;
            a->reverb       = 0;
            a->name         = "normal";
            a->program      = 0;
            a->volume       = 100;
            a ->pan         = 60;
            channel.append(a);
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

