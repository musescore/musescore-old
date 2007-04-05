//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: key.cpp,v 1.8 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "key.h"
#include "xml.h"
#include "utils.h"
#include "score.h"

//---------------------------------------------------------
//   key
//---------------------------------------------------------

int KeyList::key(int tick) const
      {
      if (empty())
            return 0;
      ciKeyEvent i = upper_bound(tick);
      if (i == begin())
            return 0;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   KeyList::write
//---------------------------------------------------------

void KeyList::write(Xml& xml, const char* name) const
      {
      xml.stag(name);
      for (ciKeyEvent i = begin(); i != end(); ++i)
            xml.tagE("key tick=\"%d\" idx=\"%d\"", i->first, i->second);
      xml.etag();
      }

//---------------------------------------------------------
//   KeyList::read
//---------------------------------------------------------

void KeyList::read(QDomNode node, Score* cs)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "key") {
                  int tick = e.attribute("tick", "0").toInt();
                  int idx  = e.attribute("idx", "0").toInt();
                  (*this)[cs->fileDivision(tick)] = idx;
                  }
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void KeyList::insertTime(int, int)
      {
      printf("KeyList::insertTime(): not impl.\n");
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void KeyList::removeTime(int, int)
      {
      printf("KeyList::removeTime(): not impl.\n");
      }

