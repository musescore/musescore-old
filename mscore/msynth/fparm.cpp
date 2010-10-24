//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "xml.h"
#include "fparm.h"
#include "synti.h"
#include "mscore/seq.h"       // HACK

extern Seq* seq;

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fparm::write(Xml& xml) const
      {
      xml.tagE(QString("f name=\"%1\" id=\"%2\" val=\"%3\"").arg(_name).arg(_id).arg(_val));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Sparm::write(Xml& xml) const
      {
      xml.tagE(QString("s name=\"%1\" id=\"%2\" val=\"%3\"")
         .arg(_name).arg(_id).arg(Xml::xmlString(_val)));
      }

//---------------------------------------------------------
//   print
//    for debugging
//---------------------------------------------------------

void Fparm::print() const
      {
      printf("<id=%d name=%s val=%f>", _id, qPrintable(_name), _val);
      }
void Sparm::print() const
      {
      printf("<id=%d name=%s val=%s>", _id, qPrintable(_name), qPrintable(_val));
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void Fparm::set(const QString& name, float val, float min, float max)
      {
      _name = name;
      _val  = val;
      _min  = min;
      _max  = max;
      }

bool Fparm::operator==(const Parameter& p) const
      {
      return (p.id() == id()) && (qAbs(((const Fparm&)p)._val - _val) < 0.000001);
      }

//---------------------------------------------------------
//   SynthParams::write
//---------------------------------------------------------

void SynthParams::write(Xml& xml) const
      {
      xml.stag(QString("Synth name=\"%1\"").arg(synth->name()));
      foreach(const Parameter* p, params)
            p->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   SyntiSettings::write
//---------------------------------------------------------

void SyntiSettings::write(Xml& xml) const
      {
      xml.stag("SyntiSettings");
      foreach(const SynthParams& sp, *this)
            sp.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   SyntiSettings::read
//---------------------------------------------------------

void SyntiSettings::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "Synth") {
                  SynthParams sp;
                  QString name = e.attribute("name");
                  MasterSynth* msynth = seq->getSynti();
                  sp.synth = msynth->synth(name);
                  if (sp.synth == 0) {
                        printf("unknown synthesizer <%s>\n", qPrintable(name));
                        return;
                        }
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString pm = ee.attribute("name");
                        int id     = ee.attribute("id").toInt();
                        if (tag == "f") {
                              double d = ee.attribute("val").toDouble();
                              Fparm* p = new Fparm(id, pm, float(d));
                              sp.params.append(p);
                              }
                        else if (tag == "s")
                              sp.params.append(new Sparm(id, pm, ee.attribute("val")));
                        else
                              domError(ee);
                        }
                  append(sp);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SynthParams::operator==(const SynthParams& sp) const
      {
      int n = params.size();
      if (sp.params.size() != n) {
            printf(" != %d %d", sp.params.size(), n);
            return false;
            }
      for (int i = 0; i < n; ++i) {
            if (!(*sp.params[i] == *params[i])) {
/*                  printf(" != ");
                  sp.params[i]->print();
                  params[i]->print();
                  printf("\n");
                  */
                  return false;
                  }
            }
      return true;
      }

bool SynthParams::operator!=(const SynthParams& sp) const
      {
      return !(*this == sp);
      }

