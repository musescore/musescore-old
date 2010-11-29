//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __STAFFTYPE_H__
#define __STAFFTYPE_H__

#include "spatium.h"
#include "globals.h"

class Staff;
class Xml;

//---------------------------------------------------------
//   StaffType
//---------------------------------------------------------

class StaffType {
      bool _modified;         // if true, this StaffType belongs to Score(),
                              // otherwise it is a global build in
      QString _name;
      StaffGroup _group;
      uchar _lines;
      Spatium _lineDistance;
      bool _genClef;          // create clef at beginning of system
      bool _genKeysig;        // create key signature at beginning of system
      bool _slashStyle;       // do not show stems
      bool _showBarlines;
      bool _showLedgerLines;

   public:
      StaffType();
      StaffType(const QString& s);
      QString name() const                     { return _name;            }
      void setName(const QString& val)         { _name = val;             }
      void setGroup(StaffGroup g)              { _group = g;              }
      StaffGroup group() const                 { return _group;           }
      void setLines(int val)                   { _lines = val;            }
      int lines() const                        { return _lines;           }
      void setLineDistance(const Spatium& val) { _lineDistance = val;     }
      Spatium lineDistance() const             { return _lineDistance;    }
      void setGenClef(bool val)                { _genClef = val;          }
      bool genClef() const                     { return _genClef;         }
      void setGenKeysig(bool val)              { _genKeysig = val;        }
      bool genKeysig() const                   { return _genKeysig;       }
      void setSlashStyle(bool val)             { _slashStyle = val;       }
      bool slashStyle() const                  { return _slashStyle;      }
      void setShowBarlines(bool val)           { _showBarlines = val;     }
      bool showBarlines() const                { return _showBarlines;    }
      void setShowLedgerLines(bool val)        { _showLedgerLines = val;  }
      bool showLedgerLines() const             { return _showLedgerLines; }
      bool modified() const                    { return _modified;        }
      void setModified(bool val)               { _modified = val;         }
      void write(Xml& xml, int) const;
      void read(QDomElement);
      };

// first three staff types in staffTypes[] are build in:

enum {
      PITCHED_STAFF_TYPE, TAB_STAFF_TYPE, PERCUSSION_STAFF_TYPE
      };

//---------------------------------------------------------
//   StaffTypePitched
//---------------------------------------------------------

class StaffTypePitched : public StaffType {

   public:
      StaffTypePitched() : StaffType() {}
      StaffTypePitched(const QString& s) : StaffType(s) {}
      };

//---------------------------------------------------------
//   StaffTypeTablature
//---------------------------------------------------------

class StaffTypeTablature : public StaffType {

   public:
      StaffTypeTablature() : StaffType() {}
      StaffTypeTablature(const QString& s) : StaffType(s) {}
      };

//---------------------------------------------------------
//   StaffTypePercussion
//---------------------------------------------------------

class StaffTypePercussion : public StaffType {

   public:
      StaffTypePercussion() : StaffType() {}
      StaffTypePercussion(const QString& s) : StaffType(s) {}
      };

extern void initStaffTypes();
extern QList<StaffType*> staffTypes;

#endif
