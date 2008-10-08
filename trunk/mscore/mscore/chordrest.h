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

#ifndef __CHORDREST_H__
#define __CHORDREST_H__

#include "globals.h"
#include "symbol.h"
#include "durationtype.h"

class Score;
class ScoreLayout;
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

class ChordRest : public Element {
      QList<Slur*> _slurFor;
      QList<Slur*> _slurBack;
      Duration _duration;

   protected:
      QList<Articulation*> articulations;
      Beam* _beam;
      BeamMode _beamMode;
      Tuplet* _tuplet;
      bool _up;
      bool _small;

      int _dots;

   public:
      ChordRest(Score*);
      ChordRest(const ChordRest&);
      ChordRest &operator=(const ChordRest&);
      virtual ElementType type() const = 0;
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual Element* drop(const QPointF&, const QPointF&, Element*);

      void writeProperties(Xml& xml, bool clipboardmode=false) const;
      bool readProperties(QDomElement);
      QList<Prop> properties(Xml&, bool clipboardmode) const;
      virtual QList<Prop> properties(Xml& xml) const { return properties(xml, false); }

      Segment* segment() const                  { return (Segment*)parent(); }
      Measure* measure() const                  { return (Measure*)(parent()->parent()); }

      void setBeamMode(BeamMode m)              { _beamMode = m; }
      BeamMode beamMode() const                 { return _beamMode; }
      void setBeam(Beam* b)                     { _beam = b; }
      Beam* beam() const                        { return _beam; }
      void setTuplet(Tuplet* t)                 { _tuplet = t; }
      Tuplet* tuplet() const                    { return _tuplet; }
      int beams() const                         { return _duration.hooks(); }
      virtual qreal upPos()   const = 0;
      virtual qreal downPos() const = 0;
      virtual qreal centerX() const = 0;

      virtual void layoutStem1(ScoreLayout*)    {}
      virtual void layoutStem(ScoreLayout*)     {}
      virtual int upLine() const                { return 0;}
      virtual int downLine() const              { return 8;}
      virtual int line(bool up) const           { return up ? upLine() : downLine(); }
      virtual QPointF stemPos(bool, bool) const { return pos(); }    // point to connect stem
      bool up() const                           { return _up;   }
      bool isUp() const;
      void setUp(bool val)                      { _up = val; }
      QList<Articulation*>* getArticulations()    { return &articulations; }
      Articulation* hasArticulation(const Articulation*);
      bool small() const                        { return _small; }
      void setSmall(bool val);
      virtual int staffMove() const = 0;

      void addSlurFor(Slur*);
      void addSlurBack(Slur*);
      void removeSlurFor(Slur*);
      void removeSlurBack(Slur*);
      const QList<Slur*> slurFor() const        { return _slurFor; }
      const QList<Slur*> slurBack() const       { return _slurBack; }

      Duration duration() const                 { return _duration; }
      virtual void setDuration(Duration t)      { _duration = t; }

      void setDots(int n)                       { _dots = n; }
      int dots() const                          { return _dots; }
      void setLen(int ticks);
      void layoutAttributes(ScoreLayout*);
      };

#endif

