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
#include "note.h"

Drumset* smDrumset;           // standard midi drumset

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
      {
      _longName.setDefaultFont(textStyles[TEXT_STYLE_INSTRUMENT_LONG].font());
      _shortName.setDefaultFont(textStyles[TEXT_STYLE_INSTRUMENT_SHORT].font());
      cs = s;
      _staves = new StaffList;
      _show = true;
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

void Part::read(Score* score, QDomElement e)
      {
      int rstaff = 0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "Staff") {
                  Staff* staff = new Staff(score, this, rstaff);
                  staff->read(e);
                  score->staves()->push_back(staff);
                  _staves->push_back(staff);
                  ++rstaff;
                  }
            else if (tag == "Instrument")
                  _instrument.read(e);
            else if (tag == "name")
                  _longName.setHtml(val);
            else if (tag == "shortName")
                  _shortName.setHtml(val);
            else if (tag == "trackName") {
                  _trackName = val;
//printf("trackName <%s>\n", val.toLocal8Bit().data());
                  }
            else if (tag == "show")
                  _show = val.toInt();
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongName(const QString& s)
      {
      _longName.setPlainText(s);
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortName(const QString& s)
      {
      _shortName.setPlainText(s);
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongName(const QTextDocument& s)
      {
      _longName.setHtml(s.toHtml("utf8"));
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortName(const QTextDocument& s)
      {
      _shortName.setHtml(s.toHtml("utf8"));
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
            xml.tag("name", _longName.toHtml("utf8"));
      if (!_shortName.isEmpty())
            xml.tag("shortName", _shortName.toHtml("utf8"));
      if (!_show)
            xml.tag("show", _show);
      _instrument.write(xml);
      xml.etag();
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
            for (Measure* im = cs->mainLayout()->first(); im; im = im->next()) {
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
      volume      = 100;
      pan         = 64;
      chorus      = 30;
      reverb      = 30;
      mute        = false;
      solo        = false;
      minPitch    = 0;
      maxPitch    = 127;
      pitchOffset = 0;
      drumset     = 0;
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
      if (minPitch > 0)
            xml.tag("minPitch", minPitch);
      if (maxPitch < 127)
            xml.tag("maxPitch", maxPitch);
      if (pitchOffset)
            xml.tag("transposition", pitchOffset);
      xml.etag();
      }

//---------------------------------------------------------
//   Instrument::read
//---------------------------------------------------------

void Instrument::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
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
            else if (tag == "minPitch")
                  minPitch = i;
            else if (tag == "maxPitch")
                  maxPitch = i;
            else if (tag == "transposition")
                  pitchOffset = i;
            else
                  domError(e);
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
      staff->setShow(_show);
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

//---------------------------------------------------------
//   setShow
//---------------------------------------------------------

void Part::setShow(bool val)
      {
      _show = val;
      foreach(Staff* staff, *_staves)
            staff->setShow(_show);
      }

//---------------------------------------------------------
//   initDrumset
//    initialize standard midi drumset
//---------------------------------------------------------

void initDrumset()
      {
      smDrumset = new Drumset;
      for (int i = 0; i < 128; ++i) {
            smDrumset->drum[i].notehead = -1;   // invalid entry
            smDrumset->drum[i].line     = 0;
            }
      smDrumset->drum[35].notehead = HEAD_NORMAL;   // Acoustic Bass Drum
      smDrumset->drum[35].line     = 7;

      smDrumset->drum[36].notehead = HEAD_NORMAL;   // Bass Drum
      smDrumset->drum[36].line     = 7;

      smDrumset->drum[37].notehead = HEAD_CROSS;   // Side Stick
      smDrumset->drum[37].line     = 3;

      smDrumset->drum[38].notehead = HEAD_NORMAL;   // Snare (Acoustic)
      smDrumset->drum[38].line     = 3;

      smDrumset->drum[40].notehead = HEAD_NORMAL;   // Snare (Electric)
      smDrumset->drum[40].line     = 3;

      smDrumset->drum[41].notehead = HEAD_NORMAL;   // Tom 5
      smDrumset->drum[41].line     = 5;

      smDrumset->drum[42].notehead = HEAD_CROSS;   // Hi-Hat Closed
      smDrumset->drum[42].line     = -1;

      smDrumset->drum[43].notehead = HEAD_NORMAL;   // Tom 4
      smDrumset->drum[43].line     = 5;

      smDrumset->drum[44].notehead = HEAD_CROSS;   // Hi-Hat Pedal
      smDrumset->drum[44].line     = 9;

      smDrumset->drum[45].notehead = HEAD_NORMAL;   // Tom 3
      smDrumset->drum[45].line     = 2;

      smDrumset->drum[46].notehead = HEAD_CROSS;   // Hi-Hat Open
      smDrumset->drum[46].line     = -1;

      smDrumset->drum[47].notehead = HEAD_NORMAL;   // Tom 2
      smDrumset->drum[47].line     = 1;

      smDrumset->drum[48].notehead = HEAD_NORMAL;   // Tom 1
      smDrumset->drum[48].line     = 0;

      smDrumset->drum[49].notehead = HEAD_CROSS;   // Crash 1
      smDrumset->drum[49].line     = -2;

      smDrumset->drum[50].notehead = HEAD_NORMAL;   // Tom
      smDrumset->drum[50].line     = 0;

      smDrumset->drum[51].notehead = HEAD_CROSS;   // Ride
      smDrumset->drum[51].line     = 0;

      smDrumset->drum[52].notehead = HEAD_CROSS;   // China
      smDrumset->drum[52].line     = -3;

      smDrumset->drum[53].notehead = HEAD_DIAMOND;   // Ride (Bell)
      smDrumset->drum[53].line     = 0;

      smDrumset->drum[55].notehead = HEAD_CROSS;   // Ride (Bell)
      smDrumset->drum[55].line     = -3;

      smDrumset->drum[56].notehead = HEAD_TRIANGLE;   // Ride (Bell)
      smDrumset->drum[56].line     = 1;

      smDrumset->drum[57].notehead = HEAD_CROSS;   // Ride (Bell)
      smDrumset->drum[57].line     = -3;

      smDrumset->drum[59].notehead = HEAD_CROSS;   // Ride (Bell)
      smDrumset->drum[59].line     = 2;
      }

