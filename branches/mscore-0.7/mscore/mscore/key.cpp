//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: key.cpp,v 1.8 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

void KeyList::read(QDomElement e, Score* cs)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "key") {
                  int tick = e.attribute("tick", "0").toInt();
                  int idx  = e.attribute("idx", "0").toInt();
                  (*this)[cs->fileDivision(tick)] = idx;
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void KeyList::removeTime(int tick, int len)
      {
      KeyList tmp;
      for (ciKeyEvent i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0)) {
                  if (i->first >= tick + len)
                        tmp[i->first - len] = i->second;
                  else
                        printf("remove key event\n");
                  }
            else
                  tmp[i->first] = i->second;
            }
      clear();
      insert(tmp.begin(), tmp.end());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void KeyList::insertTime(int tick, int len)
      {
      KeyList tmp;
      for (ciKeyEvent i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0))
                  tmp[i->first + len] = i->second;
            else
                  tmp[i->first] = i->second;
            }
      clear();
      insert(tmp.begin(), tmp.end());
      }

