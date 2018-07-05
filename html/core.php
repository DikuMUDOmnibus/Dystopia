<?php

include 'socket.php';

function echoHead($title)
{
  header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
  header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
  header("Cache-Control: no-store, no-cache, must-revalidate");
  header("Cache-Control: post-check=0, pre-check=0", false);
  header("Pragma: no-cache");

  echo "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n";
  echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n";
  echo "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
  echo "<html>\n";
  echo "  <head>\n";
  echo "    <title>Calims's Cradle :: " . $title . "</title>\n";
  echo "    <link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>\n";
  echo "  </head>\n";
  echo "<body onload=\"init()\">\n";
  echo "<div class=\"page\">\n\n";
}

function echoFoot()
{
  echo "\n</div>\n\n";
  echo "</body>\n";
  echo "</html>\n";
}

?>
