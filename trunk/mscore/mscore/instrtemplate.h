//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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

#ifndef __INSTRTEMPLATE_H__
#define __INSTRTEMPLATE_H__

#include "ui_instrdialog.h"
#include "globals.h"
#include "instrument.h"

class Xml;
class Part;
class Staff;

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

struct InstrumentTemplate {
      QString group;
      QString trackName;     ///< also used for track name
      QString name;          ///< shown on first system
      QString shortName;     ///< shown on followup systems

      int staves;             // 1 <= MAX_STAVES
      int clefIdx[MAX_STAVES];
      int staffLines[MAX_STAVES];
      bool smallStaff[MAX_STAVES];
      int bracket;            // bracket type (NO_BRACKET)

      char minPitchA;         // pitch range playable by an amateur
      char maxPitchA;
      char minPitchP;         // pitch range playable by professional
      char maxPitchP;

      char transposeChromatic;      // for transposing instruments
      char transposeDiatonic;

      bool useDrumset;
      Drumset* drumset;
      QList<NamedEventList> midiActions;
      QList<MidiArticulation*> articulation;

      QList<Channel*> channel;

      InstrumentTemplate();
      InstrumentTemplate(const InstrumentTemplate&);
      ~InstrumentTemplate();

      void setPitchRange(const QString& s, char* a, char* b) const;
      void write(Xml& xml) const;
      void read(const QString& group, QDomElement);
      };

extern QList<InstrumentTemplate*> instrumentTemplates;
extern QList<MidiArticulation*> articulation;

extern bool loadInstrumentTemplates(const QString& instrTemplates);

//---------------------------------------------------------
//   InstrumentTemplateListItem
//---------------------------------------------------------

class InstrumentTemplateListItem : public QTreeWidgetItem {
      InstrumentTemplate* _instrumentTemplate;
      QString _group;

   public:
      InstrumentTemplateListItem(QString group, QTreeWidget* parent);
      InstrumentTemplateListItem(InstrumentTemplate* i, InstrumentTemplateListItem* parent);
      InstrumentTemplateListItem(InstrumentTemplate* i, QTreeWidget* parent);

      InstrumentTemplate* instrumentTemplate() const { return _instrumentTemplate; }
      virtual QString text(int col) const;
      };

enum { ITEM_KEEP, ITEM_DELETE, ITEM_ADD };
enum { PART_LIST_ITEM = QTreeWidgetItem::UserType, STAFF_LIST_ITEM };

//---------------------------------------------------------
//   PartListItem
//---------------------------------------------------------

class PartListItem : public QTreeWidgetItem {

   public:
      int op;
      Part* part;
      const InstrumentTemplate* it;

      PartListItem(Part* p, QTreeWidget* lv);
      PartListItem(const InstrumentTemplate* i, QTreeWidget* lv);
      };

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

class StaffListItem : public QTreeWidgetItem {
      int _clef;
      int _partIdx;

   public:
      StaffListItem();
      StaffListItem(PartListItem* li);

      int op;
      Staff* staff;
      int partIdx() const      { return _partIdx; }
      void setPartIdx(int val);
      int staffIdx;

      void setClef(int val);
      int clef() const { return _clef; }
      };

#endif

