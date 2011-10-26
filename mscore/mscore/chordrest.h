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

#ifndef __CHORDREST_H__
#define __CHORDREST_H__

#include "globals.h"
#include "symbol.h"
#include "duration.h"

class Score;
class Measure;
class Beam;
class Tuplet;
class Segment;
class Slur;
class Articulation;

//---------------------------------------------------------
//   ChordRest
//    chords and rests can be part of a beam
//---------------------------------------------------------

class ChordRest : public DurationElement {
      Duration _duration;
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

   public:
      ChordRest(Score*);
      ChordRest(const ChordRest&);
      ChordRest &operator=(const ChordRest&);
      virtual ElementType type() const = 0;
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);

      Segment* segment() const                   { return (Segment*)parent(); }
      virtual Measure* measure() const           { return (Measure*)(parent()->parent()); }

      virtual void read(QDomElement, QList<Tuplet*>&, Measure*) = 0;
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement e, QList<Tuplet*>&, Measure*);
      QList<Prop> properties(Xml&, bool clipboardmode) const;
      virtual QList<Prop> properties(Xml& xml) const { return properties(xml, false); }
      virtual void scanElements(void* data, void (*func)(void*, Element*));

      void setBeamMode(BeamMode m)              { _beamMode = m; }
      BeamMode beamMode() const                 { return _beamMode; }
      void setBeam(Beam* b);
      virtual Beam* beam() const                { return _beam; }
      int beams() const                         { return duration().hooks(); }
      virtual qreal upPos()   const = 0;
      virtual qreal downPos() const = 0;
      virtual qreal centerX() const = 0;

      virtual void layoutStem1()                {}
      virtual void layoutStem()                 {}
      virtual int upLine() const                { return 0;}
      virtual int downLine() const              { return 8;}
      virtual int line(bool up) const           { return up ? upLine() : downLine(); }
      virtual QPointF stemPos(bool, bool) const { return pos(); }    // point to connect stem
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
      const QList<Slur*> slurFor() const        { return _slurFor; }
      const QList<Slur*> slurBack() const       { return _slurBack; }

      void layoutArticulations();
      Spatium extraLeadingSpace() const         { return _extraLeadingSpace;  }
      void setExtraLeadingSpace(Spatium v)      { _extraLeadingSpace = v;     }
      Spatium extraTrailingSpace() const        { return _extraTrailingSpace; }
      void setExtraTrailingSpace(Spatium v)     { _extraTrailingSpace = v;    }
      virtual void toDefault();

      const Duration& duration() const               { return _duration;        }
      void setDurationType(Duration::DurationType t);
      void setDurationType(const QString& s);
      void setDurationVal(int ticks);
      void setDuration(const Duration& v);
      void setDots(int n)                            { _duration.setDots(n); }
      int dots() const                               { return _duration.dots(); }

      virtual Fraction fraction() const;
      virtual void setFraction(const Fraction&);
      virtual void setTrack(int val);
      };

#endif

