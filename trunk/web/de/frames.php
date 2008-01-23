<?php
  $file="frames.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a> -- <a href="manual.php">Dokumentation</a> -- <a href="reference.php">Index</a> -- Rahmen</h4>

    Rahmen sind Platzhalter außerhalb der normalen Takte. Sie können leer sein
    oder Texte oder Bilder enthalten. <i>MuseScore</i> kennt zwei Arten von
    Rahmen:
    <br/>
<br/>
<table cellspacing="0" cellpadding="0">
  <tr>
    <td valign="top">
<b>horizontal</b>      </td>
    <td>
&nbsp;&nbsp;      </td>
    <td>
Diese Rahmen unterbrechen ein Notensystem. Ihe Breite ist einstellbar,
           die Höhe enspricht der aktuellen Systemhöhe. Sie werden z.B. zum Absetzen
           einer Coda benutzt.      </td>
    </tr>
  <tr>
    <td>
      <br/>
      </td>
    </tr>
  <tr>
    <td valign="top">
<b>vertikal</b>      </td>
    <td>
      </td>
    <td>
Vertikale Rahmen schaffen Platz zwischen oder vor Notensystemen. Ihre
       Höhe ist einstellbar und ihre Breite entspricht der Systembreite.
       Sie werden z.B. für Titel, Subtitel oder Komponist genutzt. Beim Erstellen
       eines Titels wird automatisch ein vertikaler Rahmen vor dem ersten Takt
       erzeugt, wenn nicht bereits vorhanden.
             </td>
    </tr>
  </table>
<br/>
<h5>
Rahmen erzeugen  </h5>

    Zunächst muß der Takt ausgewählt werden, <i>vor</i> dem ein Rahmen einfügt
    werden soll. Das Kommando zum Einfügen von Rahmen befindet sich unter
    <b>Erzeugen-&gt;Takte</b>.

    <h5>
Rahmen löschen  </h5>

    Rahmen markieren und dann &lt;<b>Entf</b>&gt; drücken.

    <h5>
Rahmen editieren  </h5>

    Ein Doppelclick innerhalb eines Rahmens schalten in den
    <a href="editmode.php">
<b>Editiermodus</b>  </a>
.
    Es erscheint ein Anfasser, mit dem die Größe des Rahmens verstellt werden
    kann.<br/>
<br/>

    Titelrahmen im Editiermodus:<br/>
<br/>
<img src="../pic/frame.png"/>
<?php require("trailer.html"); ?>
