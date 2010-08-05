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
      QString _id;
      QString _diff;          // diff to parent
      QDateTime _dateTime;
      Revision* _parent;
      QList<Revision*> _branches;

   public:
      Revision();
      void read(QDomElement);
      void write(Xml&) const;
      void setParent(Revision* r)              { _parent = r; }
      Revision* parent() const                 { return _parent; }
      const QList<Revision*>& branches() const { return _branches; }
      void setId(const QString& s)             { _id = s; }
      void setDiff(const QString& s)           { _diff = s; }
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

      void write(Xml&, const Revision*) const;

   public:
      Revisions();
      void add(Revision*);
      QString getRevision(QString id);
      Revision* trunk() { return _trunk; }
      void write(Xml&) const;
      };

#endif

