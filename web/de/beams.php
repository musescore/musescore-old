<?php
  $lang="de";
  $file="beams.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Balken</h4>

<table>
<tr><td>
      Balken werden automatisch gesetzt. Diese Automatik kann mit
      manuell gesetzten Hinweisen gesteuert werden. Dazu wird die
      Note selektiert deren Balkenverhalten geändert werden soll
      und anschließend ein Button im Balken-Pad gedrückt.
  </td>
  <td>
     <img src="../pic/beampad.png">
     </td>
  </tr>
  </table>

<table cellpadding="0" cellspacing="0">
  <tr><td><img src="../pic/beambutton1.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Beginnt einen Balken bei der selektierten Note.</td>
    </tr>
  <tr><td><img src="../pic/beambutton2.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Ein Balken wird an dieser Note nicht beendet.</td>
    </tr>
  <tr><td><img src="../pic/beambutton3.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Diese Note soll nicht verbalkt werden.</td>
    </tr>
  <tr><td><img src="../pic/beambutton4.png"></td>
    <td>&nbsp;&nbsp;</td>
    <td>Beginnt eine neue Balkenebene an dieser Note.</td>
    </tr>

  </table>

<br>
<?php require("trailer.html");  ?>

