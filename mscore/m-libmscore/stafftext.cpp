//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: stafftext.cpp 3649 2010-11-01 08:49:38Z wschweer $
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "score.h"
#include "stafftext.h"
#include "system.h"
#include "staff.h"
#include "m-al/xml.h"

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_STAFF);
      setTextStyle(TEXT_STYLE_STAFF);
      _setAeolusStops = false;
      clearAeolusStops();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(XmlReader* r)
      {
      for (int voice = 0; voice < VOICES; ++voice)
            _channelNames[voice].clear();
      clearAeolusStops();

      while (r->readElement()) {
            MString8 tag = r->tag();
#if 0
            if (tag == "MidiAction") {
                  int channel = e.attribute("channel", 0).toInt();
                  QString name = e.attribute("name");
                  bool found = false;
                  int n = _channelActions.size();
                  for (int i = 0; i < n; ++i) {
                        ChannelActions* a = &_channelActions[i];
                        if (a->channel == channel) {
                              a->midiActionNames.append(name);
                              found = true;
                              break;
                              }
                        }
                  if (!found) {
                        ChannelActions a;
                        a.channel = channel;
                        a.midiActionNames.append(name);
                        _channelActions.append(a);
                        }
                  }
            else if (tag == "channelSwitch" || tag == "articulationChange") {
                  int voice = e.attribute("voice", "-1").toInt();
                  if (voice >= 0 && voice < VOICES)
                        _channelNames[voice] = e.attribute("name");
                  else if (voice == -1) {
                        // no voice applies channel to all voices for
                        // compatibility
                        for (int i = 0; i < VOICES; ++i)
                              _channelNames[i] = e.attribute("name");
                        }
                  }
            else if (tag == "aeolus") {
                  int group = e.attribute("group", "-1").toInt();
                  if (group >= 0 && group <= 4) {
                        aeolusStops[group] = e.text().toInt();
                        }
                  _setAeolusStops = true;
                  }
#endif
            if (!Text::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   clearAeolusStops
//---------------------------------------------------------

void StaffText::clearAeolusStops()
      {
      for (int i = 0; i < 4; ++i)
            aeolusStops[i] = 0;
      }

//---------------------------------------------------------
//   setAeolusStop
//---------------------------------------------------------

void StaffText::setAeolusStop(int group, int idx, bool val)
      {
      if (val)
            aeolusStops[group] |= (1 << idx);
      else
            aeolusStops[group] &= ~(1 << idx);
      }

//---------------------------------------------------------
//   getAeolusStop
//---------------------------------------------------------

bool StaffText::getAeolusStop(int group, int idx) const
      {
      return aeolusStops[group] & (1 << idx);
      }

