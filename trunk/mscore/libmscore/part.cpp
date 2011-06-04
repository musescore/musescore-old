//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.cpp 3699 2010-11-10 10:42:37Z wschweer $
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
#include "al/xml.h"
#include "score.h"
#include "style.h"
#include "note.h"
#include "drumset.h"
#include "text.h"
#include "measure.h"
#include "tablature.h"
#include "stafftype.h"

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
      {
      _score = s;
      _show  = true;
      setInstrument(Instrument(), 0);     // default instrument
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

void Part::read(XmlReader* r)
      {
      int rstaff = 0;

      QString val;
      while (r->readElement()) {
            MString8 tag(r->tag());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score, this, rstaff);
                  _score->staves().push_back(staff);
                  _staves.push_back(staff);
                  staff->read(r);
                  ++rstaff;
                  }
            else if (tag == "Instrument")
                  instr(0)->read(r);
            else if (r->readString("name", &val)) {
                  ; // instr(0)->longName() = QTextDocumentFragment::fromHtml(val);
                  }
            else if (r->readString("shortName", &val)) {
                  ; // instr(0)->shortName() = QTextDocumentFragment::fromHtml(val);
                  }
            else if (r->readString("trackName", &_trackName))
                  ;
            else if (r->readBool("show", &_show))
                  _show = val.toInt();
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongName(const QString& /*name*/)
      {
      // instr(0)->longName() = QTextDocumentFragment::fromPlainText(name);
      }

void Part::setShortName(const QString& /*s*/)
      {
      // instr(0)->shortName() = QTextDocumentFragment::fromPlainText(s);
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
            for (MeasureBase* mb = _score->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  m->insertStaff(staff, staffIdx);
                  }
            ++staffIdx;
            }
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

void Part::setMidiProgram(int p)
      {
      // LVIFIX: check if this is correct
      // at least it fixes the MIDI program handling in the MusicXML regression test
      Channel c = instr(0)->channel(0);
      c.program = p;
      c.updateInitList();
//TODOxx      instr(0)->setChannel(0, c);
      }

int Part::volume() const
      {
      return instr(0)->channel(0).volume;
      }

int Part::reverb() const
      {
      return instr(0)->channel(0).reverb;
      }

int Part::chorus() const
      {
      return instr(0)->channel(0).chorus;
      }

int Part::pan() const
      {
      return instr(0)->channel(0).pan;
      }

int Part::midiProgram() const
      {
      return instr(0)->channel(0).program;
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Part::midiChannel() const
      {
      return score()->midiChannel(instr(0)->channel(0).channel);
      }

//---------------------------------------------------------
//   setMidiChannel
//    called from importmusicxml
//---------------------------------------------------------

void Part::setMidiChannel(int) const
      {
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void Part::setInstrument(const Instrument& i, int tick)
      {
      _instrList.setInstrument(i, tick);
      }

//---------------------------------------------------------
//   removeInstrument
//---------------------------------------------------------

void Part::removeInstrument(int tick)
      {
      _instrList.erase(tick);
      }

//---------------------------------------------------------
//   instr
//---------------------------------------------------------

Instrument* Part::instr(int tick)
      {
      return &_instrList.instrument(tick);
      }

//---------------------------------------------------------
//   instr
//---------------------------------------------------------

const Instrument* Part::instr(int tick) const
      {
      return &_instrList.instrument(tick);
      }

