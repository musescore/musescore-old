<?php
  $file="clefs.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Schlüssel</h4>

<table>
<tr><td>
      Durch ziehen eines Schlüssels von der Schlüsselpalette auf einen
      Takt oder einen Schlüssel der Partitur erzeugt oder
      verändert Schlüssel.<br>
      <br>

  <b>Verändern</b><br>
  Ziehen sie einen Schlüssel von der Palette auf einen Schlüssel
  der Partitur. Statt von einer Palette kann mit
  &lt;<b>Shift+leftMouseButton+Drag</b>&gt;.<br> auch ein
  Schlüssel der Partitur wie von einer Palette gezogen
  werden.
  <br>
  <br>
  <b>Einfügen</b><br>
  Ziehen sie einen Schlüssel von einer Palette auf einen leeren
  Teil eines Taktes. Dies erstellt einen Schlüssel am Anfang des
  Taktes. Schlüssel, die nicht am Zeilenanfang stehen werden
  kleiner dargestellt.
  <br>
  <br>
  <b>Löschen</b><br>
  Selektieren sie einen Schlüssel und drücken dann &lt;<b>Del</b>&gt;.
  </td>
  <td>
     <img src="../pic/clefpalette.png">
     </tr>
  </td>
  </table>

<br>
Achtung: Das verändern/einfügen von Schlüsseln verändert nicht die
Tonhöhe irgendeiner Note. Statt dessen werden die Noten verschoben.

<?php require("trailer.html");  ?>

