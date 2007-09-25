//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: symbols.cpp,v 1.34 2006/04/06 13:03:11 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "globals.h"
#include "style.h"
#include "sym.h"
#include "spatium.h"
#include "utils.h"
#include "mscore.h"
#include "score.h"

QVector<Sym> symbols(lastSym);

static const double FMAG = 1.0;

QPrinter* printer = 0;
static double printerDpi;

#ifdef __MINGW32__
#define CTABLE_HACK
#endif

// #define CTABLE_HACK

#ifdef CTABLE_HACK
      struct CTable {
            int code;
            int font;
            double x, y, w, h;
            double width;
            };

      CTable ctable[] = {
            {    56, 2, 8.000000, -91.000000, 51.000000, 92.000000, 67.000000 },
            {    49, 2, 15.000000, -91.000000, 35.000000, 91.000000, 67.000000 },
            {    53, 2, 7.000000, -89.000000, 50.000000, 90.000000, 67.000000 },
            { 61583, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61583, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61584, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            {    43, 0, -1.000000, -126.000000, 84.000000, 84.000000, 83.000000 },
            {    48, 0, 0.000000, -168.000000, 122.000000, 168.000000, 122.000000 },
            {    49, 0, -1.000000, -169.000000, 109.000000, 169.000000, 106.000000 },
            {    50, 0, -1.000000, -167.000000, 124.000000, 168.000000, 122.000000 },
            {    51, 0, 0.000000, -167.000000, 112.000000, 167.000000, 110.000000 },
            {    52, 0, -1.000000, -169.000000, 135.000000, 170.000000, 133.000000 },
            {    53, 0, -1.000000, -168.000000, 118.000000, 169.000000, 112.000000 },
            {    54, 0, -1.000000, -167.000000, 114.000000, 167.000000, 113.000000 },
            {    55, 0, 0.000000, -172.000000, 123.000000, 173.000000, 122.000000 },
            {    56, 0, -3.000000, -167.000000, 131.000000, 168.000000, 122.000000 },
            {    57, 0, 0.000000, -167.000000, 114.000000, 172.000000, 113.000000 },
            {   102, 0, -34.000000, -158.000000, 179.000000, 217.000000, 106.000000 },
            {   109, 0, -15.000000, -102.000000, 168.000000, 108.000000, 146.000000 },
            {   112, 0, -45.000000, -98.000000, 164.000000, 147.000000, 121.000000 },
            {   114, 0, -11.000000, -107.000000, 107.000000, 107.000000, 72.000000 },
            {   115, 0, 6.000000, -98.000000, 75.000000, 98.000000, 68.000000 },
            {   122, 0, 0.000000, -89.000000, 96.000000, 100.000000, 95.000000 },
            { 57600, 0, 0.000000, 0.000000, 126.000000, 52.000000, 124.000000 },
            { 57601, 0, 0.000000, -52.000000, 126.000000, 52.000000, 124.000000 },
            { 57602, 0, -52.000000, -9.000000, 230.000000, 62.000000, 124.000000 },
            { 57603, 0, -52.000000, -53.000000, 230.000000, 62.000000, 124.000000 },
            { 57604, 0, 0.000000, -84.000000, 150.000000, 168.000000, 150.000000 },
            { 57605, 0, 0.000000, -84.000000, 50.000000, 168.000000, 50.000000 },
            { 57606, 0, 0.000000, -83.000000, 50.000000, 83.000000, 50.000000 },
            { 57607, 0, -9.000000, -133.000000, 91.000000, 241.000000, 78.000000 },
            { 57608, 0, 0.000000, -68.000000, 84.000000, 155.000000, 83.000000 },
            { 57609, 0, -1.000000, -70.000000, 84.000000, 157.000000, 83.000000 },
            { 57610, 0, -8.000000, -70.000000, 108.000000, 242.000000, 100.000000 },
            { 57611, 0, -19.000000, -153.000000, 127.000000, 324.000000, 108.000000 },
            { 57612, 0, -22.000000, -153.000000, 139.000000, 408.000000, 116.000000 },
            { 57613, 0, -20.000000, -237.000000, 145.000000, 492.000000, 124.000000 },
            { 57614, 0, 0.000000, -127.000000, 92.000000, 255.000000, 91.000000 },
            { 57617, 0, -3.000000, -129.000000, 61.000000, 258.000000, 55.000000 },
            { 57618, 0, -10.000000, -156.000000, 78.000000, 212.000000, 66.000000 },
            { 57620, 0, -10.000000, -156.000000, 133.000000, 212.000000, 120.000000 },
            { 57622, 0, -3.000000, -46.000000, 90.000000, 93.000000, 83.000000 },
            { 57623, 0, 0.000000, -89.000000, 39.000000, 178.000000, 39.000000 },
            { 57624, 0, 0.000000, -89.000000, 39.000000, 178.000000, 39.000000 },
            { 57625, 0, 0.000000, -20.000000, 38.000000, 39.000000, 37.000000 },
            { 57626, 0, -13.000000, -47.000000, 191.000000, 96.000000, 164.000000 },
            { 57627, 0, 0.000000, -47.000000, 165.000000, 95.000000, 164.000000 },
            { 57628, 0, -1.000000, -46.000000, 117.000000, 93.000000, 115.000000 },
            { 57629, 0, 0.000000, -47.000000, 109.000000, 94.000000, 109.000000 },
            { 57630, 0, 0.000000, -47.000000, 166.000000, 93.000000, 164.000000 },
            { 57631, 0, -1.000000, -47.000000, 123.000000, 93.000000, 121.000000 },
            { 57632, 0, 0.000000, -47.000000, 123.000000, 93.000000, 122.000000 },
            { 57633, 0, -2.000000, -63.000000, 196.000000, 137.000000, 192.000000 },
            { 57634, 0, -2.000000, -56.000000, 143.000000, 123.000000, 139.000000 },
            { 57636, 0, -1.000000, -57.000000, 119.000000, 124.000000, 116.000000 },
            { 57638, 0, 0.000000, -89.000000, 252.000000, 177.000000, 251.000000 },
            { 57639, 0, 0.000000, -89.000000, 196.000000, 177.000000, 195.000000 },
            { 57640, 0, 0.000000, -89.000000, 144.000000, 177.000000, 142.000000 },
            { 57641, 0, 0.000000, -51.000000, 144.000000, 102.000000, 142.000000 },
            { 57642, 0, 0.000000, -48.000000, 127.000000, 97.000000, 126.000000 },
            { 57643, 0, 0.000000, -48.000000, 110.000000, 95.000000, 109.000000 },
            { 57644, 0, -1.000000, -58.000000, 131.000000, 116.000000, 130.000000 },
            { 57672, 0, -111.000000, -122.000000, 222.000000, 133.000000, 110.000000 },
            { 57673, 0, -111.000000, -9.000000, 222.000000, 131.000000, 110.000000 },
            { 57680, 0, -34.000000, -41.000000, 67.000000, 109.000000, 33.000000 },
            { 57681, 0, -77.000000, -42.000000, 153.000000, 85.000000, 74.000000 },
            { 57682, 0, -160.000000, -43.000000, 320.000000, 86.000000, 158.000000 },
            { 57683, 0, -19.000000, -19.000000, 36.000000, 38.000000, 16.000000 },
            { 57684, 0, -19.000000, -86.000000, 38.000000, 90.000000, 16.000000 },
            { 57685, 0, -19.000000, -4.000000, 36.000000, 92.000000, 16.000000 },
            { 57686, 0, -51.000000, -6.000000, 101.000000, 13.000000, 50.000000 },
            { 57687, 0, -51.000000, -71.000000, 102.000000, 77.000000, 50.000000 },
            { 57688, 0, -51.000000, -6.000000, 102.000000, 78.000000, 50.000000 },
            { 57689, 0, -41.000000, -93.000000, 83.000000, 93.000000, 41.000000 },
            { 57690, 0, -41.000000, 0.000000, 83.000000, 93.000000, 41.000000 },
            { 57691, 0, -34.000000, -42.000000, 68.000000, 83.000000, 33.000000 },
            { 57692, 0, -46.000000, -48.000000, 93.000000, 94.000000, 46.000000 },
            { 57693, 0, -55.000000, -174.000000, 109.000000, 174.000000, 54.000000 },
            { 57694, 0, -63.000000, -111.000000, 126.000000, 111.000000, 62.000000 },
            { 57695, 0, -91.000000, -47.000000, 181.000000, 94.000000, 90.000000 },
            { 57696, 0, -91.000000, -46.000000, 182.000000, 92.000000, 90.000000 },
            { 57697, 0, -108.000000, -180.000000, 201.000000, 185.000000, 70.000000 },
            { 57698, 0, -42.000000, -57.000000, 83.000000, 100.000000, 41.000000 },
            { 57699, 0, -42.000000, -43.000000, 83.000000, 101.000000, 41.000000 },
            { 57700, 0, -42.000000, -127.000000, 84.000000, 128.000000, 41.000000 },
            { 57701, 0, -42.000000, 0.000000, 84.000000, 127.000000, 41.000000 },
            { 57702, 0, -44.000000, -44.000000, 89.000000, 89.000000, 44.000000 },
            { 57703, 0, -84.000000, -126.000000, 168.000000, 253.000000, 83.000000 },
            { 57704, 0, -90.000000, -119.000000, 180.000000, 237.000000, 84.000000 },
            { 57705, 0, -90.000000, -119.000000, 180.000000, 238.000000, 84.000000 },
            { 57706, 0, -2.000000, -52.000000, 39.000000, 104.000000, 41.000000 },
            { 57707, 0, -42.000000, -51.000000, 43.000000, 103.000000, 0.000000 },
            { 57710, 0, -2.000000, -103.000000, 70.000000, 122.000000, 66.000000 },
            { 57711, 0, -20.000000, -69.000000, 122.000000, 71.000000, 83.000000 },
            { 57712, 0, -20.000000, -104.000000, 107.000000, 104.000000, 66.000000 },
            { 57713, 0, -20.000000, -84.000000, 107.000000, 105.000000, 66.000000 },
            { 57714, 0, -40.000000, -43.000000, 80.000000, 85.000000, 34.000000 },
            { 57715, 0, -84.000000, -43.000000, 168.000000, 85.000000, 69.000000 },
            { 57716, 0, -85.000000, -56.000000, 169.000000, 112.000000, 69.000000 },
            { 57717, 0, -119.000000, -43.000000, 238.000000, 85.000000, 104.000000 },
            { 57718, 0, -119.000000, -56.000000, 238.000000, 112.000000, 104.000000 },
            { 57719, 0, -131.000000, -42.000000, 249.000000, 144.000000, 104.000000 },
            { 57720, 0, -131.000000, -57.000000, 250.000000, 159.000000, 104.000000 },
            { 57721, 0, -119.000000, -43.000000, 249.000000, 145.000000, 104.000000 },
            { 57722, 0, -135.000000, -80.000000, 254.000000, 122.000000, 104.000000 },
            { 57723, 0, -135.000000, -80.000000, 254.000000, 137.000000, 104.000000 },
            { 57724, 0, -119.000000, -79.000000, 253.000000, 121.000000, 104.000000 },
            { 57725, 0, -119.000000, -164.000000, 238.000000, 207.000000, 104.000000 },
            { 57727, 0, -11.000000, 0.000000, 83.000000, 252.000000, 74.000000 },
            { 57728, 0, -11.000000, 0.000000, 81.000000, 294.000000, 74.000000 },
            { 57729, 0, -11.000000, 0.000000, 81.000000, 360.000000, 69.000000 },
            { 57730, 0, -11.000000, 0.000000, 83.000000, 443.000000, 69.000000 },
            { 57731, 0, -11.000000, -243.000000, 101.000000, 243.000000, 89.000000 },
            { 57732, 0, -59.000000, 73.000000, 140.000000, 118.000000, 69.000000 },
            { 57733, 0, -74.000000, -181.000000, 172.000000, 106.000000, 89.000000 },
            { 57734, 0, -11.000000, -252.000000, 102.000000, 252.000000, 89.000000 },
            { 57735, 0, -11.000000, -327.000000, 101.000000, 327.000000, 89.000000 },
            { 57736, 0, -11.000000, -369.000000, 101.000000, 369.000000, 89.000000 },
            { 57737, 0, 0.000000, -169.000000, 227.000000, 337.000000, 226.000000 },
            { 57738, 0, 0.000000, -133.000000, 183.000000, 267.000000, 182.000000 },
            { 57739, 0, -4.000000, -89.000000, 224.000000, 261.000000, 223.000000 },
            { 57740, 0, -4.000000, -71.000000, 180.000000, 209.000000, 178.000000 },
            { 57741, 0, -2.000000, -420.000000, 220.000000, 639.000000, 213.000000 },
            { 57742, 0, 0.000000, -335.000000, 174.000000, 513.000000, 170.000000 },
            { 57743, 0, 56.000000, -84.000000, 110.000000, 168.000000, 167.000000 },
            { 57744, 0, 45.000000, -67.000000, 88.000000, 134.000000, 133.000000 },
            { 57745, 0, 13.000000, -243.000000, 221.000000, 485.000000, 233.000000 },
            { 57746, 0, 11.000000, -194.000000, 176.000000, 390.000000, 186.000000 },
            { 57747, 0, 0.000000, -89.000000, 145.000000, 178.000000, 141.000000 },
            { 57748, 0, 0.000000, -118.000000, 145.000000, 235.000000, 141.000000 },
            { 57749, 0, 0.000000, -170.000000, 130.000000, 133.000000, 129.000000 },
            { 57750, 0, 0.000000, -81.000000, 70.000000, 30.000000, 70.000000 },
            { 57751, 0, 0.000000, -24.000000, 24.000000, 24.000000, 23.000000 },
            { 57752, 0, 0.000000, -167.000000, 140.000000, 167.000000, 138.000000 },
            { 57753, 0, -2.000000, -146.000000, 114.000000, 151.000000, 110.000000 },
            { 57754, 0, -2.000000, -97.000000, 69.000000, 98.000000, 66.000000 },
            { 57755, 0, 0.000000, -167.000000, 267.000000, 169.000000, 265.000000 },
            { 57756, 0, 0.000000, -111.000000, 157.000000, 130.000000, 158.000000 },
            { 57757, 0, 0.000000, -18.000000, 156.000000, 131.000000, 158.000000 },
            { 57758, 0, -132.000000, -260.000000, 262.000000, 263.000000, 130.000000 },
            { 57759, 0, -24.000000, -23.000000, 47.000000, 48.000000, 20.000000 },
            { 57760, 0, -90.000000, -175.000000, 178.000000, 175.000000, 88.000000 },
            { 57761, 0, -174.000000, -343.000000, 349.000000, 343.000000, 172.000000 },
            { 57762, 0, -89.000000, -262.000000, 177.000000, 262.000000, 88.000000 },
            { 61609, 0, 44.000000, -240.000000, 208.000000, 180.000000, 297.000000 },
            { 61610, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 57763, 0, -89.000000, -176.000000, 177.000000, 176.000000, 88.000000 },
            { 61612, 0, 44.000000, -241.000000, 208.000000, 241.000000, 297.000000 },
            { 61613, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61614, 0, 28.000000, -240.000000, 240.000000, 240.000000, 297.000000 },
            { 61615, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61616, 0, 28.000000, -240.000000, 240.000000, 240.000000, 297.000000 },
            { 61617, 0, 28.000000, -240.000000, 240.000000, 240.000000, 297.000000 },
            { 61618, 0, 28.000000, -242.000000, 240.000000, 242.000000, 297.000000 },
            { 61619, 0, 58.000000, -211.000000, 182.000000, 181.000000, 297.000000 },
            { 61620, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61621, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61622, 0, 22.000000, -241.000000, 253.000000, 218.000000, 297.000000 },
            { 61623, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61624, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61625, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61626, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61627, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61628, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61629, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61630, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61631, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 61632, 0, 28.000000, -241.000000, 241.000000, 241.000000, 297.000000 },
            { 57629, 1, 0.000000, -33.000000, 76.000000, 66.000000, 76.000000 },
            { 57628, 1, 0.000000, -31.000000, 82.000000, 65.000000, 80.000000 },
            { 57627, 1, 0.000000, -33.000000, 115.000000, 66.000000, 115.000000 },
            { 57626, 1, -9.000000, -33.000000, 134.000000, 67.000000, 115.000000 },
            { 57625, 1, 0.000000, -14.000000, 27.000000, 27.000000, 26.000000 },
            { 57727, 1, -8.000000, 0.000000, 59.000000, 176.000000, 52.000000 },
            { 57728, 1, -8.000000, 0.000000, 57.000000, 205.000000, 52.000000 },
            { 57729, 1, -8.000000, 0.000000, 57.000000, 251.000000, 48.000000 },
            { 57730, 1, -8.000000, 0.000000, 58.000000, 309.000000, 48.000000 },
            { 57731, 1, -8.000000, -169.000000, 71.000000, 169.000000, 62.000000 },
            { 57734, 1, -8.000000, -175.000000, 71.000000, 175.000000, 62.000000 },
            { 57735, 1, -8.000000, -228.000000, 71.000000, 228.000000, 62.000000 },
            { 57736, 1, -8.000000, -257.000000, 71.000000, 257.000000, 62.000000 },
            { 57614, 1, 0.000000, -89.000000, 64.000000, 178.000000, 64.000000 },
            { 57617, 1, -2.000000, -90.000000, 43.000000, 180.000000, 38.000000 },
            { 57618, 1, -7.000000, -109.000000, 54.000000, 148.000000, 46.000000 },
            { 57620, 1, -7.000000, -109.000000, 93.000000, 148.000000, 84.000000 },
            { 57622, 1, -3.000000, -33.000000, 63.000000, 66.000000, 58.000000 },
            { 57597, 0, 0.000000, -332.000000, 109.000000, 467.000000, 109.000000 },
            { 57630, 1, 0.000000, -33.000000, 116.000000, 65.000000, 115.000000 },
            { 57631, 1, -2.000000, -33.000000, 86.000000, 65.000000, 85.000000 },
            { 57632, 1, 0.000000, -33.000000, 86.000000, 65.000000, 86.000000 },
            { 57633, 1, -1.000000, -44.000000, 137.000000, 95.000000, 134.000000 },
            { 57634, 1, -1.000000, -39.000000, 100.000000, 86.000000, 97.000000 },
            { 57636, 1, -1.000000, -40.000000, 84.000000, 87.000000, 81.000000 },
            { 57638, 1, 0.000000, -62.000000, 177.000000, 123.000000, 175.000000 },
            { 57639, 1, 0.000000, -62.000000, 137.000000, 123.000000, 136.000000 },
            { 57640, 1, 0.000000, -62.000000, 101.000000, 124.000000, 99.000000 },
            { 57641, 1, -1.000000, -36.000000, 101.000000, 72.000000, 99.000000 },
            { 57642, 1, -1.000000, -34.000000, 90.000000, 68.000000, 88.000000 },
            { 57643, 1, 0.000000, -32.000000, 77.000000, 65.000000, 76.000000 },
            { 57599, 0, -1.000000, -337.000000, 118.000000, 384.000000, 115.000000 },
            { 57596, 0, 0.000000, -334.000000, 109.000000, 381.000000, 109.000000 },
            { 57592, 0, 0.000000, -334.000000, 181.000000, 381.000000, 179.000000 },
            { 57593, 0, 0.000000, -335.000000, 178.000000, 382.000000, 179.000000 },
            { 57594, 0, 0.000000, -335.000000, 180.000000, 382.000000, 179.000000 },
            { 57595, 0, -1.000000, -349.000000, 181.000000, 444.000000, 179.000000 },
            { 57597, 0, 0.000000, -332.000000, 109.000000, 467.000000, 109.000000 },

            };
#endif

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const char* name, const QChar& c, int f)
   : _code(c), fontId(f), _name(name)
      {
      QFont font;
      if (fontId == 0) {
            font = QFont("MScore");
            font.setPointSizeF(20.0 * FMAG);
            }
      else if (fontId == 1) {
            font = QFont("MScore");
            font.setPointSizeF(14.0 * FMAG);
            }
      else {
            font = QFont("Times New Roman");
            font.setPointSizeF(8.0 * FMAG);
            }
#ifdef CTABLE_HACK
      unsigned i;
      for (i = 0; i < sizeof(ctable)/sizeof(*ctable); ++i) {
            if ((c.unicode() == ctable[i].code) && (f == ctable[i].font)) {
                  _bbox = QRectF(
                        ctable[i].x,
                        ctable[i].y,
                        ctable[i].w,
                        ctable[i].h);
                  _width = ctable[i].width;
                  break;
                  }
            }
      if (i == sizeof(ctable)/sizeof(*ctable)) {
printf("sym not found\n");
            _bbox   = QFontMetricsF(font, printer).boundingRect(_code);
            _width  = QFontMetricsF(font, printer).width(_code);
            }
#else
      _bbox   = QFontMetricsF(font, printer).boundingRect(_code);
      _width  = QFontMetricsF(font, printer).width(_code);
#if 0
      printf("            { %5d, %d, %f, %f, %f, %f, %f },\n",
            c.unicode(), f, _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
            _width);
#endif

#endif
      }

