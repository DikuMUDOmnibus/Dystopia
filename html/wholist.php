<?php
  include 'core.php';

  echoHead("Who's Online");

  echo "<h3>Who's Online at Dystopia II</h3>\n<pre>\n";
  WebSubmit("GET wholist.html", TRUE);
  echo "</pre>\n";

  echoFoot();
?>
