//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chordrest.h,v 1.4 2006/03/03 21:47:11 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __ARTICULATION_H__
#define __ARTICULATION_H__

#include "ui_articulation.h"
#include "symbol.h"

//---------------------------------------------------------
//   ArticulationIdx
//---------------------------------------------------------

enum ArticulationIdx {
      UfermataSym,
      DfermataSym,
      ThumbSym,
      SforzatoaccentSym,
      EspressivoSym,
      StaccatoSym,
      UstaccatissimoSym,
      DstaccatissimoSym,
      TenutoSym,
      UportatoSym,
      DportatoSym,
      UmarcatoSym,
      DmarcatoSym,
      OuvertSym,
      PlusstopSym,
      UpbowSym,
      DownbowSym,
      ReverseturnSym,
      TurnSym,
      TrillSym,
      PrallSym,
      MordentSym,
      PrallPrallSym,
      PrallMordentSym,
      UpPrallSym,
      DownPrallSym,
      UpMordentSym,
      DownMordentSym,
      ARTICULATIONS
      };

//---------------------------------------------------------
//   ArticulationInfo
//    gives infos about note attributes
//---------------------------------------------------------

enum ArticulationAnchor {
      A_TOP_STAFF,
      A_BOTTOM_STAFF,
      A_CHORD,          // anchor depends on chord direction
      A_TOP_CHORD,      // attribute is alway placed at top of chord
      A_BOTTOM_CHORD,   // attribute is placed at bottom of chord
      };

struct ArticulationInfo {
      int sym;
      QString name;
      static int name2index();
      };

//---------------------------------------------------------
//   Articulation
//    articulation marks
//---------------------------------------------------------

class Articulation : public Symbol {
      QString _channelName;

      virtual bool isMovable() const { return true; }

   public:
      Articulation(Score*);
      Articulation &operator=(const Articulation&);

      virtual Articulation* clone() const { return new Articulation(*this); }
      virtual ElementType type() const     { return ATTRIBUTE; }

      virtual void setSubtype(int);
      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;
      QString name() const { return articulationList[subtype()].name; }

      static ArticulationInfo articulationList[];

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      ArticulationAnchor anchor() const;

      QString channelName() const           { return _channelName; }
      void setChannelName(const QString& s) { _channelName = s;    }

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      static QString idx2name(int idx);
      static int name2idx(const QString& name);
      };

//---------------------------------------------------------
//   ArticulationProperties
//    Dialog
//---------------------------------------------------------

class ArticulationProperties : public QDialog, public Ui::ArticulationProperties {
      Q_OBJECT

      Articulation* noteAttribute;

   private slots:
      void saveValues();

   public:
      ArticulationProperties(Articulation*, QWidget* parent = 0);
      };

typedef QList<Articulation*>::iterator iArticulation;
typedef QList<Articulation*>::const_iterator ciArticulation;

#endif

