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

#include "noteevent.h"
#include "xml.h"

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteEvent::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "pitch")
                  _pitch = i;
            else if (tag == "ontime")
                  _ontime = i;
            else if (tag == "len")
                  _len = i;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteEvent::write(Xml& xml) const
      {
      xml.stag("Event");
      xml.tag("pitch", _pitch);
      xml.tag("ontime", _ontime);
      xml.tag("len", _len);
      xml.etag();
      }
