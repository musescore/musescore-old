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
class Tablature;

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

struct InstrumentTemplate {
      QString trackName;
      QList<StaffName> longNames;      ///< shown on first system
      QList<StaffName> shortNames;     ///< shown on followup systems

      char minPitchA;         // pitch range playable by an amateur
      char maxPitchA;
      char minPitchP;         // pitch range playable by professional
      char maxPitchP;

      Interval transpose;     // for transposing instruments

      bool useDrumset;
      Drumset* drumset;

      QList<NamedEventList>   midiActions;
      QList<MidiArticulation> articulation;
      QList<Channel>          channel;

      int staves;             // 1 <= MAX_STAVES
      ClefType clefIdx[MAX_STAVES];
      int staffLines[MAX_STAVES];
      bool useTablature;
      Tablature* tablature;
      int bracket[MAX_STAVES];            // bracket type (NO_BRACKET)
      int bracketSpan[MAX_STAVES];
      int barlineSpan[MAX_STAVES];
      bool smallStaff[MAX_STAVES];

      bool extended;          // belongs to extended instrument set if true

      InstrumentTemplate();
      InstrumentTemplate(const InstrumentTemplate&);
      ~InstrumentTemplate();
      void init(const InstrumentTemplate&);

      void setPitchRange(const QString& s, char* a, char* b) const;
      void write(Xml& xml) const;
      void read(QDomElement);
      };

//---------------------------------------------------------
//   InstrumentGroup
//---------------------------------------------------------

struct InstrumentGroup {
      QString name;
      bool extended;
      QList<InstrumentTemplate*> instrumentTemplates;
      };

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
      ClefType _clef;
      int _partIdx;
      bool _linked;

   public:
      StaffListItem();
      StaffListItem(PartListItem* li);

      int op;
      Staff* staff;
      int partIdx() const      { return _partIdx; }
      void setPartIdx(int val);
      int staffIdx;

      void setClef(ClefType val);
      ClefType clef() const    { return _clef;    }
      void setLinked(bool val);
      bool linked() const      { return _linked;  }
      bool visible() const;
      void setVisible(bool val);
      };

extern QList<InstrumentGroup*> instrumentGroups;
extern QList<MidiArticulation*> articulation;
extern bool loadInstrumentTemplates(const QString& instrTemplates);
extern InstrumentTemplate* searchTemplate(const QString& name);
extern void populateInstrumentList(QTreeWidget* instrumentList, bool extended);
#endif

