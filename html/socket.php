<?php
function WebSubmit($in, $echo)
{
  $out = '';
  $sockend = fsockopen("127.0.0.1", 9008);

  if ($sockend)
  {
    fputs($sockend, $in);
    fputs($sockend, "\r\n\r\n");

    while (!feof($sockend))
      $out .= fgets($sockend, 1024);

    if ($echo == TRUE)
      echo $out;

    fclose($sockend);
  }
  else
  {
    echo "fsockopen error";
  }

  return $out;
}

function AppendEntry($key, $entry)
{
  return ($key . chr(27) . $entry . chr(27));
}

function ShowNote($in)
{
  $sockend = fsockopen("127.0.0.1", 9008);
  $out = '';

  if ($sockend)
  {
    fputs($sockend, $in);
    fputs($sockend, "\r\n\r\n");

    while (!feof($sockend))
      $out .= fgets($sockend, 1024);

    fclose($sockend);
  }
  else
  {
    $out = "0" . chr(27) . "0" . chr(27) . "The MUD is currently down.";
  }

  return $out;
}

function GetLevel()
{
  global $account;
  global $password;
  $sockend = fsockopen("127.0.0.1", 9008);
  $request  = "REQUEST lvl.info " . chr(27) . "2" . chr(27);
  $request .= AppendEntry("account", $account) . AppendEntry("password", $password);
  static $oldaccount = '';
  static $oldpass = '';
  static $oldlevel = 0;
  $out = '';

  /* no need to redo old requests */
  if (!strcmp($account, $oldaccount) && !strcmp($password, $oldpass))
  {
    if ($sockend)
      fclose($sockend);

    return $oldlevel;
  }

  if ($sockend)
  {
    fputs($sockend, $request);
    fputs($sockend, "\r\n\r\n");

    while (!feof($sockend))
      $out .= fgets($sockend, 1024);

    fclose($sockend);

    if (is_numeric($out))
    {
      $oldaccount = $account;
      $oldpass = $password;
      $oldlevel = $out;

      return $out;
    }
    else
      return 0;
  }
  else
  {
    return 0;
  }
}

function GetPlayers()
{
  global $account;
  global $password;
  $sockend = fsockopen("127.0.0.1", 9008);
  $request  = "REQUEST player.info " . chr(27) . "2" . chr(27);
  $request .= AppendEntry("account", $account) . AppendEntry("password", $password);
  $players[0] = '';
  $out = '';
  $k = 0;

  if ($sockend)
  {
    fputs($sockend, $request);
    fputs($sockend, "\r\n\r\n");

    while (!feof($sockend))
      $out .= fgets($sockend, 1024);

    fclose($sockend);

    for ($i = 0; $i < strlen($out); $i++)
    {
      $temp = '';

      while ($out[$i] != chr(27) && $i < strlen($out))
        $temp .= $out[$i++];

      $players[$k++] = $temp;
    }
  }

  return $players;
}
?>
