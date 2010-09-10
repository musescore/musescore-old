//=============================================================================
//  MuseScore
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

#ifndef __HELPER_H__
#define __HELPER_H__

#include "tileset.h"
#include "colorscheme.h"

enum AnimationMode {
      AnimationNone = 0,
      AnimationHover = 1<<0,
      AnimationFocus = 1<<1,
      AnimationEnable = 1<<2
      };

Q_DECLARE_FLAGS(AnimationModes, AnimationMode)

//---------------------------------------------------------
//   BaseCache
//---------------------------------------------------------

template<typename T> class BaseCache: public QCache<quint64, T> {
      bool _enabled;

   public:
      //! constructor
      BaseCache(int maxCost) : QCache<quint64, T>(maxCost), _enabled(true) {}
      explicit BaseCache() : _enabled(true) { }
      ~BaseCache() {}

      //! enable
      void setEnabled(bool value) { _enabled = value; }

      //! enable state
      bool enabled() const { return _enabled; }

      //! access
      T* object(const quint64& key) { return _enabled ? QCache<quint64, T>::object(key) : 0; }

      //! max cost
      void setMaxCost( int cost ) {
            if( cost <= 0 ) {
                  QCache<quint64, T>::clear();
                  QCache<quint64, T>::setMaxCost(1);
                  setEnabled( false );
                  }
            else {
                  setEnabled(true);
                  QCache<quint64, T>::setMaxCost(cost);
                  }
            }
      };

//---------------------------------------------------------
//   Cache
//---------------------------------------------------------

template<typename T> class Cache {
   public:
      Cache() {}
      ~Cache() {}

      //! return cache matching a given key
      //typedef QCache<quint64, T> Value;
      typedef BaseCache<T> Value;

      Value* get(const QColor& color) {
            quint64 key = (quint64(color.rgba()) << 32);
            Value *cache = data_.object(key);
            if (!cache) {
                  cache = new Value( data_.maxCost() );
                  data_.insert(key, cache);
                  }
            return cache;
            }

      void clear() { data_.clear(); }

      //! max cache size
      void setMaxCacheSize(int value) {
            data_.setMaxCost(value);
            foreach(quint64 key, data_.keys()) {
                  data_.object(key)->setMaxCost(value);
                  }
            }
   private:
      BaseCache<Value> data_;
      };


typedef BaseCache<QColor> ColorCache;
typedef BaseCache<TileSet> TileSetCache;
typedef BaseCache<QPixmap> PixmapCache;

typedef QMap<quint32, bool> ColorMap;

//---------------------------------------------------------
//   StyleHelper
//---------------------------------------------------------

class StyleHelper {
      mutable ColorCache m_midColorCache;
      mutable ColorCache m_backgroundColorCache;
      mutable ColorCache m_backgroundTopColorCache;
      mutable ColorCache m_backgroundBottomColorCache;
      mutable ColorCache m_decoColorCache;
      mutable ColorCache m_lightColorCache;
      mutable ColorCache m_darkColorCache;
      mutable ColorCache m_shadowColorCache;
      mutable ColorCache m_backgroundRadialColorCache;


      mutable TileSetCache m_cornerCache;
      mutable TileSetCache m_slabSunkenCache;
      mutable TileSetCache m_slabInvertedCache;
      mutable TileSetCache m_holeCache;
      mutable TileSetCache m_holeFlatCache;
      mutable TileSetCache m_slopeCache;
      mutable TileSetCache m_grooveCache;
      mutable TileSetCache m_slitCache;
      mutable TileSetCache m_dockFrameCache;
      mutable TileSetCache m_scrollHoleCache;
      mutable TileSetCache m_selectionCache;

      Cache<QPixmap> m_dialSlabCache;
      Cache<QPixmap> m_roundSlabCache;
      Cache<TileSet> m_holeFocusedCache;
      PixmapCache m_backgroundCache;
      PixmapCache m_dotCache;

      qreal _bgcontrast;
      qreal _contrast;
      static const qreal _shadowGain;
      static const qreal _glowBias;
      static const qreal _slabThickness;

