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

#ifndef __NOTE_H__
#define __NOTE_H__

/**
 \file
 Definition of classes Note and ShadowNote.
*/

#include "element.h"
#include "symbol.h"
#include "durationtype.h"

class Tie;
class Chord;
class NoteEvent;
class Text;
class Score;
class Sym;
class MuseScoreView;
class Bend;
class Painter;
class AccidentalState;
class Accidental;

extern const int noteHeads[2][HEAD_GROUPS][HEAD_TYPES];

//---------------------------------------------------------
//   NoteHead
//---------------------------------------------------------

class NoteHead : public Symbol {

   public:
      NoteHead(Score* s) : Symbol(s) {}
      NoteHead &operator=(const NoteHead&);
      virtual NoteHead* clone() const  { return new NoteHead(*this); }
      virtual ElementType type() const { return NOTEHEAD; }
      virtual void write(Xml& xml) const;
      };

//---------------------------------------------------------
//   NoteDot
//---------------------------------------------------------

class NoteDot : public Symbol {

      int _idx;

   public:
      NoteDot(Score*);
      NoteDot &operator=(const NoteHead&);
      virtual NoteDot* clone() const  { return new NoteDot(*this); }
      virtual ElementType type() const { return NOTEDOT; }
      int idx() const      { return _idx; }
      void setIdx(int val) { _idx = val; }
      };

//---------------------------------------------------------
//   Note
//---------------------------------------------------------

/**
 Graphic representation of a note.
*/

class Note : public Element {
      Q_DECLARE_TR_FUNCTIONS(Note)

      int _subchannel;        ///< articulation
      int _line;              ///< y-Position; 0 - top line.
      int _fret;              ///< for tablature view
      int _string;
      int _headGroup;
      int _tpc;               ///< tonal pitch class
      int _pitch;             ///< Note pitch as midi value (0 - 127).
      int  _ppitch;           ///< played pitch (honor voltas etc.); cached value
      bool _ghost;            ///< ghost note (guitar: death note)
      bool _hidden;           ///< markes this note as the hidden one if there are
                              ///< overlapping notes; hidden notes are not played
                              ///< and heads + accidentals are not shown
      bool _fretConflict;     ///< used by TAB staves to mark a fretting conflict:
                              ///< two or mor enotes on the same string

      bool dragMode;
      bool _mirror;           ///< True if note is mirrored at stem.
      bool _small;

      NoteHeadType _headType;

      ValueType _veloType;
      int _veloOffset;        ///< velocity user offset in percent, or absolute velocity for this note

      qreal _tuning;         ///< pitch offset in cent, playable only by internal synthesizer

      int _onTimeOffset;      ///< start note offset in ticks
      int _onTimeUserOffset;  ///< start note user offset

      int _offTimeOffset;     ///< stop note offset in ticks
      int _offTimeUserOffset; ///< stop note user offset

      DirectionH _userMirror; ///< user override of mirror
      Direction _dotPosition; ///< dot position: above or below current staff line

      Accidental* _accidental;

      ElementList _el;        ///< fingering, other text, symbols or images
      Tie* _tieFor;
      Tie* _tieBack;
      Bend* _bend;

      NoteDot* _dots[3];

      QList<NoteEvent*> _playEvents;

      int _lineOffset;        ///< Used during mouse dragging.

      virtual QRectF drag(const EditData& s);
      virtual void endDrag();
      virtual void endEdit();

   public:
      Note(Score* s = 0);
      Note(const Note&);
      Note &operator=(const Note&);
      ~Note();
      virtual Note* clone() const      { return new Note(*this); }
      virtual ElementType type() const { return NOTE; }
      virtual QPointF pagePos() const;      ///< position in page coordinates
      virtual QPointF canvasPos() const;    ///< position in page coordinates
      virtual void layout();
      void layout10(AccidentalState*);
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      virtual void setTrack(int val);

      int playTicks() const;

      qreal headWidth() const;
      qreal headHeight() const;
      int noteHead() const;
      int headGroup() const            { return _headGroup; }
      NoteHeadType headType() const    { return _headType;  }
      void setHeadGroup(int val);
      void setHeadType(NoteHeadType t) { _headType = t;     }

