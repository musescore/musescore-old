<?php
  $file="accidentals.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a> -- <a href="manual.php">Dokumentation</a> -- <a href="reference.php">Index</a> -- Vorzeichen</h4>
<table>
  <tr>
    <td>

          Vorzeichen können durch Ziehen eines Vorzeichensymbols von der
          Vorzeichenpalette auf eine Note gesetzt oder verändert werden.
                <br/>
      <br/>

          Wenn sie nur die Tonhöhe einer Note verändern wollen können
          sie auch die Note selektieren und dann:
                <br/>
      <br/>
      </td>
    <td valign="top">
      <img src="../pic/accidentalspalette.png"/>
      </td>
    </tr>
  </table>
<table cellspacing="0" cellpadding="0">
  <tr>
    <td>
&lt;<b>Hoch</b>&gt;      </td>
    <td>
Erhöht die Note um einen Semiton.      </td>
    </tr>
  <tr>
    <td>
&lt;<b>Tief</b>&gt;      </td>
    <td>
Erniedigt die Note um einen Semiton.      </td>
    </tr>
  <tr>
    <td>
&lt;<b>Groß+Hoch</b>&gt;      </td>
    <td>
Erhöht die Note um eine Oktave.      </td>
    </tr>
  <tr>
    <td>
&lt;<b>Groß+Tief</b>&gt;      </td>
    <td>
Erniedigt die Note um eine Oktave.      </td>
    </tr>
  </table>
<br/>
<i>MuseScore</i> wählt automatisch die Position der Note und evtl. ein
    Vorzeichen. Wenn sie damit nicht einverstanden sind oder einfach
    nur ein Sicherheitsvorzeichen setzen wollen, dann können sie immer noch
    manuell ein Vorzeichen aus der Vorzeichenpalette auf die Note ziehen
    und die Automatik damit überstimmen. Wenn anschließend wieder die
    Tonhöhe verändert wird, geht diese manuelle Wahl natürlich verloren.
    <br/>
<br/>

    Die Menüfunktion &lt;<b>Notes/Pitch spell</b>&gt; versucht, die Vorzeichen
    der gesamten Partitur neu zu berechnen.
  <?php require("trailer.html"); ?>
