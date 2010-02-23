//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: importmidi.cpp 2721 2010-02-15 19:41:28Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "importpdf.h"
#include "score.h"
#include "omr/omr.h"

//---------------------------------------------------------
//   importPdf
//---------------------------------------------------------

bool Score::importPdf(const QString& path)
      {
      _omr = new Omr(path);
      if (!_omr->read()) {
            delete _omr;
            _omr = 0;
            return false;
            }
      _spatium = _omr->spatiumMM() * DPMM;

      setShowOmr(true);
      return true;
      }

