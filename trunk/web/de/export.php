<?php
  $file="export.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Referenz</a>
   -- Export</h4>

Eine Partitur kann über das Pulldownmenü "Partitur->Sichern als" in
verschiedene Formate exportiert werden:
<br>
<br>
<table>
   <tr>
      <td valign="top"><b><tt>*.msc</tt></b></td>
      <td>&nbsp;&nbsp;&nbsp;</td>
      <td>
         Dies ist das <i>MuseScore</i> eigene Dateiformat. Wird die Partitur in diesem
         Format gespeichert, so gehen keine Informationen verloren.
         </td>
      </tr>
   <tr>
      <td valign="top"><b><tt>*.xml</tt></b></td>
      <td/>
      <td>
         <a href="http://www.recordare.com/xml.html">MusicXml</a>
         wird von vielen anderen Notensatzprogrammen verstanden
         und eignet sich deshalb als programmunabhängiges Austauschformat.
         </td>
      </tr>
   <tr>
      <td valign="top"><b><tt>*.mid</tt></b></td>
      <td/>
      <td>
         Praktisch alle Notensatzprogramme und Sequencer verstehen Midifiles. Als
         Austauschformat eignet es sich jedoch nicht gut da viele
         Notensatzinformationen verloren gehen.
         </td>
      </tr>
   <tr>
      <td valign="top"><b><tt>*.pdf</tt></b></td>
      <td/>
      <td>
         PDF
         </td>
      </tr>
   <tr>
      <td valign="top"><b><tt>*.ps</tt></b></td>
      <td/>
      <td>Postscript</td>
      </tr>
   <tr>
      <td valign="top"><b><tt>*.png</tt></b></td>
      <td/>
      <td>PNG Bitmap Grafik</td>
      </tr>
   <tr>
      <td valign="top"><b><tt>*.svg</tt></b></td>
      <td/>
      <td>SVG Skalierbare Vektorgrafik</td>
      </tr>
   <tr>
      <td valign="top"><b><tt>*.ly</tt></b></td>
      <td/>
      <td>
         Dieses Format kann vom Lilypond Notensatzprogramm gelesen werden.
         Die aktuelle <i>MuseScore</i> Version unterstützt dieses Format nur
         rudimentär.
         </td>
      </tr>
   </table>
<?php require("trailer.html");  ?>

