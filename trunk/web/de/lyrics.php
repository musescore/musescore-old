<?php
  $lang="de";
  $file="lyrics.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a>
   -- <a href="manual.php">Reference</a>
   -- Lyrics</h4>

  <ul>
    <li>First enter notes.</li>
    <li>Select first note.</li>
    <li>Type <code>&lt;<b>Ctrl+L</b>&gt;</code> and enter lyrics text for
      first note.</li>
    <li>Type <code>&lt;<b>space</b>&gt;</code> at end of word to go next
       note.</li>
    <li>Type <code>&lt;<b>-</b>&gt;</code> at end of syllable to go to next note.
       The syllables are connected with a dash.
       </li>
    <li><code>&lt;<b>Shift+Space</b>&gt;</code> moves to the previous note.</li>
    <li><code>&lt;<b>Ctrl+Space</b>&gt;</code> enters a <code>&lt;<b>space</b>&gt;</code> into
        the lyrics text.
        </li>
    <li><code>&lt;<b>Ctrl+Minus</b>&gt;</code> enters a <code>&lt;<b>-</b>&gt;</code> into
        the lyrics text.
        </li>
    </ul>

  Lyrics can be <a href="textedit.php">edited</a> as normal text.

<?php require("trailer.html");  ?>

