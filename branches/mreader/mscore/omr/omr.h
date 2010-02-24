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

class ScanView;
class Scan;
class Xml;

//---------------------------------------------------------
//   Omr
//---------------------------------------------------------

class Omr {
      QString _path;
      Scan* _scan;

   public:
      Omr() { _scan = 0; }
      Omr(const QString& path);
      bool read();
      ScanView* newScanView() const;
      void write(Xml&) const;
      void read(QDomElement e);
      double spatiumMM() const;           // spatium in millimeter
      double staffDistance() const;
      double systemDistance() const;
      };

#endif



