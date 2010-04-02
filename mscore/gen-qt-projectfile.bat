@echo off

set OLD_DIR=%CD%

echo TRANSLATIONS = \
echo       %1/share/locale/mscore_af.ts \
echo       %1/share/locale/mscore_ar.ts \
echo       %1/share/locale/mscore_bg.ts \
echo       %1/share/locale/mscore_ca.ts \
echo       %1/share/locale/mscore_cs.ts \
echo       %1/share/locale/mscore_da.ts \
echo       %1/share/locale/mscore_de.ts \
echo       %1/share/locale/mscore_el.ts \
echo       %1/share/locale/mscore_en_GB.ts \
echo       %1/share/locale/mscore_en_US.ts \
echo       %1/share/locale/mscore_es.ts \
echo       %1/share/locale/mscore_fi.ts \
echo       %1/share/locale/mscore_fr.ts \
echo       %1/share/locale/mscore_gl.ts \
echo       %1/share/locale/mscore_hi_IN.ts \
echo       %1/share/locale/mscore_hu.ts \
echo       %1/share/locale/mscore_id.ts \
echo       %1/share/locale/mscore_it.ts \
echo       %1/share/locale/mscore_ja.ts \
echo       %1/share/locale/mscore_nb.ts \
echo       %1/share/locale/mscore_nl.ts \
echo       %1/share/locale/mscore_pl.ts \
echo       %1/share/locale/mscore_pt_BR.ts \
echo       %1/share/locale/mscore_pt.ts \
echo       %1/share/locale/mscore_ro.ts \
echo       %1/share/locale/mscore_ru.ts \
echo       %1/share/locale/mscore_sv.ts \
echo       %1/share/locale/mscore_th.ts \
echo       %1/share/locale/mscore_tr.ts \
echo       %1/share/locale/mscore_uk.ts \
echo       %1/share/locale/mscore_zh_Hant.ts \
echo       %1/share/locale/mscore_zh_TW.ts \
REM == Qt missing files from r27026 http://www.virtualbox.org/svn/vbox/trunk/src/VBox/Frontends/VirtualBox/nls/==
echo       %1/share/locale/qt_ar.ts \
echo       %1/share/locale/qt_bg.ts \
echo       %1/share/locale/qt_ca.ts \
echo       %1/share/locale/qt_cs.ts \
echo       %1/share/locale/qt_da.ts \
echo       %1/share/locale/qt_de.ts \
echo       %1/share/locale/qt_el.ts \
echo       %1/share/locale/qt_es.ts \
echo       %1/share/locale/qt_fi.ts \
echo       %1/share/locale/qt_fr.ts \
echo       %1/share/locale/qt_gl.ts \
echo       %1/share/locale/qt_hu.ts \
echo       %1/share/locale/qt_id.ts \
echo       %1/share/locale/qt_it.ts \
echo       %1/share/locale/qt_ja.ts \
echo       %1/share/locale/qt_nl.ts \
echo       %1/share/locale/qt_pl.ts \
echo       %1/share/locale/qt_pt.ts \
echo       %1/share/locale/qt_pt_BR.ts \
echo       %1/share/locale/qt_ro.ts \
echo       %1/share/locale/qt_ru.ts \
echo       %1/share/locale/qt_sv.ts \
echo       %1/share/locale/qt_tr.ts \
echo       %1/share/locale/qt_uk.ts \
echo       %1/share/locale/qt_zh_TW.ts

cd /d %1

echo FORMS = \
for /r %1 %%a in (*.ui) do echo     %%a \
echo.

echo SOURCES = \
for /r %1 %%a in (*.cpp) do echo     %%a \
echo.
echo.

cd /d %OLD_DIR%
