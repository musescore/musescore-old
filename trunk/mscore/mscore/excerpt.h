//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

class Score;
class Part;
class Xml;

//---------------------------------------------------------
//   Excerpt
//---------------------------------------------------------

class Excerpt {
      Score* _score;
      QString _name;
      QString _title;
      QList<Part*> _parts;

   public:
      Excerpt(Score* s)               { _score = s; }
      QList<Part*>* parts()           { return &_parts; }
      Score* score() const            { return _score;  }

      void write(Xml&) const;
      void read(QDomElement);

      bool operator!=(const Excerpt&) const;
      };

extern Score* createExcerpt(const QList<Part*>&);
extern void cloneStaves(Score* oscore, Score* score, const QList<int>& map);

#endif

