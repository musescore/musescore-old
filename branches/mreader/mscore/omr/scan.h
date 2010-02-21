//=============================================================================
//  MusE Reader
//  Linux Music Score Reader
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

#ifndef __SCAN_H__
#define __SCAN_H__

class Page;
class Pdf;

//---------------------------------------------------------
//   Scan
//---------------------------------------------------------

class Scan {
      QString _pdfPath;
      double _spatium;
      double _dpmm;

      Pdf* _doc;
      QList<Page*> pages;

      void process1(int page);

   public:
      Scan();
      bool read(const QString& path);
      Page* page(int idx)                  { return pages[idx];            }
      int numPages() const                 { return pages.size();          }
      int pagesInDocument() const;
      double spatiumMM() const;           // spatium in millimeter
      double spatium() const               { return _spatium; }
      double dpmm() const                  { return _dpmm;    }
      };

#endif

