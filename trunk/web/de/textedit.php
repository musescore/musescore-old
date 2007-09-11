<?php
  $lang="de";
  $file="textedit.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Text Editieren</h4>

  Ein Doppelclick auf einen Text startet den <i>Editiermodus</i>:
  <img src="../pic/textedit.png" align="center"><br>
  <br>
  Im <i>edit mode</i> stehen folgende Kommandos zur Verfügung:

  <ul>
    <li><code>&lt;<b>Ctrl+B</b>&gt;</code> Fettschrift ein/aus</li>
    <li><code>&lt;<b>Ctrl+I</b>&gt;</code> Kursivschrift ein/aus</li>
    <li><code>&lt;<b>Ctrl+U</b>&gt;</code> Unterstreichen ein/aus</li>
    <li><code>&lt;<b>Up</b>&gt;</code>Hochstellen</li>
    <li><code>&lt;<b>Down</b>&gt;</code>Tiefstellen</li>
    <li>Cursor bewegen: <code>&lt;<b>Pos1</b>&gt; &lt;<b>Ende</b>&gt; &lt;<b>Links</b>&gt;
       &lt;<b>Rechts</b>&gt;</code>
    <li><code>&lt;<b>Zurück</b>&gt;</code> löscht Zeichen links vom Cursor</li>
    <li><code>&lt;<b>Delete</b>&gt;</code> löscht Zeichen rechts vom Cursor</li>
    <li><code>&lt;<b>Return</b>&gt;</code> neue Zeile beginnen</li>
    <li><code>&lt;<b>F2</b>&gt;</code> Zeigt die Textpalette. Die Textpalette kann
        zur Eingabe von Zeichen und Symbolen verwendet werden, die nicht auf
        der Tastatur verfügbar sind.</li>
    </ul>

      See also:
            <a href="chordnames.php">Akkordnamen</a>,
            <a href="lyrics.php">Liedtext</a>

<?php require("trailer.html");  ?>

