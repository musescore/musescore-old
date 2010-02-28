//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "omr.h"
#include "omrview.h"
#include "omr.h"
#include "xml.h"
#include "page.h"
#include "pdf.h"
#include "ocr.h"

//---------------------------------------------------------
//   Omr
//---------------------------------------------------------

Omr::Omr()
      {
      _ocr = 0;
      }

Omr::Omr(const QString& p)
      {
      _path = p;
      _ocr = 0;
      }

//---------------------------------------------------------
//   newOmrView
//---------------------------------------------------------

OmrView* Omr::newOmrView()
      {
      OmrView* sv = new OmrView;
      sv->setOmr(this);
      return sv;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Omr::write(Xml& xml) const
      {
      xml.stag("Omr");
      xml.tag("path", _path);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Omr::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "path") {
                  _path = val;
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   pagesInDocument
//---------------------------------------------------------

int Omr::pagesInDocument() const
      {
      return _doc ? _doc->numPages() : 0;
      }

//---------------------------------------------------------
//   read
//    return true on success
//---------------------------------------------------------

bool Omr::read()
      {
      if (_ocr == 0)
            _ocr = new Ocr;
      _ocr->init();

      _doc = new Pdf(_path);

      int n = _doc->numPages();
      for (int i = 0; i < n; ++i) {
            Page* page = new Page(this);
            QImage image = _doc->page(i);
            page->setImage(image);
            pages.append(page);
            }
      double sp = 0;
      double w  = 0;
      for (int i = 0; i < n; ++i) {
            pages[i]->read(i);
            sp += pages[i]->spatium();
            w  += pages[i]->width();
            }
      _spatium = sp / n;
      w       /= n;
      _dpmm    = w / 210.0;            // PaperSize A4

printf("*** spatium: %f mm  dpmm: %f\n", spatiumMM(), _dpmm);
      return true;
      }

//---------------------------------------------------------
//   spatiumMM
//---------------------------------------------------------

double Omr::spatiumMM() const
      {
      return _spatium / _dpmm;
      }

//---------------------------------------------------------
//   staffDistance
//---------------------------------------------------------

double Omr::staffDistance() const
      {
      return pages[0]->staffDistance();
      }

//---------------------------------------------------------
//   systemDistance
//---------------------------------------------------------

double Omr::systemDistance() const
      {
      return pages[0]->systemDistance();
      }




