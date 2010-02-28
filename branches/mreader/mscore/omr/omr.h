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

#ifndef __OMR_H__
#define __OMR_H__

class OmrView;
class Xml;
class Pdf;
class Page;
class Ocr;

//---------------------------------------------------------
//   Omr
//---------------------------------------------------------

class Omr {
      QString _path;
      double _spatium;
      double _dpmm;
      Pdf* _doc;
      QList<Page*> pages;
      Ocr* _ocr;

      void process1(int page);

   public:
      Omr();
      Omr(const QString& path);
      bool read();
      int pagesInDocument() const;
      int numPages() const                 { return pages.size();          }
      Page* page(int idx)                  { return pages[idx];            }
      OmrView* newOmrView();
      Ocr* ocr() const                     { return _ocr; }

      void write(Xml&) const;
      void read(QDomElement e);

      double spatiumMM() const;           // spatium in millimeter
      double spatium() const               { return _spatium; }
      double dpmm() const                  { return _dpmm;    }
      double staffDistance() const;
      double systemDistance() const;
      };

#endif