//---------------------------------------------------------
//   findSymbol
//---------------------------------------------------------

const Sym* findSymbol(QChar code, int fontId)
      {
      foreach(const Sym& s, symbols) {
            if (s.code() == code && s.getFontId() == fontId)
                  return &s;
            }
      return 0;
      }

//---------------------------------------------------------
//   bbox
//    BUG:
//      For unknown reasons i cannot get the right font
//      metrics from Qt.
//      The problem is that the floating point metrics
//      of Qt are the same as the integer metrics (rounded
//      down to screen resolution). This makes no sense.
//      Painting with float accuracy does not work well.
//
//      Qt4.3 delivers slightly different font metrics than
//      Qt4.2.2
//---------------------------------------------------------

const QRectF Sym::bbox() const
      {
      double m = _spatium / (spatiumBase20 * FMAG * printerDpi);
      return QRectF(_bbox.topLeft() * m, _bbox.size() * m);
      }

double Sym::width() const
      {
      double m = _spatium / (spatiumBase20 * FMAG * printerDpi);
      return _width * m;
      }

double Sym::height() const
      {
      double m = _spatium / (spatiumBase20 * FMAG * printerDpi);
      return _bbox.height() * m;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, qreal x, qreal y) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y), QString(_code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, qreal x, qreal y, int n) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y), QString(n, _code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(0,0), QString(_code));
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Sym::font(double extraMag) const
      {
      double mag = _spatium * extraMag / (spatiumBase20 * DPI);
      if (fontId == 0) {
            QFont f("MScore");
            f.setPointSizeF(20.0 * mag);
            return f;
            }
      else if (fontId == 1) {
            QFont f("MScore");
            f.setPointSizeF(14.0 * mag);
            return f;
            }
      else {
            QFont f("Times New Roman");
            f.setPointSizeF(8.0 * mag);
            return f;
            }
      }

