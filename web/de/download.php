<?php
  $file="download.php";
  require("header.html");
  ?>
<div id="intro">
  <h2> Downloads</h2>

  <h3> Sources</h3>
    Die <i>MuseScore</i> Quellen gibt es bei
   <a href="http://sourceforge.net/project/showfiles.php?group_id=109430">SourceForge</a>.
  <br>

  <h3> Binaries</h3>
  Für Ubuntu/Kubuntu 7.10 gibt es fertige Binärpakete unter dem Namen
  "mscore" und "mscore-common" in der "universe" Paketquelle.
  <br>
  Die
  <a href="http://sourceforge.net/project/showfiles.php?group_id=109430">Windows</a>
  Version kann
  <a href="http://sourceforge.net/project/showfiles.php?group_id=109430">hier</a>
  heruntergeladen werden.
  <br>

  <h3> Subversion Sourcecode Repository</h3>
  Der aktuellste <i>MuseScore</i> Code ist immer im SourceForge SVN
  Repository verfügbar.
  Für Hilfe zum Download bitte auf der SourceForge Projektseite
  nachschlagen.
  <br>

  <h3> Windows Testreleases </h3>
  <table>
    <tr>
      <td><a href="http://prereleases.musescore.org/mscore-r736.exe">mscore-r736.exe</a></td>
      <td>&nbsp;&nbsp</td>
      <td>26. feb 2008</td>
      </tr>
  </table>

</div>

  <h2>Voraussetzungen</h2>
<ul>
  <li> <a href="ftp://ftp.trolltech.com/qt/source">qt gui lib version 4.3</a>
      oder neuer
      <br>
      <i>MuseScore</i> kompiliert nicht mit älteren Versionen.<br>
      Vorkompilierte Pakete sind oft in "runtime" und "development"
      gesplittet. Es müssen beide Pakete installiert werden.

  <li> <a href="http://www.alsa-project.org/">ALSA</a>
       Version 1.0 oder neuer. ALSA wird für Midi-Input und Soundausgabe
       benötigt.

  <li> CMake 2.4
</ul>

Die aktuelle <i>MuseScore</i> Entwicklerplatform ist Kubuntu 7.04.

<?php require("trailer.html");  ?>


