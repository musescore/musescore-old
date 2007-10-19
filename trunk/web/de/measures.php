<?php
  $lang="de";
  $file="measures.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Takte bearbeiten</h4>


<h5>Eigenschaften</h5>

<table cellspacing="0" cellpadding="0">
<tr>
  <td>
      <b>Taktdauer</b><br>
      Normalerweise ist die nominelle und die tatsächliche Taktdauer gleich.
      Ein Auftakt kann jedoch eine andere tatsächliche Dauer haben.
      <br><br>

      <b>Irregular</b><br>
        Wenn der Takt als irregular gekennzeichnet ist, dann wird er
        nicht gezählt. Dieses Flag wird allgemein für Auftakte gesetzt.
      <br><br>

      <b>Wiederholungszahl</b><br>
        Wenn der Takt das Ende einer
        <a href="repeats.php">Wiederholung</a> darstellt, dann kann hier
        angegeben werden, wie oft wiederholt werden soll.

     </td>
  <td valign="top">
     <img src="../pic/measure.png">
     </td>
  </tr>
</table>
<br>

<?php require("trailer.html");  ?>