//---------------------------------------------------------
//   initSymbols
//---------------------------------------------------------

void initSymbols()
      {
      if (printer == 0) {
            printer = new QPrinter(QPrinter::HighResolution);
#ifdef CTABLE_HACK
            printerDpi = 1200.0;
#else
            printerDpi = double(printer->logicalDpiX());
#endif
            }

      symbols[clefEightSym] = Sym("clef eight", 0x38, 2);
      symbols[clefOneSym]   = Sym("clef one",   0x31, 2);
      symbols[clefFiveSym]  = Sym("clef five",  0x35, 2);

      symbols[stemSym]      = Sym("stem",  0xf08f, 0);
      symbols[stemSym]      = Sym("stem",  0xf08f, 0);
      symbols[dstemSym]     = Sym("dstem", 0xf090, 0);

      symbols[plusSym]                    = Sym("plus",               0x2b, 0);

      symbols[zeroSym]                    = Sym("zero",               0x30, 0);
      symbols[oneSym]                     = Sym("one",                0x31, 0);
      symbols[twoSym]                     = Sym("two",                0x32, 0);
      symbols[threeSym]                   = Sym("three",              0x33, 0);
      symbols[fourSym]                    = Sym("four",               0x34, 0);
      symbols[fiveSym]                    = Sym("five",               0x35, 0);
      symbols[sixSym]                     = Sym("six",                0x36, 0);
      symbols[sevenSym]                   = Sym("seven",              0x37, 0);
      symbols[eightSym]                   = Sym("eight",              0x38, 0);
      symbols[nineSym]                    = Sym("nine",               0x39, 0);

      symbols[letterfSym]                 = Sym("f",                  0x66, 0);
      symbols[lettermSym]                 = Sym("m",                  0x6d, 0);
      symbols[letterpSym]                 = Sym("p",                  0x70, 0);
      symbols[letterrSym]                 = Sym("r",                  0x72, 0);
      symbols[lettersSym]                 = Sym("s",                  0x73, 0);
      symbols[letterzSym]                 = Sym("z",                  0x7a, 0);

//      symbols[wholerestSym]               = Sym("whole rest",         0xe100, 0, QPointF(.0,-1.) );
      symbols[wholerestSym]               = Sym("whole rest",         0xe100, 0);

      symbols[halfrestSym]                = Sym("half rest",          0xe101, 0);
      symbols[outsidewholerestSym]        = Sym("outside whole rest", 0xe102, 0);
      symbols[outsidehalfrestSym]         = Sym("outside half rest",  0xe103, 0);
      symbols[rest_M3]                    = Sym("rest M3",            0xe104, 0);
      symbols[longarestSym]               = Sym("longa rest",         0xe105, 0);
      symbols[breverestSym]               = Sym("breve rest",         0xe106, 0);
      symbols[quartrestSym]               = Sym("quart rest",         0xe107, 0);
      symbols[clasquartrestSym]           = Sym("clas quart rest",    0xe108, 0);
      symbols[eighthrestSym]              = Sym("eight rest",         0xe109, 0);
      symbols[sixteenthrestSym]           = Sym("16' rest",           0xe10a, 0);
      symbols[thirtysecondrestSym]        = Sym("32'  rest",          0xe10b, 0);
      symbols[sixtyfourthrestSym]         = Sym("64' rest",           0xe10c, 0);
      symbols[hundredtwentyeighthrestSym] = Sym("128' rest",          0xe10d, 0);

      symbols[sharpSym]                   = Sym("sharp",                    0xe10e, 0);
      symbols[naturalSym]                 = Sym("natural",                  0xe111, 0);
      symbols[flatSym]                    = Sym("flat",                     0xe112, 0);
      symbols[flatflatSym]                = Sym("flat flat",                0xe114, 0);
      symbols[sharpsharpSym]              = Sym("sharp sharp",              0xe116, 0);
      symbols[rightparenSym]              = Sym("right parenthesis",        0xe117, 0);
      symbols[leftparenSym]               = Sym("left parenthesis",         0xe118, 0);
      symbols[dotSym]                     = Sym("dot",                      0xe119, 0);
      symbols[brevisheadSym]              = Sym("brevis head",              0xe11a, 0);
      symbols[wholeheadSym]               = Sym("whole head",               0xe11b, 0);
      symbols[halfheadSym]                = Sym("half head",                0xe11c, 0);
      symbols[quartheadSym]               = Sym("quart head",               0xe11d, 0);

      symbols[wholediamondheadSym]        = Sym("whole diamond head",       0xe11e, 0);
      symbols[halfdiamondheadSym]         = Sym("half diamond head",        0xe11f, 0);
      symbols[diamondheadSym]             = Sym("diamond head",             0xe120, 0);
      symbols[wholetriangleheadSym]       = Sym("whole triangle head",      0xe121, 0);
      symbols[halftriangleheadSym]        = Sym("half triangle head",       0xe122, 0);
      symbols[triangleheadSym]            = Sym("triangle head",            0xe124, 0);

      symbols[wholeslashheadSym]          = Sym("whole slash head",         0xe126, 0);
      symbols[halfslashheadSym]           = Sym("half slash head",          0xe127, 0);
      symbols[quartslashheadSym]          = Sym("quart slash head",         0xe128, 0);

      symbols[wholecrossedheadSym]        = Sym("whole cross head",         0xe129, 0);
      symbols[halfcrossedheadSym]         = Sym("half cross head",          0xe12a, 0);
      symbols[crossedheadSym]             = Sym("cross head",               0xe12b, 0);
      symbols[xcircledheadSym]            = Sym("x circle head",            0xe12c, 0);

      symbols[ufermataSym]                = Sym("ufermata",                 0xe148, 0);
      symbols[dfermataSym]                = Sym("dfermata",                 0xe149, 0);
      symbols[thumbSym]                   = Sym("thumb",                    0xe150, 0);
      symbols[sforzatoaccentSym]          = Sym("sforza to accent",         0xe151, 0);
      symbols[esprSym]                    = Sym("espressivo",               0xe152, 0);
      symbols[staccatoSym]                = Sym("staccato",                 0xe153, 0);
      symbols[ustaccatissimoSym]          = Sym("ustaccatissimo",           0xe154, 0);
      symbols[dstaccatissimoSym]          = Sym("dstacattissimo",           0xe155, 0);

      symbols[tenutoSym]                  = Sym("tenuto",                   0xe156, 0);
      symbols[uportatoSym]                = Sym("uportato",                 0xe157, 0);
      symbols[dportatoSym]                = Sym("dportato",                 0xe158, 0);
      symbols[umarcatoSym]                = Sym("umarcato",                 0xe159, 0);
      symbols[dmarcatoSym]                = Sym("dmarcato",                 0xe15a, 0);
      symbols[ouvertSym]                  = Sym("ouvert",                   0xe15b, 0);
      symbols[plusstopSym]                = Sym("plus stop",                0xe15c, 0);
      symbols[upbowSym]                   = Sym("up bow",                   0xe15d, 0);
      symbols[downbowSym]                 = Sym("down bow",                 0xe15e, 0);
      symbols[reverseturnSym]             = Sym("reverse turn",             0xe15f, 0);
      symbols[turnSym]                    = Sym("turn",                     0xe160, 0);
      symbols[trillSym]                   = Sym("trill",                    0xe161, 0);
      symbols[upedalheelSym]              = Sym("upedal heel",              0xe162, 0);
      symbols[dpedalheelSym]              = Sym("dpedalheel",               0xe163, 0);
      symbols[upedaltoeSym]               = Sym("upedal toe",               0xe164, 0);
      symbols[dpedaltoeSym]               = Sym("dpedal toe",               0xe165, 0);

      symbols[flageoletSym]               = Sym("flageolet",                0xe166, 0);
      symbols[segnoSym]                   = Sym("segno",                    0xe167, 0);
      symbols[codaSym]                    = Sym("coda",                     0xe168, 0);
      symbols[varcodaSym]                 = Sym("varcoda",                  0xe169, 0);

      symbols[rcommaSym]                  = Sym("rcomma",                   0xe16a, 0);
      symbols[lcommaSym]                  = Sym("lcomma",                   0xe16b, 0);
      symbols[arpeggioSym]                = Sym("arpeggio",                 0xe16e, 0);
      symbols[trillelementSym]            = Sym("trillelement",             0xe16f, 0);
      symbols[arpeggioarrowdownSym]       = Sym("arpeggio arrow down",      0xe170, 0);
      symbols[arpeggioarrowupSym]         = Sym("arpeggio arrow up",        0xe171, 0);
      symbols[trilelementSym]             = Sym("trill element",            0xe172, 0);
      symbols[prallSym]                   = Sym("prall",                    0xe173, 0);
      symbols[mordentSym]                 = Sym("mordent",                  0xe174, 0);
      symbols[prallprallSym]              = Sym("prall prall",              0xe175, 0);
      symbols[prallmordentSym]            = Sym("prall mordent",            0xe176, 0);
      symbols[upprallSym]                 = Sym("up prall",                 0xe177, 0);
      symbols[upmordentSym]               = Sym("up mordent",               0xe178, 0);
      symbols[pralldownSym]               = Sym("prall down",               0xe179, 0);
      symbols[downprallSym]               = Sym("down prall",               0xe17a, 0);
      symbols[downmordentSym]             = Sym("down mordent",             0xe17b, 0);
      symbols[prallupSym]                 = Sym("prall up",                 0xe17c, 0);
      symbols[lineprallSym]               = Sym("line prall",               0xe17d, 0);
      symbols[eighthflagSym]              = Sym("eight flag",               0xe17f, 0);
      symbols[sixteenthflagSym]           = Sym("sixteenth flag",           0xe180, 0);
      symbols[thirtysecondflagSym]        = Sym("thirtysecond flag",        0xe181, 0);
      symbols[sixtyfourthflagSym]         = Sym("sixtyfour flag",           0xe182, 0);
      symbols[deighthflagSym]             = Sym("deight flag",              0xe183, 0);
      symbols[gracedashSym]               = Sym("grace dash",               0xe184, 0);
      symbols[dgracedashSym]              = Sym("dgrace dash",              0xe185, 0);
      symbols[dsixteenthflagSym]          = Sym("dsixteenth flag",          0xe186, 0);
      symbols[dthirtysecondflagSym]       = Sym("dthirtysecond flag",       0xe187, 0);
      symbols[dsixtyfourthflagSym]        = Sym("dsixtyfourth flag",        0xe188, 0);
      symbols[altoclefSym]                = Sym("alto clef",                0xe189, 0);

      symbols[caltoclefSym]               = Sym("calto clef",               0xe18a, 0);
      symbols[bassclefSym]                = Sym("bass clef",                0xe18b, 0);
      symbols[cbassclefSym]               = Sym("cbass clef",               0xe18c, 0);
      symbols[trebleclefSym]              = Sym("trebleclef",               0xe18d, 0);   //G-Clef
      symbols[ctrebleclefSym]             = Sym("ctrebleclef",              0xe18e, 0);
      symbols[percussionclefSym]          = Sym("percussion clef",          0xe18f, 0);
      symbols[cpercussionclefSym]         = Sym("cpercussion clef",         0xe190, 0);
      symbols[tabclefSym]                 = Sym("tab clef",                 0xe191, 0);
      symbols[ctabclefSym]                = Sym("ctab clef",                0xe192, 0);

      symbols[fourfourmeterSym]           = Sym("four four meter",          0xe193, 0);
      symbols[allabreveSym]               = Sym("allabreve",                0xe194, 0);
      symbols[pedalasteriskSym]           = Sym("pedalasterisk",            0xe195, 0);
      symbols[pedaldashSym]               = Sym("pedaldash",                0xe196, 0);
      symbols[pedaldotSym]                = Sym("pedaldot",                 0xe197, 0);

      symbols[pedalPSym]                  = Sym("pedalP",                   0xe198, 0);
      symbols[pedaldSym]                  = Sym("pedald",                   0xe199, 0);
      symbols[pedaleSym]                  = Sym("pedale",                   0xe19a, 0);
      symbols[pedalPedSym]                = Sym("pedal ped",                0xe19b, 0);

      symbols[brackettipsUp]              = Sym("bracket ticks up",         0xe19c, 0);
      symbols[brackettipsDown]            = Sym("bracket ticks down",       0xe19d, 0);

      symbols[accDiscantSym]              = Sym("acc discant",              0xe19e, 0);
      symbols[accDotSym]                  = Sym("acc dot",                  0xe19f, 0);
      symbols[accFreebaseSym]             = Sym("acc freebase",             0xe1a0, 0);
      symbols[accStdbaseSym]              = Sym("acc stdbase",              0xe1a1, 0);
      symbols[accBayanbaseSym]            = Sym("acc bayanbase",            0xe1a2, 0);
      symbols[accSBSym]                   = Sym("acc sb",                   0xf0a9, 0);
      symbols[accBBSym]                   = Sym("acc bb",                   0xf0aa, 0);
      symbols[accOldEESym]                = Sym("acc old ee",               0xe1a3, 0);
      symbols[accOldEESSym]               = Sym("acc old ees",              0xf0ac, 0);
      symbols[wholedoheadSym]             = Sym("whole do head",            0xf0ad, 0);
      symbols[halfdoheadSym]              = Sym("half do head",             0xf0ae, 0);
      symbols[doheadSym]                  = Sym("do head",                  0xf0af, 0);

      symbols[wholereheadSym]             = Sym("whole re head",             0xf0b0, 0);
      symbols[halfreheadSym]              = Sym("half re head",              0xf0b1, 0);
      symbols[reheadSym]                  = Sym("re head",                   0xf0b2, 0);
      symbols[wholemeheadSym]             = Sym("whole me head",             0xf0b3, 0);
      symbols[halfmeheadSym]              = Sym("half me head",              0xf0b4, 0);
      symbols[meheadSym]                  = Sym("me head",                   0xf0b5, 0);
      symbols[wholefaheadSym]             = Sym("whole fa head",             0xf0b6, 0);
      symbols[halffauheadSym]             = Sym("half fau head",             0xf0b7, 0);
      symbols[fauheadSym]                 = Sym("fau head",                  0xf0b8, 0);
      symbols[halffadheadSym]             = Sym("half fad head",             0xf0b9, 0);
      symbols[fadheadSym]                 = Sym("fad head",                  0xf0ba, 0);
      symbols[wholelaheadSym]             = Sym("whole la head",             0xf0bb, 0);
      symbols[halflaheadSym]              = Sym("half la head",              0xf0bc, 0);
      symbols[laheadSym]                  = Sym("la head",                   0xf0bd, 0);
      symbols[wholeteheadSym]             = Sym("whole te head",             0xf0be, 0);
      symbols[halfteheadSym]              = Sym("half te head",              0xf0bf, 0);
      symbols[teheadSym]                  = Sym("te head",                   0xf0c0, 0);

      symbols[s_quartheadSym]             = Sym("small quart head",          0xe11d, 1);
      symbols[s_halfheadSym]              = Sym("small half head",           0xe11c, 1);
      symbols[s_wholeheadSym]             = Sym("small whole head",          0xe11b, 1);
      symbols[s_brevisheadSym]            = Sym("small brevis head",         0xe11a, 1);
      symbols[s_dotSym]                   = Sym("small dot",                 0xe119, 1);

      symbols[s_eighthflagSym]            = Sym("small eight flag",          0xe17f, 1);
      symbols[s_sixteenthflagSym]         = Sym("small sixteenth flag",      0xe180, 1);
      symbols[s_thirtysecondflagSym]      = Sym("small thirtysecond flag",   0xe181, 1);
      symbols[s_sixtyfourthflagSym]       = Sym("small sixtyfourth flag",    0xe182, 1);
      symbols[s_deighthflagSym]           = Sym("small deight flag",         0xe183, 1);
      symbols[s_dsixteenthflagSym]        = Sym("small d sixteenth flag",    0xe186, 1);
      symbols[s_dthirtysecondflagSym]     = Sym("small d thirtysecond flag", 0xe187, 1);
      symbols[s_dsixtyfourthflagSym]      = Sym("small d sixtyfourth flag",  0xe188, 1);

      symbols[s_sharpSym]                 = Sym("small sharp",               0xe10e, 1);
      symbols[s_naturalSym]               = Sym("small natural",             0xe111, 1);
      symbols[s_flatSym]                  = Sym("small flat",                0xe112, 1);
      symbols[s_flatflatSym]              = Sym("small flat flat",           0xe114, 1);
      symbols[s_sharpsharpSym]            = Sym("small sharp sharp",         0xe116, 1);
      symbols[flipSym]                    = Sym("flip stem",  0xe0fd, 0);

      symbols[s_wholediamondheadSym]        = Sym("whole diamond head",       0xe11e, 1);
      symbols[s_halfdiamondheadSym]         = Sym("half diamond head",        0xe11f, 1);
      symbols[s_diamondheadSym]             = Sym("diamond head",             0xe120, 1);

      symbols[s_wholetriangleheadSym]       = Sym("whole triangle head",      0xe121, 1);
      symbols[s_halftriangleheadSym]        = Sym("half triangle head",       0xe122, 1);
      symbols[s_triangleheadSym]            = Sym("triangle head",            0xe124, 1);

      symbols[s_wholeslashheadSym]          = Sym("whole slash head",         0xe126, 1);
      symbols[s_halfslashheadSym]           = Sym("half slash head",          0xe127, 1);
      symbols[s_quartslashheadSym]          = Sym("quart slash head",         0xe128, 1);

      symbols[s_wholecrossedheadSym]        = Sym("whole cross head",         0xe129, 1);
      symbols[s_halfcrossedheadSym]         = Sym("half cross head",          0xe12a, 1);
      symbols[s_crossedheadSym]             = Sym("cross head",               0xe12b, 1);

      // used for GUI:
      symbols[note2Sym]                   = Sym("note 1/2",   0xe0ff, 0);
      symbols[note4Sym]                   = Sym("note 1/4",   0xe0fc, 0);
      symbols[note8Sym]                   = Sym("note 1/8",   0xe0f8, 0);
      symbols[note16Sym]                  = Sym("note 1/16",  0xe0f9, 0);
      symbols[note32Sym]                  = Sym("note 1/32",  0xe0fa, 0);
      symbols[note64Sym]                  = Sym("note 1/64",  0xe0fb, 0);
      symbols[dotdotSym]                  = Sym("dot dot",    0xe0fd, 0);
      }

