//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chordrest.h,v 1.4 2006/03/03 21:47:11 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __CHORDREST_H__
#define __CHORDREST_H__

#include "element.h"

class Score;
class Measure;
class Beam;
class Tuplet;
class Segment;

enum BeamMode { BEAM_AUTO, BEAM_BEGIN, BEAM_MID, BEAM_END,
      BEAM_NO, BEAM_BEGIN32 };

const int NOTE_ATTRIBUTES = 28;

enum NoteAttributeIdx {
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
      DownMordentSym
      };

//---------------------------------------------------------
//   AttributeInfo
//    gives infos about note attributes
//---------------------------------------------------------

enum AttrAnchor {
      A_TOP_STAFF, 
      A_BOTTOM_STAFF, 
      A_CHORD,          // anchor depends on chord direction
      A_TOP_CHORD,      // attribute is alway placed at top of chord
      A_BOTTOM_CHORD,   // attribute is placed at bottom of chord
      };

struct AttributeInfo {
      Sym sym;
      QString name;
      AttrAnchor anchor;
      };

//---------------------------------------------------------
//   NoteAttribute
//    Artikulationszeichen
//---------------------------------------------------------

class NoteAttribute : public Symbol {
   public:

   protected:
      virtual bool startDrag(const QPointF&) { return true; }

   public:
      NoteAttribute(Score*);
      NoteAttribute(Score*, int);
      NoteAttribute(const NoteAttribute&);
      NoteAttribute &operator=(const NoteAttribute&);

      virtual ElementType type() const { return ATTRIBUTE; }

      virtual void setSubtype(int);
      virtual void read(QDomNode);
      virtual void write(Xml& xml) const;
      virtual QRectF drag(const QPointF& s);
      QString name() const { return atrList[subtype()].name; }

      static AttributeInfo atrList[];
      };

typedef pstl::plist<NoteAttribute*>::iterator iAttribute;
typedef pstl::plist<NoteAttribute*>::const_iterator ciAttribute;

//---------------------------------------------------------
//   ChordRest
//    chords and rests can be part of a beam
//---------------------------------------------------------

class ChordRest : public Element {

   protected:
      pstl::plist<NoteAttribute*> attributes;
      Beam* _beam;
      BeamMode _beamMode;
      Tuplet* _tuplet;
      bool _up;

      void layoutAttributes();

   public:
      ChordRest(Score*);
      ChordRest(const ChordRest&);
      ChordRest &operator=(const ChordRest&);
      virtual ElementType type() const = 0;

      void writeProperties(Xml& xml) const;
      bool readProperties(QDomNode);

      Segment* segment() const      { return (Segment*)parent(); }

      void setBeamMode(BeamMode m);
      BeamMode beamMode() const     { return _beamMode; }
      void setBeam(Beam* b)         { _beam = b; }
      Beam* beam() const            { return _beam; }
      void setTuplet(Tuplet* t)     { _tuplet = t; }
      Tuplet* tuplet() const        { return _tuplet; }
      Measure* measure() const      { return (Measure*)(parent()->parent()); }
      int beams() const;
      virtual int move()      const = 0;
      virtual qreal upPos()   const = 0;
      virtual qreal downPos() const = 0;
      virtual qreal centerX() const = 0;

      virtual void layoutStem()     {}
      virtual int upLine() const    { return 0;}
      virtual int downLine() const  { return 8;}
      virtual int line(bool up) const { return up ? upLine() : downLine(); }
      virtual QPointF stemPos(bool, bool) const { return pos(); }    // point to connect stem
      bool isUp() const             { return _up; }
      void setUp(bool val)          { _up = val; }
      pstl::plist<NoteAttribute*>* getAttributes() { return &attributes; }
      NoteAttribute* hasAttribute(const NoteAttribute*);
      virtual Element* findSelectableElement(QPointF p) const = 0;
      };

#endif



