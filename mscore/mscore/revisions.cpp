//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id:$
//
//  Copyright (C) 2010 by Werner Schweer and others
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

#include "revisions.h"
#include "xml.h"

//---------------------------------------------------------
//   Revision
//---------------------------------------------------------

Revision::Revision()
      {
      _parent = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Revision::write(Xml& xml) const
      {
      xml.stag("Revision");
      xml.tag("id",   _id);
      xml.tag("date", _dateTime.toString());
      xml.tag("diff", _diff);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Revision::read(QDomElement e)
      {
      _dateTime = QDateTime::currentDateTime();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "id")
                  _id = val;
            else if (tag == "diff")
                  _diff = val;
            else if (tag == "date")
                  _dateTime = QDateTime::fromString(val);
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   Revisions
//---------------------------------------------------------

Revisions::Revisions()
      {
      _trunk = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Revisions::write(Xml& xml) const
      {
      for (Revision* r = _trunk; r; r = r->parent())
            write(xml, r);
      }

void Revisions::write(Xml& xml, const Revision* r) const
      {
      r->write(xml);
      foreach(const Revision* rr, r->branches())
            write(xml, rr);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Revisions::add(Revision* r)
      {
      if (_trunk == 0) {
            _trunk = r;
            _trunk->setParent(0);
            return;
            }
      }

//---------------------------------------------------------
//   getRevision
//---------------------------------------------------------

QString Revisions::getRevision(QString id)
      {
      return QString();
      }

