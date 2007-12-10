<?php
  $file="chordnames.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Akkordnamen</h4>

  Akkordnamen können gesetzt werden, indem eine Note des Akkordes selektiert
  und dann <code>&lt;<b>Ctrl+K</b>&gt;</code> gedrückt wird.
  Dies erzeugt einen Akkordnamen und schaltet in den
  <a href="textedit.php"><i>Editmodus</i></a>.
  <br>
  <br>
  Zusätzlich stehen folgenden Kommandos zur Verfügung:
  <ul>
    <li><code>&lt;<b>Leertaste</b>&gt;</code> positioniert auf den nächsten Akkord.</li>
    <li><code>&lt;<b>Groß+Leertaste</b>&gt;</code> positioniert auf den vorherigen Akkord.</li>
    <li><code>&lt;<b>Strg+Leertaste</b>&gt;</code> gibt ein <code>&lt;<b>Leerzeichen</b>&gt;</code>
      ein </li>
    </ul>

  Akkordnamen können wie normaler Text <a href="textedit.php">editiert</a> werden.

<?php require("trailer.html");  ?>

