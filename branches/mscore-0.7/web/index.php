<head>
<?php
  $file="idx.php";
  $languages = $_SERVER['HTTP_ACCEPT_LANGUAGE'];
  $ar = preg_split('/,/', $languages, -1, PREG_SPLIT_NO_EMPTY);
  if ($ar[0] == "de") {
      echo "<meta http-equiv=\"refresh\" content=\"0; URL=de/idx.php\">";
      }
  else {
      echo "<meta http-equiv=\"refresh\" content=\"0; URL=en/idx.php\">";
      }
  ?>
</head>
