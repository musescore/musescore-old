//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
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

#include "pdf.h"
extern "C" {
#include <fitz.h>
#include <mupdf.h>
      }

bool Pdf::init = false;
static fz_context* ctx;
static fz_glyph_cache* cache;

//---------------------------------------------------------
//   numPages
//---------------------------------------------------------

int Pdf::numPages() const
      {
      return pdf_count_pages(xref);
      }

//---------------------------------------------------------
//   Pdf
//---------------------------------------------------------

Pdf::Pdf(const QString& path)
      {
      if (!init) {
            init = true;
            ctx = fz_new_context(&fz_alloc_default, 256 << 20);
            if (!ctx) {
                  printf("Pdf::init ctx failed\n");
                  return;
                  }
            cache = fz_new_glyph_cache(ctx);
            }
      char* name = path.toAscii().data();
      xref = pdf_open_xref(ctx, name, 0);
      pdf_load_page_tree(xref);
      }

//---------------------------------------------------------
//   page
//---------------------------------------------------------

QImage Pdf::page(int i)
      {
      pdf_page* page = pdf_load_page(xref, i);
      if (page == 0) {
            printf("cannot load page %d\n", i);
            return QImage();
            }
      static const float resolution = 300.0;
      const float zoom = resolution / 72.0;

      fz_matrix ctm  = fz_translate(0, -page->mediabox.y1);
      ctm            = fz_concat(ctm, fz_scale(zoom, -zoom));
      ctm            = fz_concat(ctm, fz_rotate(page->rotate));
      fz_bbox bbox   = fz_round_rect(fz_transform_rect(ctm, page->mediabox));
      fz_pixmap* pix = fz_new_pixmap_with_rect(ctx, fz_device_gray, bbox);

      fz_clear_pixmap_with_color(pix, 255);

      fz_device* dev = fz_new_draw_device(ctx, cache, pix);
      pdf_run_page(xref, page, dev, ctm);
      fz_free_device(dev);

      int w = pix->w;
      int h = pix->h;

      QImage image(w, h, QImage::Format_Mono);
      QVector<QRgb> ct(2);
      ct[0] = qRgb(255, 255, 255);
      ct[1] = qRgb(0, 0, 0);
      image.setColorTable(ct);

      uchar* s   = pix->samples;
      int stride = image.bytesPerLine();
      int bytes  = w >> 3;
      for (int line = 0; line < h; ++line) {
            uchar* d = image.bits() + stride * line;
            for (int col = 0; col < bytes; ++col) {
                  uchar data = 0;
                  for (int i = 0; i < 8; ++i) {
                        uchar v = *s++;
                        s++;
                        data <<= 1;
                        if (v < 128)
                              data |= 1;
                        }
                  *d++ = data;
                  }
            }
      fz_drop_pixmap(ctx, pix);
      pdf_free_page(ctx, page);
      return image;
      }

