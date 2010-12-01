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

#include "stafftype.h"
#include "staff.h"
#include "score.h"
#include "xml.h"

QList<StaffType*> staffTypes;

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

void initStaffTypes()
      {
      StaffType* st = new StaffTypePitched("Pitched 5 lines");
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(true);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      staffTypes.append(st);

      st = new StaffTypeTablature("Tab");
      st->setLines(6);
      st->setLineDistance(Spatium(1.5));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(false);
      staffTypes.append(st);

      st = new StaffTypePercussion("Percussion 5 lines");
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      staffTypes.append(st);
      }

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

StaffType::StaffType()
      {
      _modified = false;
      }

StaffType::StaffType(const QString& s)
      {
      _name = s;
      _modified = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffType::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\" group=\"%2\"").arg(idx).arg(groupName()));
      xml.tag("name", name());
      if (lines() != 5)
            xml.tag("lines", lines());
      if (lineDistance().val() != 1.0)
            xml.tag("lineDistance", lineDistance().val());
      if (!genClef())
            xml.tag("clef", genClef());
      if (!genKeysig())
            xml.tag("keysig", genKeysig());
      if (slashStyle())
            xml.tag("slashStyle", slashStyle());
      if (!showBarlines())
            xml.tag("barlines", showBarlines());
      if (!showLedgerLines())
            xml.tag("ledgerlines", showLedgerLines());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffType::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int v = e.text().toInt();
            if (tag == "name")
                  setName(e.text());
            else if (tag == "lines")
                  setLines(v);
            else if (tag == "lineDistance")
                  setLineDistance(Spatium(e.text().toDouble()));
            else if (tag == "clef")
                  setGenClef(v);
            else if (tag == "keysig")
                  setGenKeysig(v);
            else if (tag == "slashStyle")
                  setSlashStyle(v);
            else if (tag == "barlines")
                  setShowBarlines(v);
            else if (tag == "ledgerlines")
                  setShowLedgerLines(v);
            else
                  domError(e);
            }
      }