      int pitch() const               { return _pitch;    }
      void setPitch(int val);
      void setPitch(int a, int b);
      int ppitch() const;
      qreal tuning() const           { return _tuning;   }
      void setTuning(qreal v)        { _tuning = v;      }

      int tpc() const                 { return _tpc;      }
      void setTpc(int v);
      void setTpcFromPitch();

      Accidental* accidental() const    { return _accidental; }
      void setAccidental(Accidental* a) { _accidental = a;    }

      int line() const                { return _line + _lineOffset;   }
      void setLine(int n);

      int fret() const                { return _fret;   }
      void setFret(int val)           { _fret = val;    }
      int string() const              { return _string; }
      void setString(int val);
      bool ghost() const              { return _ghost;  }
      void setGhost(bool val)         { _ghost = val;   }
      bool fretConflict() const       { return _fretConflict; }
      void setFretConflict(bool val)  { _fretConflict = val; }

      virtual void add(Element*);
      virtual void remove(Element*);

      bool mirror() const             { return _mirror;  }
      void setMirror(bool val)        { _mirror = val;   }

      bool small() const              { return _small;   }
      void setSmall(bool val)         { _small = val;    }

      Tie* tieFor() const             { return _tieFor;  }
      Tie* tieBack() const            { return _tieBack; }
      void setTieFor(Tie* t)          { _tieFor = t;     }
      void setTieBack(Tie* t)         { _tieBack = t;    }

      Chord* chord() const            { return (Chord*)parent(); }
      void setChord(Chord* a)         { setParent((Element*)a);  }

      virtual void draw(Painter*) const;
      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      QPointF stemPos(bool upFlag) const;    ///< Point to connect stem.
      qreal stemYoff(bool upFlag) const;
      qreal yPos() const;

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);

      bool hidden() const              { return _hidden; }
      void setHidden(bool val)         { _hidden = val;  }

      NoteType noteType() const;

      ElementList* el()                { return &_el; }
      const ElementList* el() const    { return &_el; }

      int subchannel() const           { return _subchannel; }
      void setSubchannel(int val)      { _subchannel = val;  }

      DirectionH userMirror() const    { return _userMirror; }
      void setUserMirror(DirectionH d) { _userMirror = d; }

      Direction dotPosition() const    { return _dotPosition; }
      void setDotPosition(Direction d) { _dotPosition = d;    }
      bool dotIsUp() const;               // actual dot position

      virtual void toDefault();
      virtual void setMag(qreal val);

      ValueType veloType() const       { return _veloType;          }
      void setVeloType(ValueType v)    { _veloType = v;             }
      int veloOffset() const           { return _veloOffset;        }
      void setVeloOffset(int v)        { _veloOffset = v;           }

      int onTimeOffset() const         { return _onTimeOffset;      }
      void setOnTimeOffset(int v)      { _onTimeOffset = v;         }
      int onTimeUserOffset() const     { return _onTimeUserOffset;  }
      void setOnTimeUserOffset(int v)  { _onTimeUserOffset = v;     }

      int offTimeOffset() const        { return _offTimeOffset;     }
      void setOffTimeOffset(int v)     { _offTimeOffset = v;        }
      int offTimeUserOffset() const    { return _offTimeUserOffset; }
      void setOffTimeUserOffset(int v) { _offTimeUserOffset = v;    }

      Bend* bend() const               { return _bend; }
      void setBend(Bend* b)            { _bend = b;    }
      int customizeVelocity(int velo) const;
      NoteDot* dot(int n)              { return _dots[n];           }
      void updateAccidental(AccidentalState*);
      void updateLine();
      void setNval(NoteVal);
      QList<NoteEvent*>& playEvents()                { return _playEvents; }
      const QList<NoteEvent*>& playEvents() const    { return _playEvents; }
      void setPlayEvents(const QList<NoteEvent*>& v);
      virtual const QString subtypeName() const      { return QString(); }

      virtual QVariant getProperty(int propertyId) const;
      virtual void setProperty(int propertyId, const QVariant&);
      };

extern Sym* noteHeadSym(bool up, int group, int n);

#endif

