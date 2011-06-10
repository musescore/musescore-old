//=============================================================================
//  MusE
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __AL_COLOR_H__
#define __AL_COLOR_H__

//---------------------------------------------------------
//   Color
//---------------------------------------------------------

class Color {
      union {
            struct { unsigned char _r, _g, _b, _a;  };
            int _rgbaVal;
            };
   public:
      Color()                  {_r = _g = _b = 0; _a = 255; }
      Color(int r, int g, int b, int a = 255) : _r(r), _g(g), _b(b), _a(a) {}
      void setRed(int val)     { _r = val; }
      void setGreen(int val)   { _g = val; }
      void setBlue(int val)    { _b = val; }
      void setRgb(int r, int g, int b, int a=255) {
            _r = r, _g = g, _b = b, _a = a;
            }
      bool operator!=(const Color& c) const {
            return _rgbaVal != c._rgbaVal;
            }
      bool operator==(const Color& c) const {
            return _rgbaVal == c._rgbaVal;
            }
      int r() const          { return _r; }
      int g() const          { return _g; }
      int b() const          { return _b; }
      int a() const          { return _a; }
      void setAlpha(int val) { _a = val; }
      };

#endif

