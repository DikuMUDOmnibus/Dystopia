<?php
  include 'socket.php';

  import_request_variables("G");

  echo "<html><body>";
  $request = "none";

  if (!isset($help) || empty($help))
    echo "There is no helpfile by that name.";
  else
    $request = "GET help.html " . chr(27) . "1" . chr(27) . AppendEntry("help", $help);

  echo "<pre>";

  if ($request != "none")
    WebSubmit($request, TRUE);

  echo "</pre>";
  echo "</body></html>";
?>
