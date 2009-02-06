//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.cpp,v 1.14 2006/03/28 14:58:58 wschweer Exp $
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

#include "part.h"
#include "staff.h"
#include "xml.h"
#include "score.h"
#include "layout.h"
#include "style.h"
#include "note.h"
#include "drumset.h"
#include "instrtemplate.h"
#include "text.h"

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
      {
      _longName  = new TextC(s);
      _shortName = new TextC(s);
      _longName->setSubtype(TEXT_INSTRUMENT_LONG);
      _shortName->setSubtype(TEXT_INSTRUMENT_SHORT);
      _score = s;
      _show  = true;
      }

Part::~Part()
      {
      delete _longName;
      delete _shortName;
      }

//---------------------------------------------------------
//   initFromInstrTemplate
//---------------------------------------------------------

void Part::initFromInstrTemplate(const InstrumentTemplate* t)
      {
      // setMidiProgram(t->midiProgram);
      setMinPitch(t->minPitch);
      setMaxPitch(t->maxPitch);
      setShortName(t->shortName);
      setTrackName(t->trackName);
      setLongName(t->name);

      setPitchOffset(t->transpose);
      if (t->useDrumset) {
            setUseDrumset(true);
            setDrumset(new Drumset(*((t->drumset) ? t->drumset : smDrumset)));
            }
      _instrument.midiActions   = t->midiActions;
      foreach(Channel* a, _instrument.channel)
            delete a;
      _instrument.channel.clear();
      _instrument.channel = t->channel;
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Part::staff(int idx) const
      {
      return _staves[idx];
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Part::read(QDomElement e)
      {
      int rstaff = 0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score, this, rstaff);
                  staff->read(e);
                  _score->staves().push_back(staff);
                  _staves.push_back(staff);
                  ++rstaff;
                  }
            else if (tag == "Instrument")
                  _instrument.read(e);
            else if (tag == "name") {
                  if (_score->mscVersion() <= 101)
                        _longName->setHtml(val);
                  else
                        _longName->setHtml(Xml::htmlToString(e.firstChildElement()));
                  }
            else if (tag == "shortName") {
                  if (_score->mscVersion() <= 101)
                        _shortName->setHtml(val);
                  else
                        _shortName->setHtml(Xml::htmlToString(e.firstChildElement()));
                  }
            else if (tag == "trackName") {
                  _trackName = val;
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
      _longName->setText(s);
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortName(const QString& s)
      {
      _shortName->setText(s);
      }

void Part::setLongNameHtml(const QString& s)
      {
      _longName->setHtml(s);
      }

void Part::setShortNameHtml(const QString& s)
      {
      _shortName->setHtml(s);
      }

QString Part::shortNameHtml() const
      {
      return _shortName->getHtml();
      }

QString Part::longNameHtml() const
      {
      return _longName->getHtml();
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongName(const QTextDocument& s)
      {
      _longName->setDoc(s);
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortName(const QTextDocument& s)
      {
      _shortName->setDoc(s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Part::write(Xml& xml) const
      {
      xml.stag("Part");
      foreach(const Staff* staff, _staves)
            staff->write(xml);
      if (!_trackName.isEmpty())
            xml.tag("trackName", _trackName);
      if (!_longName->isEmpty()) {
            xml.stag("name");
            xml.writeHtml(_longName->getHtml());
            xml.etag();
            }
      if (!_shortName->isEmpty()) {
            xml.stag("shortName");
            xml.writeHtml(_shortName->getHtml());
            xml.etag();
            }
      if (!_show)
            xml.tag("show", _show);
      _instrument.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   setStaves
//---------------------------------------------------------

void Part::setStaves(int n)
      {
      int ns = _staves.size();
      if (n < ns) {
            printf("Part::setStaves(): remove staves not implemented!\n");
            return;
            }
      int staffIdx = _score->staffIdx(this) + ns;
      for (int i = ns; i < n; ++i) {
            Staff* staff = new Staff(_score, this, i);
            _staves.push_back(staff);
            _score->staves().insert(staffIdx, staff);
            for (MeasureBase* mb = _score->layout()->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  m->insertStaff(staff, staffIdx);
                  }
            ++staffIdx;
            }
      }

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void Part::setUseDrumset(bool val)
      {
      _instrument.useDrumset = val;
      if (val && _instrument.drumset == 0)
            _instrument.drumset = new Drumset(*smDrumset);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Part::insertStaff(Staff* staff)
      {
      int idx = staff->rstaff();
      if (idx > _staves.size())
            idx = _staves.size();
      _staves.insert(idx, staff);
      staff->setShow(_show);
      idx = 0;
      foreach(Staff* staff, _staves)
            staff->setRstaff(idx++);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Part::removeStaff(Staff* staff)
      {
      _staves.removeAll(staff);
      int idx = 0;
      foreach(Staff* staff, _staves)
            staff->setRstaff(idx++);
      }

//---------------------------------------------------------
//   setShow
//---------------------------------------------------------

void Part::setShow(bool val)
      {
      _show = val;
      foreach(Staff* staff, _staves)
            staff->setShow(_show);
      }

//---------------------------------------------------------
//   setMidiProgram
//    TODO
//---------------------------------------------------------

void Part::setMidiProgram(int)
      {
      }

int Part::volume() const
      {
      return _instrument.channel[0]->volume;
      }

int Part::reverb() const
      {
      return _instrument.channel[0]->reverb;
      }

int Part::chorus() const
      {
      return _instrument.channel[0]->chorus;
      }

int Part::pan() const
      {
      return _instrument.channel[0]->pan;
      }

int Part::midiProgram() const
      {
      return _instrument.channel[0]->program;
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Part::midiChannel() const
      {
      return score()->midiChannel(_instrument.channel[0]->channel);
      }

//---------------------------------------------------------
//   setMidiChannel
//---------------------------------------------------------

void Part::setMidiChannel(int)
      {

      }

