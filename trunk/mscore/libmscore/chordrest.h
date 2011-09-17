//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CHORDREST_H__
#define __CHORDREST_H__

#include "symbol.h"
#include "duration.h"

class Score;
class Measure;
class Beam;
class Tuplet;
class Segment;
class Slur;
class Articulation;
class Lyrics;
class TabDurationSymbol;

//---------------------------------------------------------
//   ChordRest
//    chords and rests can be part of a beam
//---------------------------------------------------------

class ChordRest : public DurationElement {
      Duration _durationType;
      int _staffMove;         // -1, 0, +1, used for crossbeaming

      QList<Slur*> _slurFor;
      QList<Slur*> _slurBack;

   protected:
      QList<Articulation*> articulations;
      Beam* _beam;
      BeamMode _beamMode;
      bool _up;
      bool _small;
      Spatium _extraLeadingSpace;
      Spatium _extraTrailingSpace;
      Space _space;
      QList<Lyrics*> _lyricsList;
      TabDurationSymbol * _tabDur;        // stores a duration symbol in tablature staves

   public:
      ChordRest(Score*);
      ChordRest(const ChordRest&);
      ChordRest &operator=(const ChordRest&);
      ~ChordRest();
      virtual ElementType type() const = 0;
      virtual QPointF pagePos() const;      ///< position in page coordinates
      virtual QPointF canvasPos() const;
      virtual Element* drop(const DropData&);

      Segment* segment() const                   { return (Segment*)parent(); }
      virtual Measure* measure() const           { return (Measure*)(parent()->parent()); }

      virtual void read(QDomElement, const QList<Tuplet*>&, QList<Slur*>*) = 0;
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement e, const QList<Tuplet*>&, QList<Slur*>*);
      QList<Prop> properties(Xml&, bool clipboardmode) const;
      virtual QList<Prop> properties(Xml& xml) const { return properties(xml, false); }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      void setBeamMode(BeamMode m)              { _beamMode = m; }
      BeamMode beamMode() const                 { return _beamMode; }
      void setBeam(Beam* b);
      virtual Beam* beam() const                { return _beam; }
      int beams() const                         { return _durationType.hooks(); }
      virtual qreal upPos()   const = 0;
      virtual qreal downPos() const = 0;
      virtual qreal centerX() const = 0;

      virtual void layoutStem1()                {}
      virtual void layoutStem()                 {}
      virtual int upLine() const                { return 0;}
      virtual int downLine() const              { return 8;}
      virtual int line(bool up) const           { return up ? upLine() : downLine(); }
      virtual QPointF stemPos(bool, bool) const { return pagePos(); }    // point to connect stem
      bool up() const                           { return _up;   }
      void setUp(bool val)                      { _up = val; }
      QList<Articulation*>* getArticulations()  { return &articulations; }
      Articulation* hasArticulation(const Articulation*);
      bool small() const                        { return _small; }
      void setSmall(bool val);

      int staffMove() const                     { return _staffMove; }
      void setStaffMove(int val)                { _staffMove = val; }

      void addSlurFor(Slur*);
      void addSlurBack(Slur*);
      void removeSlurFor(Slur*);
      void removeSlurBack(Slur*);
      const QList<Slur*>& slurFor() const       { return _slurFor;  }
      const QList<Slur*>& slurBack() const      { return _slurBack; }

      void setSlurFor(const QList<Slur*>& s)    { _slurFor = s;  }
      void setSlurBack(const QList<Slur*>& s)   { _slurBack = s;  }

      void layoutArticulations();
      Spatium extraLeadingSpace() const         { return _extraLeadingSpace;  }
      void setExtraLeadingSpace(Spatium v)      { _extraLeadingSpace = v;     }
      Spatium extraTrailingSpace() const        { return _extraTrailingSpace; }
      void setExtraTrailingSpace(Spatium v)     { _extraTrailingSpace = v;    }
      virtual void toDefault();

      const Duration& durationType() const      { return _durationType;        }
      void setDurationType(Duration::DurationType t);
      void setDurationType(const QString& s);
      void setDurationType(int ticks);
      void setDurationType(const Duration& v);
      void setDots(int n)                       { _durationType.setDots(n); }
      int dots() const                          { return _durationType.dots(); }

      virtual void setTrack(int val);
      virtual int tick() const;
      virtual Space space() const               { return _space; }

      const QList<Lyrics*>& lyricsList() const { return _lyricsList; }
      QList<Lyrics*>& lyricsList()             { return _lyricsList; }
      Lyrics* lyrics(int no)                   { return _lyricsList.value(no); }
      virtual void add(Element*);
      virtual void remove(Element*);
      void removeDeleteBeam();
      virtual const QString subtypeName() const { return QString(); }
      };

#endif

