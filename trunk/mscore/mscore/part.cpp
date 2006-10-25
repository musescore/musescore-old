//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.cpp,v 1.14 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "part.h"
#include "staff.h"
#include "xml.h"
#include "score.h"
#include "layout.h"
#include "style.h"

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
      {
      cs = s;
//      _longName.setStyle(TEXT_STYLE_INSTRUMENT_LONG);
//      _shortName.setStyle(TEXT_STYLE_INSTRUMENT_SHORT);
      _staves = new StaffList;
      }

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::~Part()
      {
      delete _staves;
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Part::staff(int idx) const
      {
      return (*_staves)[idx];
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Part::read(Score* score, QDomNode node)
      {
      int rstaff = 0;
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "Staff") {
                  Staff* staff = new Staff(score, this, rstaff);
                  staff->read(node);
                  score->staves()->push_back(staff);
                  _staves->push_back(staff);
                  ++rstaff;
                  }
            else if (tag == "Instrument")
                  _instrument.read(node);
            else if (tag == "name")
                  _longName = val; // .read(node);
            else if (tag == "shortName")
                  _shortName = val; // .read(node);
            else if (tag == "trackName") {
                  _trackName = val;
printf("trackName <%s>\n", val.toLocal8Bit().data());
                  }
            else
                  printf("Mscore:Part: unknown tag %s\n", tag.toLocal8Bit().data());
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Part::write(Xml& xml) const
      {
      xml.stag("Part");
      for (iStaff i = _staves->begin(); i != _staves->end(); ++i)
            (*i)->write(xml);
      if (!_trackName.isEmpty())
            xml.tag("trackName", _trackName);
      if (!_longName.isEmpty())
            xml.tag("name", _longName); // .write(xml, "name");
      if (!_shortName.isEmpty())
            xml.tag("shortName", _shortName); // .write(xml, "shortName");
      _instrument.write(xml);
      xml.etag("Part");
      }

//---------------------------------------------------------
//   nstaves
//---------------------------------------------------------

int Part::nstaves() const
      {
      return _staves->size();
      }

//---------------------------------------------------------
//   setStaves
//---------------------------------------------------------

void Part::setStaves(int n)
      {
      int ns = _staves->size();
      if (n < ns) {
            printf("Part::setStaves(): remove staves not implemented!\n");
            return;
            }
      int staffIdx = cs->staff(this) + ns;
      for (int i = ns; i < n; ++i) {
            Staff* staff = new Staff(cs, this, i);
            _staves->push_back(staff);
            cs->staves()->insert(cs->staves()->begin() + staffIdx, staff);
            for (Measure* im = cs->scoreLayout()->first(); im; im = im->next()) {
                  im->insertStaff1(staff, staffIdx);
                  }
            ++staffIdx;
            }
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument()
      {
      midiChannel = 0;
      midiProgram = 0;
      volume  = 100;
      pan     = 64;
      chorus  = 30;
      reverb  = 30;
      mute    = false;
      solo    = false;
      show    = true;
      minPitch = 0;
      maxPitch = 127;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Instrument::write(Xml& xml) const
      {
      xml.stag("Instrument");
      xml.tag("midiChannel", midiChannel);
      xml.tag("midiProgram", midiProgram);
      xml.tag("volume", volume);
      if (pan != 64)
            xml.tag("pan", pan);
      if (chorus)
            xml.tag("chorus", chorus);
      if (reverb)
            xml.tag("reverb", reverb);
      if (mute)
            xml.tag("mute", mute);
      if (solo)
            xml.tag("solo", solo);
      if (!show)
            xml.tag("show", show);
      if (minPitch > 0)
            xml.tag("minPitch", minPitch);
      if (maxPitch < 127)
            xml.tag("maxPitch", maxPitch);
      xml.etag("Instrument");
      }

//---------------------------------------------------------
//   Instrument::read
//---------------------------------------------------------

void Instrument::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "midiChannel")
                  midiChannel = i;
            else if (tag == "midiProgram")
                  midiProgram = i;
            else if (tag == "volume")
                  volume = i;
            else if (tag == "pan")
                  pan = i;
            else if (tag == "chorus")
                  chorus = i;
            else if (tag == "reverb")
                  reverb = i;
            else if (tag == "mute")
                  mute = i;
            else if (tag == "solo")
                  solo = i;
            else if (tag == "show")
                  show = i;
            else if (tag == "minPitch")
                  minPitch = i;
            else if (tag == "maxPitch")
                  maxPitch = i;
            else
                  printf("Mscore:Instrument: unknown tag %s\n",
                     tag.toLatin1().data());
            }
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Part::insertStaff(Staff* staff)
      {
      int idx = staff->rstaff();
      if (idx > _staves->size())
            idx = _staves->size();
      _staves->insert(_staves->begin() + idx, staff);
      idx = 0;
      for (iStaff i = _staves->begin(); i != _staves->end(); ++i, ++idx)
            (*i)->setRstaff(idx);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Part::removeStaff(Staff* staff)
      {
      _staves->remove(staff);
      int idx = 0;
      for (iStaff i = _staves->begin(); i != _staves->end(); ++i, ++idx)
            (*i)->setRstaff(idx);
      }

