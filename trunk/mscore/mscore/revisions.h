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

#ifndef __REVISIONS_H__
#define __REVISIONS_H__

class Xml;

//---------------------------------------------------------
//   Revision
//---------------------------------------------------------

class Revision {
      int _id;
      QString _diff;           // diff to parent
      Revision* _parent;
      QList<Revision*> _branches;
      QDateTime _dateTime;

   public:
      Revision();
      void read(QDomElement);
      };

//---------------------------------------------------------
//   Revisions
//    id:  2.3.1
//         | | +-- revision of branch
//         | +---- branch number
//         +------ revision
//---------------------------------------------------------

class Revisions {
      Revision* _trunk;

   public:
      Revisions();
      void add(Revision*);
      QString getRevision(QString id);
      Revision* trunk() { return _trunk; }
      void write(Xml&);
      };

#endif

