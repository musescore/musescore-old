//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __INSTRTEMPLATE_H__
#define __INSTRTEMPLATE_H__

#include "ui_instrdialog.h"
#include "globals.h"

class Xml;

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

struct InstrumentTemplate {
      QString group;
      QString name;           // also used for track name
      QString shortName;
      int staves;             // 1 <= MAX_STAVES
      int clefIdx[MAX_STAVES];
      int staffLines[MAX_STAVES];
      int smallStaff[MAX_STAVES];
      int bracket;            // bracket type (NO_BRACKET)
      int midiProgram;
      int minPitch;
      int maxPitch;
      int transpose;          // for transposing instruments
      bool useDrumset;

      void write(Xml& xml) const;
      void read(const QString& group, QDomElement);
      };

extern QList<InstrumentTemplate*> instrumentTemplates;
extern bool loadInstrumentTemplates(const QString& instrTemplates);

#endif