      StatefulBrush _viewHoverBrush;
      StatefulBrush _viewFocusBrush;
      mutable ColorMap m_highThreshold;
      mutable ColorMap m_lowThreshold;

      void drawHole(QPainter&, const QColor&, qreal shade, int r = 7) const;

   public:
      StyleHelper();
      StatefulBrush viewHoverBrush() const { return _viewHoverBrush; }
      StatefulBrush viewFocusBrush() const { return _viewFocusBrush; }

      TileSet* hole(const QColor &color, qreal shade, int size, bool outline);
      TileSet* holeFlat(const QColor&, qreal shade, int size = 7) const;
      //! generic hole
      void renderHole(QPainter *p, const QColor& color, const QRect &r, bool focus=false, bool hover=false,
         TileSet::Tiles posFlags = TileSet::Ring, bool outline = false) {
            renderHole(p, color, r, focus, hover, -1, AnimationNone, posFlags, outline);
            }
      //! generic hole (with animated glow)
      void renderHole(QPainter *p, const QColor&, const QRect &r,
         bool focus, bool hover,
         qreal opacity, AnimationMode animationMode,
         TileSet::Tiles posFlags = TileSet::Ring, bool outline = false);
      TileSet* holeFocused(const QColor&, const QColor &glowColor, qreal shade, int size=7, bool outline=false);

      inline const QColor& calcMidColor(const QColor& color) const;
      const QWidget* checkAutoFillBackground( const QWidget* ) const;

      //! returns menu background color matching position in a top level widget of given height
      const QColor& backgroundColor(const QColor &color, int height, int y) const {
            return backgroundColor(color, qMin(qreal(1.0), qreal(y)/qMin(300, 3*height/4)));
            }
      const QColor& backgroundColor(const QColor&, qreal ratio) const;

      const QColor& backgroundColor(const QColor &color, const QWidget* w, const QPoint& point) const {
            if(!(w && w->window()) || checkAutoFillBackground(w))
                  return color;
            else
                  return backgroundColor(color, w->window()->height(), w->mapTo(w->window(), point).y());
            }

      const QColor& backgroundRadialColor(const QColor &color);
      const QColor& backgroundTopColor(const QColor &color) const;
      const QColor& backgroundBottomColor(const QColor &color) const;

      bool lowThreshold(const QColor &color) const;
      bool highThreshold(const QColor &color) const;
      static QColor alphaColor(QColor color, qreal alpha);
      const QColor& calcLightColor(const QColor &color) const;
      const QColor& calcDarkColor(const QColor &color) const;
      void drawInverseShadow(QPainter&, const QColor&, int pad, int size, qreal fuzz);
      void drawInverseGlow(QPainter&, const QColor&, int pad, int size, int rsize) const;
      const QColor& calcShadowColor(const QColor &color);
      void renderMenuBackground(QPainter* p, const QRect& clipRect, const QWidget* widget, const QPalette& pal) {
            renderMenuBackground(p, clipRect, widget, pal.color(widget->window()->backgroundRole()));
            }
      // render menu background
      void renderMenuBackground(QPainter*, const QRect&, const QWidget*, const QColor&);
      QPixmap verticalGradient(const QColor &color, int height, int offset = 0);
      QPixmap radialGradient(const QColor &color, int width, int height = 64);
      inline bool hasAlphaChannel(const QWidget*) const;
      bool compositingActive() const { return false; } // return KWindowSystem::compositingActive();
      };

//---------------------------------------------------------
//   calcMidColor
//---------------------------------------------------------

const QColor& StyleHelper::calcMidColor(const QColor& color) const
      {
      const quint64 key(color.rgba());
      QColor* out = m_midColorCache.object(key);
      if (!out) {
            out = new QColor(ColorScheme::shade(color, ColorScheme::MidShade, _contrast - 1.0));
            m_midColorCache.insert(key, out);
            }
      return *out;
      }

bool StyleHelper::hasAlphaChannel( const QWidget* widget ) const
      {
#ifdef Q_WS_X11
      if (compositingActive()) {
            if (widget)
                  return widget->x11Info().depth() == 32;
            else
                  return QX11Info().appDepth() == 32;
            }
      else
            return false;
#else
      return compositingActive();
#endif
      }



#endif



