#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2010-2011 by Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

add_library(poppler STATIC
      poppler/Annot.cc
      poppler/Array.cc
      poppler/BuiltinFont.cc
      poppler/BuiltinFontTables.cc
      poppler/CachedFile.cc
      poppler/Catalog.cc
      poppler/CharCodeToUnicode.cc
      poppler/CMap.cc
      poppler/DateInfo.cc
      poppler/Decrypt.cc
      poppler/Dict.cc
      poppler/Error.cc
      poppler/FileSpec.cc
      poppler/FontEncodingTables.cc
      poppler/Form.cc
      poppler/FontInfo.cc
      poppler/Function.cc
      poppler/Gfx.cc
      poppler/GfxFont.cc
      poppler/GfxState.cc
      poppler/GlobalParams.cc
      poppler/JArithmeticDecoder.cc
      poppler/JBIG2Stream.cc
      poppler/Lexer.cc
      poppler/Link.cc
      poppler/LocalPDFDocBuilder.cc
      poppler/Movie.cc
      poppler/NameToCharCode.cc
      poppler/Object.cc
      poppler/OptionalContent.cc
      poppler/Outline.cc
      poppler/OutputDev.cc
      poppler/Page.cc
      poppler/PageTransition.cc
      poppler/Parser.cc
      poppler/PDFDoc.cc
      poppler/PDFDocEncoding.cc
      poppler/PDFDocFactory.cc
      poppler/PopplerCache.cc
      poppler/ProfileData.cc
      poppler/PreScanOutputDev.cc
      poppler/PSTokenizer.cc
      poppler/Rendition.cc
      poppler/StdinCachedFile.cc
      poppler/StdinPDFDocBuilder.cc
      poppler/Stream.cc
      poppler/strtok_r.cpp
      poppler/UnicodeMap.cc
      poppler/UnicodeTypeTable.cc
      poppler/XRef.cc
      poppler/PSOutputDev.cc
      poppler/TextOutputDev.cc
      poppler/PageLabelInfo.h
      poppler/PageLabelInfo.cc
      poppler/SecurityHandler.cc
      poppler/Sound.cc
      poppler/XpdfPluginAPI.cc
      poppler/JPXStream.cc

      goo/FixedPoint.cc
      goo/gfile.cc
      goo/gmem.cc
      goo/gmempp.cc
      goo/GooHash.cc
      goo/GooList.cc
      goo/GooString.cc
      goo/GooTimer.cc
      goo/gstrtod.cc
      goo/ImgWriter.cc
      goo/JpegWriter.cc
      goo/PNGWriter.cc

      fofi/FoFiBase.cc
      fofi/FoFiEncodings.cc
      fofi/FoFiTrueType.cc
      fofi/FoFiType1.cc
      fofi/FoFiType1C.cc
   )
set_target_properties(poppler
      PROPERTIES
      COMPILE_FLAGS
            "-O2 -Wno-write-strings -ansi -Wnon-virtual-dtor -Woverloaded-virtual"
      )

include_directories(
      ${PROJECT_SOURCE_DIR}/poppler/goo
      )

