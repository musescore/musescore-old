<?php
  $file="measures.php";
  require("header.html");
  ?>
<h4><a href="idx.php">MuseScore</a> -- <a href="manual.php">Documentation</a> -- <a href="reference.php">Index</a> -- Measure Operations</h4>
<h5>
Insert Measures  </h5>


      First select a measure then press &lt;<b>Ins</b>&gt; to insert a new empty measure before
      the selected on.

    <h5>
Delete Measures  </h5>


      Complete measures can be deletet by first selecting them with &lt;<b>Ctrl+Click</b>&gt;.
      The measure is marked with a dottet line indicating that you selected a &quot;piece of time&quot;.
      Press &lt;<b>Shift+Click</b>&gt; to extent the selection.
      Pressing &lt;<b>Del</b>&gt; removes the selected measures.


    <h5>
Properties  </h5>
<table cellspacing="0" cellpadding="0">
  <tr>
    <td>
<b>Measure Duration</b>      <br/>

          Normally the nominal and actual duration of a measure is identical.
          An upbeat can have a different actual duration.
                <br/>
      <br/>
<b>Irregular</b>      <br/>

            An &quot;irregular&quot; measure will not be counted. Normally an upbeat
            is not counted as a regular measure.
                <br/>
      <br/>
<b>Repeat Count</b>      <br/>

            If the measure is the end of a
                  <a href="repeats.php">
repeat        </a>
 you can define how often the
            repeat should be played.

               </td>
    <td valign="top">
      <img src="../pic/measure.png"/>
      </td>
    </tr>
  </table>
<br/>
<?php require("trailer.html"); ?>
