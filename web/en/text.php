<?php
  $file="text.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a> -- <a href="manual.php">Documentation</a> -- <a href="reference.php">Index</a> -- Text</h4>

    Text elements are created from a <i>Text Style</i>. This style defines
    the initial properties of text.
    <br/>
<h5>
Text properties:  </h5>
<table cellspacing="0" cellpadding="0">
  <tr>
    <td>
<b>font family</b>      </td>
    <td>
&nbsp;&nbsp;      </td>
    <td>
this is the name of the font      </td>
    </tr>
  <tr>
    <td>
<b>point size</b>      </td>
    <td>
      </td>
    <td>
the size of the font in points      </td>
    </tr>
  <tr>
    <td>
<b>italic, bold, underline</b>      </td>
    <td>
      </td>
    <td>
font properties      </td>
    </tr>
  <tr>
    <td>
<b>anchor</b>      </td>
    <td>
      </td>
    <td>
page, time, notehead, system, staff      </td>
    </tr>
  <tr>
    <td>
<b>alignment</b>      </td>
    <td>
      </td>
    <td>
horizontal: left, right center; vertical: top, bottom, center      </td>
    </tr>
  <tr>
    <td>
<b>offset</b>      </td>
    <td>
      </td>
    <td>
an offset to the normal anchor position      </td>
    </tr>
  <tr>
    <td>
<b>offset type</b>      </td>
    <td>
      </td>
    <td>
mm, space or % of pagesize      </td>
    </tr>
  </table>
<br/>
<h5>
Text types:  </h5>
<table cellspacing="0" cellpadding="0">
  <tr>
    <td>
<b>titel, subtitle,      <br/>
composer, poet</b>      </td>
    <td>
&nbsp;&nbsp;      </td>
    <td>
achord to page      </td>
    </tr>
  <tr>
    <td>
      <a href="fingering.php">
<b>fingering</b>        </a>
      </td>
    <td>
      </td>
    <td>
Fingerings are anchord to note heads.      </td>
    </tr>
  <tr>
    <td>
      <a href="lyrics.php">
<b>lyrics</b>        </a>
      </td>
    <td>
      </td>
    <td>
Lyrics are achored to a time position.      </td>
    </tr>
  <tr>
    <td>
      <a href="chordnames.php">
<b>chord names</b>        </a>
      </td>
    <td>
      </td>
    <td>
Chord names are also anchord to a time position.      </td>
    </tr>
  </table>
<?php require("trailer.html"); ?>
