<?php
  $file="repeats.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a> -- <a href="manual.php">Dokumentation</a> -- <a href="reference.php">Index</a> -- Wiederholungen</h4>
<ul>
  <li>
 Einfache Wiederholungen    </li>
  <li>
    <a href="volta.php">
Volta      </a>
    </li>
  <li>
 Coda    </li>
  </ul>
<h5>
Einfache Wiederholungen  </h5>


    Anfang und Ende einfacher Wiederholungen können durch Setzen enprechender
    <a href="barlines.php">
Taktstriche  </a>
 definiert werden.<br/>

    Im letzen Takt einer Wiederholung kann unter
    <a href="measures.php">
&quot;Eigenschaften&quot;  </a>
 die Anzahl
    der vorgesehenen Wiederholungen gesetzt werden. Soll eine Wiederholung
    beim Zweiten mal etwas anders gespielt werden, so können die
    Varianten mit einer <a href="volta.php">
Volta  </a>
 gekennzeichnet werden.


    <h5>
Sprünge  </h5>


    Sprünge bestehen allgemein aus drei Teilen:
          <ul>
  <li>
 springe nach <i>Marke</i>    </li>
  <li>
 spiele bis <i>Marke</i>    </li>
  <li>
 weiter ab <i>Marke</i>    </li>
  </ul>

    Marken sind Namen, die wir einer Taktposition geben. Zwei Sprungmarken
    (&quot;Anfang&quot;, &quot;Ende&quot;) bezeichnen den Anfang und das Ende eines Stückes und müssen
    nicht speziell vereinbart werden.
    <br/>
<br/>

    Beispiele:<br/>
<br/>

    Bei der Sprunganweisung <i>Da Capo</i> springt die Wiedergabe an den Anfang
    und spielt das gesamte Stück noch einmal (bis zur impliziten <i>Ende</i> Marke).
    <br/>
<br/>

    Bei der Sprunganweisung <i>Da Capo al Fine</i> springt die Wiedergabe an den
    Anfang um dann bis zur Marke <i>Fine</i> zu spielen.
    <br/>
<br/>
<i>Dal Segno al Fine</i> (oder <i>D.S. al Fine</i>) springt zur <i>Segno</i> Marke und
    spielt dann bis zur Marke <i>Fine</i><br/>
<br/>
<i>Dal Segno al Coda</i> springt zur <i>Segno</i> Marke und spielt dann bis zur
    ersten <i>Coda</i> Marke. Das Spiel wird dann an der zweiten Codamarke
    fortgesetzt.
    <br/>
<br/>
<br/>

    Siehe auch: <a href="volta.php">
Volta  </a>
<?php require("trailer.html"); ?>
