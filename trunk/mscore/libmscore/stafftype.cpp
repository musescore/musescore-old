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

#include <QtCore/QList>

#include "stafftype.h"
#include "staff.h"
#include "score.h"
#include "al/xml.h"

QList<StaffType*> defaultStaffTypes;

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

void initStaffTypes()
      {
      StaffType* st = new StaffType("Pitched 5 lines");
      st->setGroup(PITCHED_STAFF);
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(true);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      defaultStaffTypes.append(st);

      st = new StaffType("Tab");
      st->setGroup(TAB_STAFF);
      st->setLines(6);
      st->setLineDistance(Spatium(1.5));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(false);
      defaultStaffTypes.append(st);

      st = new StaffType("Percussion 5 lines");
      st->setGroup(PERCUSSION_STAFF);
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      defaultStaffTypes.append(st);
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
//   read
//---------------------------------------------------------

void StaffType::read(XmlReader* r)
      {
      while (r->readElement()) {
            int v;
            QString s;
            qreal d;

            if (r->readString("name", &s))
                  setName(s);
            else if (r->readInt("lines", &v))
                  setLines(v);
            else if (r->readReal("lineDistance", &d))
                  setLineDistance(Spatium(d));
            else if (r->readInt("clef", &v))
                  setGenClef(v);
            else if (r->readInt("keysig", &v))
                  setGenKeysig(v);
            else if (r->readInt("slashStyle", &v))
                  setSlashStyle(v);
            else if (r->readInt("barlines", &v))
                  setShowBarlines(v);
            else if (r->readInt("ledgerlines", &v))
                  setShowLedgerLines(v);
            else
                  r->unknown();
            }
      }

