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
      Score* score;
      QString _name;
      QString _title;
      QList<Part*> _parts;

   public:
      Excerpt(Score* s)               { score = s; }
      QString name() const            { return _name;   }
      QString title() const           { return _title;  }
      QList<Part*>* parts()           { return &_parts; }
      void setName(const QString& s)  { _name = s;      }
      void setTitle(const QString& s) { _title = s;     }

      void write(Xml&) const;
      void read(QDomElement);

      bool operator!=(const Excerpt&) const;
      };

#endif

