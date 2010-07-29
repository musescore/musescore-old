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
      _id     = 0;
      _parent = 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Revision::read(QDomElement)
      {
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

void Revisions::write(Xml&)
      {
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Revisions::add(Revision*)
      {
      }

//---------------------------------------------------------
//   getRevision
//---------------------------------------------------------

QString Revisions::getRevision(QString id)
      {
      return QString();
      }

