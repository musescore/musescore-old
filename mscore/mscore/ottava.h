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

#ifndef __OTTAVA_H__
#define __OTTAVA_H__

#include "textline.h"

//---------------------------------------------------------
//   OttavaE
//---------------------------------------------------------

struct OttavaE {
      int offset;
      unsigned start;
      unsigned end;
      };

class Ottava;

//---------------------------------------------------------
//   OttavaSegment
//---------------------------------------------------------

class OttavaSegment : public TextLineSegment {
   protected:

   public:
      OttavaSegment(Score* s) : TextLineSegment(s) {}
      virtual ElementType type() const     { return OTTAVA_SEGMENT; }
      virtual OttavaSegment* clone() const { return new OttavaSegment(*this); }
      Ottava* ottava() const               { return (Ottava*)parent(); }
      };

//---------------------------------------------------------
//   Ottava
//    brackets
//---------------------------------------------------------

class Ottava : public TextLine {

   protected:
      QString text;
      int _pitchShift;
      mutable qreal textHeight;     ///< cached value

      friend class OttavaSegment;

   public:
      Ottava(Score* s);
      virtual Ottava* clone() const    { return new Ottava(*this); }
      virtual ElementType type() const { return OTTAVA; }
      virtual void setSubtype(int val);
      virtual LineSegment* createLineSegment();
      virtual void layout();
      int pitchShift() const { return _pitchShift; }
      };

#endif

