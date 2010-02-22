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
#include "scanview.h"
#include "scan.h"
#include "xml.h"

//---------------------------------------------------------
//   Omr
//---------------------------------------------------------

Omr::Omr(const QString& p)
      {
      _path = p;
      _scan = 0;
      }

//---------------------------------------------------------
//   read
//    return true on success
//---------------------------------------------------------

bool Omr::read()
      {
      if (_scan == 0)
            _scan = new Scan;
      if (!_scan->read(_path)) {
            delete _scan;
            _scan = 0;
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   newScanView
//---------------------------------------------------------

ScanView* Omr::newScanView() const
      {
      ScanView* sv = new ScanView;
      sv->setScan(_scan);
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
//   spatiumMM
//---------------------------------------------------------

double Omr::spatiumMM() const
      {
      return _scan->spatiumMM();
      }

