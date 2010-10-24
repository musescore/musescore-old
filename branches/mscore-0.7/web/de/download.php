<?php
  $lang="de";
  $file="download.php";
  require("header.html");
  ?>
<div id="intro">
  <h2> Downloads</h2>
  <h3> Sources</h3>
    Die <i>MuseScore</i> Quellen gibt es bei
   <a href="http://sourceforge.net/project/showfiles.php?group_id=109430">SourceForge</a>.
  <br><br>

  <h3> Binaries</h3>
  <b>Windows: </b><a href="http://sourceforge.net/project/showfiles.php?group_id=109430">SourceForge</a>.
  <br>
  (Die Windowsversion enthält z.Z. noch keinen Sequencer und keinen
  Synthesizer.)

  <h3> Subversion Sourcecode Repository</h3>
  Der aktuellste <i>MuseScore</i> Code ist immer im SourceForge SVN
  Repository verfügbar.<br>
  Für Hilfe zum Download bitte auf der SourceForge Projektseite
  nachschlagen.

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

