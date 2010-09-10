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

#include "stylehelper.h"
#include "colorscheme.h"
#include "colorutils.h"

const qreal StyleHelper::_slabThickness = 0.45;
const qreal StyleHelper::_shadowGain = 1.5;
const qreal StyleHelper::_glowBias = 0.6;

//---------------------------------------------------------
//   StyleHelper
//---------------------------------------------------------

StyleHelper::StyleHelper()
      {
      _contrast = 1.0;
      }

//---------------------------------------------------------
//   checkAutoFillBackground
//---------------------------------------------------------

const QWidget* StyleHelper::checkAutoFillBackground(const QWidget* w) const
      {
      if (!w)
            return 0;
      if (w->autoFillBackground())
            return w;
      if (w->isWindow())
            return 0;
      for (const QWidget* parent = w->parentWidget(); parent; parent = parent->parentWidget()) {
            if (parent->autoFillBackground())
                  return parent;
            if (parent == w->window())
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   backgroundColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundColor(const QColor &color, qreal ratio) const
      {
      const quint64 key((quint64(color.rgba()) << 32) | int(ratio*512));
      QColor* out = m_backgroundColorCache.object(key);
      if (!out) {
            if (ratio < 0.5) {
                  const qreal a(2.0 * ratio);
                  out = new QColor(ColorUtils::mix(backgroundTopColor(color), color, a));
                  }
            else {
                  const qreal a( 2.0*ratio-1 );
                  out = new QColor(ColorUtils::mix(color, backgroundBottomColor(color), a));
                  }
            m_backgroundColorCache.insert(key, out);
            }
      return *out;
      }

//---------------------------------------------------------
//   backgroundTopColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundTopColor(const QColor &color) const
      {
      const quint64 key( color.rgba() );
      QColor* out( m_backgroundTopColorCache.object( key ) );
      if (!out) {
            if (lowThreshold(color) )
                  out = new QColor(ColorScheme::shade(color, ColorScheme::MidlightShade, 0.0) );
            else {
                  const qreal my(ColorUtils::luma(ColorScheme::shade(color, ColorScheme::LightShade, 0.0) ) );
                  const qreal by(ColorUtils::luma(color) );
                  out = new QColor(ColorUtils::shade(color, (my - by) * _bgcontrast) );
                  }
            m_backgroundTopColorCache.insert( key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   backgroundBottomColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundBottomColor(const QColor &color) const
      {
      const quint64 key( color.rgba() );
      QColor* out( m_backgroundBottomColorCache.object( key ) );
      if( !out ) {
            const QColor midColor(ColorScheme::shade(color, ColorScheme::MidShade, 0.0) );
            if( lowThreshold(color) )
                  out = new QColor( midColor );
            else {
                  const qreal by(ColorUtils::luma(color) );
                  const qreal my(ColorUtils::luma(midColor) );
                  out = new QColor(ColorUtils::shade(color, (my - by) * _bgcontrast) );
                  }
            m_backgroundBottomColorCache.insert( key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   lowThreshold
//---------------------------------------------------------

bool StyleHelper::lowThreshold(const QColor &color) const
      {
      const quint32 key( color.rgba() );
      ColorMap::iterator iter( m_lowThreshold.find( key ) );
      if (iter != m_lowThreshold.end())
            return iter.value();
      else {
            const QColor darker(ColorScheme::shade(color, ColorScheme::MidShade, 0.5 ) );
            const bool result(ColorUtils::luma(darker) > ColorUtils::luma(color) );
            m_lowThreshold.insert(key, result);
            return result;
            }
      }

//---------------------------------------------------------
//   highThreshold
//---------------------------------------------------------

bool StyleHelper::highThreshold(const QColor &color) const
      {
      const quint32 key( color.rgba() );
      ColorMap::iterator iter( m_highThreshold.find( key ) );
      if (iter != m_highThreshold.end() )
            return iter.value();
      else {
            const QColor lighter(ColorScheme::shade(color, ColorScheme::LightShade, 0.5 ) );
            const bool result(ColorUtils::luma(lighter) < ColorUtils::luma(color) );
            m_highThreshold.insert(key, result);
            return result;
            }
      }

//---------------------------------------------------------
//   hole
//---------------------------------------------------------

TileSet* StyleHelper::hole(const QColor &color, qreal shade, int size, bool outline)
    {
      const quint64 key( (quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size << 1 | outline );
      TileSet *tileSet = m_holeCache.object(key);

      if (!tileSet) {
            const int rsize( (int)ceil(qreal(size) * 5.0/7.0 ) );
            QPixmap pixmap(rsize*2, rsize*2);
            pixmap.fill(Qt::transparent);

            QPainter p(&pixmap);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(2,2,10,10);

            // hole mask
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setBrush(Qt::black);
            p.drawEllipse(3,3,8,8);

            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            if( outline ) {
                  QLinearGradient blend( 0, 3, 0, 11 );
                  blend.setColorAt(0, Qt::transparent );
                  blend.setColorAt(1, calcDarkColor( color ) );

                  p.setBrush( Qt::NoBrush );
                  p.setPen( QPen( blend, 1 ) );
                  p.drawEllipse( 3, 3.5, 8, 7 );
                  p.setPen( Qt::NoPen );
                  }

            // shadow
            drawInverseShadow(p, calcShadowColor( color ), 3, 8, 0.0);

            p.end();

            tileSet = new TileSet(pixmap, rsize, rsize, rsize, rsize, rsize-1, rsize, 2, 1);

            m_holeCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   holeFlat
//---------------------------------------------------------

TileSet* StyleHelper::holeFlat(const QColor& color, qreal shade, int size) const
      {
      const quint64 key((quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size );
      TileSet *tileSet = m_holeFlatCache.object(key);

      if (!tileSet) {
            const int rsize((int)ceil(qreal(size) * 5.0/7.0) );
            QPixmap pixmap(rsize*2, rsize*2);
            pixmap.fill(Qt::transparent);

            QPainter p(&pixmap);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(2,2,10,10);

            // hole
            drawHole(p, color, shade, 7);

            // hole inside
            p.setBrush(color);
            p.drawEllipse(QRectF(3.4,3.4,7.2,7.2));

            p.end();

            tileSet = new TileSet(pixmap, rsize, rsize, rsize, rsize, rsize-1, rsize, 2, 1);
            m_holeFlatCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   drawHole
//---------------------------------------------------------

void StyleHelper::drawHole(QPainter &p, const QColor &color, qreal shade, int r) const
      {
      const int r2( 2*r );
      const QColor base(ColorUtils::shade(color, shade) );
      const QColor light(ColorUtils::shade(calcLightColor(color), shade) );
      const QColor dark(ColorUtils::shade(calcDarkColor(color), shade) );
      const QColor mid(ColorUtils::shade(calcMidColor(color), shade) );

      // bevel
      const qreal y(ColorUtils::luma(base) );
      const qreal yl(ColorUtils::luma(light) );
      const qreal yd(ColorUtils::luma(dark) );
      QLinearGradient bevelGradient1(0, 2, 0, r2-2);
      bevelGradient1.setColorAt(0.2, dark);
      bevelGradient1.setColorAt(0.5, mid);
      bevelGradient1.setColorAt(1.0, light);
      if (y < yl && y > yd) {
            // no middle when color is very light/dark
            bevelGradient1.setColorAt(0.6, base);
            }
      p.setBrush(bevelGradient1);
      p.drawEllipse(3,3,r2-6,r2-6);

      // mask
      QRadialGradient maskGradient(r,r,r-2);
      maskGradient.setColorAt(0.80, Qt::black );
      maskGradient.setColorAt(0.90, alphaColor( Qt::black,0.55) );
      maskGradient.setColorAt(1.00, Qt::transparent );
      p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
      p.setBrush(maskGradient);
      p.drawRect(0,0,r2,r2);
      p.setCompositionMode(QPainter::CompositionMode_SourceOver);
      }

//---------------------------------------------------------
//   renderHole
//---------------------------------------------------------

void StyleHelper::renderHole(QPainter *p, const QColor &base, const QRect &r, bool focus, bool hover, qreal opacity, AnimationMode animationMode,  TileSet::Tiles tiles, bool outline)
    {
    if( !r.isValid() )
            return;
    if( opacity >= 0 && ( animationMode & AnimationFocus ) ) {
            // calculate proper glow color based on current settings and opacity
            const QColor glow( hover ?
               ColorUtils::mix( viewHoverBrush().brush(QPalette::Active).color(), viewFocusBrush().brush(QPalette::Active).color(), opacity ):
               alphaColor(  viewFocusBrush().brush(QPalette::Active).color(), opacity ) );

            holeFocused(base, glow, 0.0, 7, outline)->render(r, p, tiles);
            }
      else if (focus) {
            holeFocused(base, viewFocusBrush().brush(QPalette::Active).color(), 0.0)->render(r, p, tiles);
            }
      else if( opacity >= 0 && ( animationMode & AnimationHover ) ) {
            // calculate proper glow color based on current settings and opacity
            const QColor glow( alphaColor(  viewHoverBrush().brush(QPalette::Active).color(), opacity ) );
            holeFocused(base, glow, 0.0, 7, outline)->render(r, p, tiles);
            }
      else if (hover) {
            holeFocused(base, viewHoverBrush().brush(QPalette::Active).color(), 0.0)->render(r, p, tiles);
            }
      else {
            hole(base, 0.0, 7, outline)->render(r, p, tiles);
            }
      }

//---------------------------------------------------------
//   holeFocused
//---------------------------------------------------------

TileSet* StyleHelper::holeFocused(const QColor &color, const QColor &glowColor, qreal shade, int size, bool outline)
      {
      // FIXME must move to s/slabcache/cache/ b/c key is wrong
      Cache<TileSet>::Value* cache( m_holeFocusedCache.get(glowColor) );

      const quint64 key( (quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size << 1 | outline );
      TileSet *tileSet = cache->object(key);

      if (!tileSet) {
            const int rsize( (int)ceil(qreal(size) * 5.0/7.0) );
            QPixmap pixmap(rsize*2, rsize*2);
            pixmap.fill(Qt::transparent);

            QPainter p(&pixmap);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);

            TileSet *holeTileSet = hole(color, shade, size, outline);

            // hole
            holeTileSet->render(QRect(0,0,10,10), &p);

            p.setWindow(2,2,10,10);

            // glow
            drawInverseGlow(p, glowColor, 3, 8, size);

            p.end();

            tileSet = new TileSet(pixmap, rsize, rsize, rsize, rsize, rsize-1, rsize, 2, 1);

            cache->insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   calcLightColor
//---------------------------------------------------------

const QColor& StyleHelper::calcLightColor(const QColor &color) const
      {
      const quint64 key( color.rgba() );
      QColor* out( m_lightColorCache.object( key ) );
      if( !out ) {
            out = new QColor( highThreshold(color) ? color: ColorScheme::shade(color, ColorScheme::LightShade, _contrast) );
            m_lightColorCache.insert(key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   calcDarkColor
//---------------------------------------------------------

const QColor& StyleHelper::calcDarkColor(const QColor &color) const
      {
      const quint64 key( color.rgba() );
      QColor* out( m_darkColorCache.object( key ) );
      if ( !out ) {
            out = new QColor( (lowThreshold(color)) ?
            ColorUtils::mix(calcLightColor(color), color, 0.3 + 0.7 * _contrast):
            ColorScheme::shade(color, ColorScheme::MidShade, _contrast) );
            m_darkColorCache.insert(key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   alphaColor
//---------------------------------------------------------

QColor StyleHelper::alphaColor(QColor color, qreal alpha)
      {
      if (alpha >= 0 && alpha < 1.0) {
            color.setAlphaF(alpha*color.alphaF());
            }
      return color;
      }

//---------------------------------------------------------
//   drawInverseShadow
//---------------------------------------------------------

void StyleHelper::drawInverseShadow(
        QPainter &p, const QColor &color,
        int pad, int size, qreal fuzz )
    {

              const qreal m( qreal(size)*0.5 );
              const QColor shadow( calcShadowColor( color ) );
              const qreal offset( 0.8 );
              const qreal k0( (m-2) / qreal(m+2.0) );
              QRadialGradient shadowGradient(pad+m, pad+m+offset, m+2);
              for (int i = 0; i < 8; i++)
                    {
                        // sinusoidal gradient
                        qreal k1 = (qreal(8 - i) + k0 * qreal(i)) * 0.125;
                        qreal a = (cos(3.14159 * i * 0.125) + 1.0) * 0.25;
                        shadowGradient.setColorAt(k1, alphaColor(shadow, a * _shadowGain));
                    }
              shadowGradient.setColorAt(k0, alphaColor(color, 0.0));
              p.setBrush(shadowGradient);
              p.drawEllipse(QRectF(pad-fuzz, pad-fuzz, size+fuzz*2.0, size+fuzz*2.0));
          }


//---------------------------------------------------------
//   drawInverseGlow
//---------------------------------------------------------

void StyleHelper::drawInverseGlow(
        QPainter &p, const QColor &color,
        int pad, int size, int rsize) const
    {

              const QRectF r(pad, pad, size, size);
              const qreal m( qreal(size)*0.5 );

              const qreal width( 3.5 );
              const qreal bias( _glowBias*7.0/rsize );
              const qreal k0( (m-width)/(m-bias) );
              QRadialGradient glowGradient(pad+m, pad+m, m-bias);
              for (int i = 0; i < 8; i++)
                    {
                        // inverse parabolic gradient
                        qreal k1 = (k0 * qreal(i) + qreal(8 - i)) * 0.125;
                        qreal a = 1.0 - sqrt(i * 0.125);
                        glowGradient.setColorAt(k1, alphaColor(color, a));

                    }

              glowGradient.setColorAt(k0, alphaColor(color, 0.0));
              p.setBrush(glowGradient);
              p.drawEllipse(r);
          }

//---------------------------------------------------------
//   calcShadowColor
//---------------------------------------------------------

const QColor& StyleHelper::calcShadowColor(const QColor &color)
      {
      const quint64 key( color.rgba() );
      QColor* out( m_shadowColorCache.object( key ) );
      if( !out ) {
            out = new QColor( (lowThreshold(color)) ?
            ColorUtils::mix( Qt::black, color, color.alphaF() ) :
            ColorScheme::shade(ColorUtils::mix( Qt::black, color, color.alphaF() ),
               ColorScheme::ShadowShade, _contrast) );
            m_shadowColorCache.insert(key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   renderMenuBackground
//---------------------------------------------------------

void StyleHelper::renderMenuBackground(QPainter* p, const QRect& clipRect, const QWidget* widget, const QColor& color)
      {
      // get coordinates relative to the client area
      // this is stupid. One could use mapTo if this was taking const QWidget* and not
      // QWidget* as argument.
      const QWidget* w( widget );
      int x(0);
      int y(0);

      while( !w->isWindow() && w != w->parentWidget() ) {
            x += w->geometry().x();
            y += w->geometry().y();
            w = w->parentWidget();
            }

      if (clipRect.isValid()) {
            p->save();
            p->setClipRegion(clipRect,Qt::IntersectClip);
            }

      // calculate upper part height
      // special tricks are needed
      // to handle both window contents and window decoration
      QRect r = w->rect();
      const int height( w->frameGeometry().height() );
      const int splitY( qMin(200, (3*height)/4) );

      const QRect upperRect( QRect(0, 0, r.width(), splitY) );
      const QPixmap tile( verticalGradient(color, splitY) );
      p->drawTiledPixmap(upperRect, tile);

      const QRect lowerRect( 0,splitY, r.width(), r.height() - splitY );
      p->fillRect(lowerRect, backgroundBottomColor(color));

      if (clipRect.isValid()) {
            p->restore();
            }
      }

//---------------------------------------------------------
//   verticalGradient
//---------------------------------------------------------

QPixmap StyleHelper::verticalGradient(const QColor &color, int height, int offset)
      {
      const quint64 key( (quint64(color.rgba()) << 32) | height | 0x8000 );
      QPixmap *pixmap( m_backgroundCache.object( key ) );

      if (!pixmap) {
            pixmap = new QPixmap(1, height);
            pixmap->fill( Qt::transparent );

            QLinearGradient gradient(0, offset, 0, height+offset);
            gradient.setColorAt(0.0, backgroundTopColor(color));
            gradient.setColorAt(0.5, color);
            gradient.setColorAt(1.0, backgroundBottomColor(color));

            QPainter p(pixmap);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(pixmap->rect(), gradient);

            p.end();

            m_backgroundCache.insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   radialGradient
//---------------------------------------------------------

QPixmap StyleHelper::radialGradient(const QColor &color, int width, int height)
      {
      const quint64 key( ( quint64(color.rgba()) << 32) | width | 0xb000 );
      QPixmap *pixmap( m_backgroundCache.object( key ) );

      if (!pixmap) {
            pixmap = new QPixmap(width, height);
            pixmap->fill(Qt::transparent);
            QColor radialColor = backgroundRadialColor(color);
            radialColor.setAlpha(255);
            QRadialGradient gradient(64, height-64, 64);
            gradient.setColorAt(0, radialColor);
            radialColor.setAlpha(101);
            gradient.setColorAt(0.5, radialColor);
            radialColor.setAlpha(37);
            gradient.setColorAt(0.75, radialColor);
            radialColor.setAlpha(0);
            gradient.setColorAt(1, radialColor);
            QPainter p(pixmap);
            p.scale(width/128.0,1);
            p.fillRect(QRect(0,0,128,height), gradient);
            p.end();
            m_backgroundCache.insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   backgroundRadialColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundRadialColor(const QColor &color)
      {
      const quint64 key( color.rgba() );
      QColor* out( m_backgroundRadialColorCache.object( key ) );
      if (!out) {
            if( lowThreshold(color) )
                  out = new QColor(ColorScheme::shade(color, ColorScheme::LightShade, 0.0) );
            else if( highThreshold(color))
                  out = new QColor(color);
            else out = new QColor(ColorScheme::shade(color, ColorScheme::LightShade, _bgcontrast) );
                  m_backgroundRadialColorCache.insert( key, out );
            }
      return *out;
      }


