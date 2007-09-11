<?php
  $lang="de";
  $file="lyrics.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Liedtext</h4>

  <ul>
    <li>Gib zuerst alle Noten ein.</li>
    <li>Wähle die erste Note aus.</li>
    <li>Gib <code>&lt;<b>Strg+L</b>&gt;</code> ein um in den Editmodus zu gelangen
      und gib dann den Liedtext für die erste Note ein</li>
    <li><code>&lt;<b>space</b>&gt;</code> positioniert auf die nächste Note.
       </li>
    <li>Die Eingabe von <code>&lt;<b>-</b>&gt;</code> positioniert auf die Note und
       verbindet die letzte Silbe mit der aktuellen mit einem Strich.
       </li>
    <li><code>&lt;<b>Umsch+Leertaste</b>&gt;</code> positioniert auf die vorherige Note.</li>
    <li><code>&lt;<b>Strg+Space</b>&gt;</code> fügt ein <code>&lt;<b>Leerzeichen</b>&gt;</code>
        in den Liedtext ein.
        </li>
    <li><code>&lt;<b>Strg+Minus</b>&gt;</code> fügt ein<code>&lt;<b>-</b>&gt;</code> in den
        Liedtext ein (Silbentrenner).
        </li>
    </ul>

  <img src="../pic/lyrics.png" align="center">
  <br>

  Liedtext kann wie normaler Text <a href="textedit.php">editiert</a> werden.
  <br>
  <br>
  Siehe auch <a href="chordnames.php">Akkordnamen</a>.

<?php require("trailer.html");  ?>

