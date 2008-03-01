//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "excerpt.h"
#include "score.h"
#include "part.h"
#include "xml.h"
#include "staff.h"
#include "layout.h"
#include "box.h"
#include "canvas.h"
#include "style.h"
#include "page.h"

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Excerpt::read(QDomElement e)
      {
      QList<Part*>* pl = score->parts();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag = e.tagName();
            if (tag == "name")
                  _name = e.text();
            else if (tag == "title")
                  _title = e.text();
            else if (tag == "part") {
                  int partIdx = e.text().toInt();
                  if (partIdx < 0 || partIdx >= pl->size())
                        printf("Excerpt::read: bad part index\n");
                  else
                        _parts.append(pl->at(partIdx));
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Excerpt::write(Xml& xml) const
      {
      xml.stag("Excerpt");
      xml.tag("name", _name);
      xml.tag("title", _title);
      QList<Part*>* pl = score->parts();
      foreach(Part* part, _parts)
            xml.tag("part", pl->indexOf(part));
      xml.etag();
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool Excerpt::operator!=(const Excerpt& e) const
      {
      if (e.score != score)
            return true;
      if (e._name != _name)
            return true;
      if (e._title != _title)
            return true;
      if (e._parts != _parts)
            return true;
      return false;
      }

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

Score* Score::createExcerpt(Excerpt* excerpt)
      {
      Score* s = new Score();
      QFileInfo* fi = s->fileInfo();
      QString name = fileInfo()->path() + "/" + excerpt->name() + ".msc";
      fi->setFile(name);

      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml << "<museScore version=\"1.4\">\n";
      writeExcerpt(excerpt, xml);
      xml << "</museScore>\n";
      buffer.close();

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(buffer.data(), &err, &line, &column)) {
            printf("error reading excerpt data at %d/%d\n<%s>\n",
            line, column, err.toLatin1().data());
            return 0;
            }
      docName = "--";
      QDomElement e = doc.documentElement();
      s->read(e);
      s->renumberMeasures();
      s->setCreated(true);
      return s;
      }

//---------------------------------------------------------
//   writeExcerpt
//---------------------------------------------------------

void Score::writeExcerpt(Excerpt* excerpt, Xml& xml)
      {
      xml.tag("Division", division);
      xml.tag("Spatium", _spatium / DPMM);
      xml.curTrack  = -1;
      xml.trackDiff = 0;

      if (editObject) {                          // in edit mode?
            endCmd();
            canvas()->setState(Canvas::NORMAL);  //calls endEdit()
            }
      _style->save(xml);
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (*_textStyles[i] != defaultTextStyleArray[i])
                  _textStyles[i]->write(xml);
            }
      xml.tag("showInvisible", _showInvisible);
      pageFormat()->write(xml);
      if (rights) {
            xml.stag("rights");
            xml << rights->toHtml("UTF-8") << '\n';
            xml.etag();
            }
      if (!movementNumber.isEmpty())
            xml.tag("movement-number", movementNumber);
      if (!movementTitle.isEmpty())
            xml.tag("movement-title", movementTitle);

      sigmap->write(xml);
      tempomap->write(xml);
      foreach(const Part* part, _parts) {
            int idx = excerpt->parts()->indexOf((Part*)part);
            if (idx != -1)
                  part->write(xml);
            }
      int trackOffset[_staves.size()];
      int idx = 0, didx = 0;

      static const int HIDDEN = 100000;

      foreach(Staff* staff, _staves) {
            int i = excerpt->parts()->indexOf(staff->part());
            if (i == -1) {
                  trackOffset[idx] = HIDDEN;
                  ++idx;
                  continue;
                  }
            trackOffset[idx] = (didx - idx) * VOICES;
            ++idx;
            ++didx;
            }
      xml.trackDiff = 0;
      foreach(Element* el, _layout->_gel) {
            if (el->track() != -1) {
                  int staffIdx = el->staffIdx();
                  if (trackOffset[staffIdx] == HIDDEN)
                        continue;
                  xml.trackDiff = trackOffset[staffIdx];
                  }
            el->write(xml);
            }
      bool isFirstStaff = true;
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Part* part = staff(staffIdx)->part();
            int idx = excerpt->parts()->indexOf(part);
            if (idx == -1)
                  continue;
            xml.curTick   = 0;
            xml.curTrack  = staffIdx * VOICES;
            xml.trackDiff = trackOffset[staffIdx];
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1 + xml.trackDiff/4));
            for (MeasureBase* m = _layout->first(); m; m = m->next()) {
                  if (isFirstStaff || m->type() == MEASURE)
                        m->write(xml, staffIdx, isFirstStaff);
                  if (m->type() == MEASURE)
                        xml.curTick = m->tick() + sigmap->ticksMeasure(m->tick());
                  }
            xml.etag();
            isFirstStaff = false;
            }
      xml.tag("cursorTrack", _is.track);
      }

