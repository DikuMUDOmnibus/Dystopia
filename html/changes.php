<?php
  include 'core.php';

  echoHead("Changes");
  echo "<h3>Latest Changes at Dystopia II</h3>\n<pre>\n";

  WebSubmit("GET changes.html", TRUE);

  echo "</pre>\n";
  echoFoot();
?>
