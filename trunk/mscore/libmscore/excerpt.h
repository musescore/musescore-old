//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2008-2011 Werner Schweer
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

#ifndef __EXCERPT_H__
#define __EXCERPT_H__

#include <QtCore/QString>
#include <QtCore/QList>

class Score;
class Part;
class Xml;
class Staff;

//---------------------------------------------------------
//   Excerpt
//---------------------------------------------------------

class Excerpt {
      Score* _score;
      QString _title;
      QList<Part*> _parts;

   public:
      Excerpt(Score* s)               { _score = s; }
      QList<Part*>* parts()           { return &_parts; }
      Score* score() const            { return _score;  }
      void setScore(Score* s)         { _score = s; }

//      void read(XmlReader*);

      bool operator!=(const Excerpt&) const;
      QString title() const           { return _title; }
      void setTitle(const QString& s) { _title = s; }
      };

extern Score* createExcerpt(const QList<Part*>&);
extern void cloneStaves(Score* oscore, Score* score, const QList<int>& map);
extern void cloneStaff(Staff* ostaff, Staff* nstaff);

#endif

