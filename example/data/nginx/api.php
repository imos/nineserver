<?php

function GetWalltime()
    { return array_sum(array_map('floatval', explode(' ', microtime()))); }

define('START_TIME', GetWalltime());

function CallApiInternal($path, $data) {
  $sock = pfsockopen($_SERVER['REMOTE_ADDR'],
                     intval($_SERVER['HTTP_X_NINESERVER_PORT']));
  if ($sock === FALSE) {
    error_log('Failed to call API.' . $_SERVER['HTTP_X_NINESERVER_PORT']);
    return FALSE;
  }
  header('X-Nineserver-Proxy: ' . ftell($sock));
  $cookies = [];
  foreach ($_COOKIE as $key => $value) {
    $cookies[] = "$key=$value";
  }
  $cookie_header = '';
  if (count($cookies) > 0) {
    $cookie_header = "Cookie: " . implode('; ', $cookies) . "\r\n";
  }
  if (fwrite(
      $sock,
      "POST $path HTTP/1.1\r\n" . $cookie_header .
      "Content-Length: " . strlen($data) . "\r\n\r\n" . $data) === FALSE) {
    error_log('Failed to call fwrite.');
    return FALSE;
  }
  $headers = [];
  while (true) {
    $header = fgets($sock);
    if ($header === FALSE) {
      error_log('Header is broken');
      return FALSE;
    }
    if (count($headers) == 0) {
      $headers[''] = trim($header);
      continue;
    }
    if (trim($header) == '') break;
    list($key, $value) = explode(': ', rtrim($header), 2);
    if ($key == 'Set-Cookie') {
      header("$key: $value");
      $cookie_value = explode('; ', $value, 2);
      list($cookie_key, $cookie_value) = explode('=', $cookie_value, 2);
      $_COOKIE[$cookie_key] = $cookie_value;
    }
    $headers[$key] = $value;
  }
  $length = intval($headers['Content-Length']);
  $response = '';
  while (strlen($response) < $length) {
    $piece = fread($sock, $length - strlen($response));
    if ($piece === FALSE) {
      error_log('Not enough contents');
      return FALSE;
    }
    $response .= $piece;
  }
  return $response;
}

function CallApi($path, $params = []) {
  $data = http_build_query($params, '', '&');
  for ($i = 1; $i <= 4; $i++) {
    $response = CallApiInternal($path, $data);
    if ($response !== FALSE) break;
    error_log("Connection refused...");
    sleep($i * 0.1);
  }
  return $response;
}

function FetchAssoc($path, $params = []) {
  $data = json_decode(CallApi($path, $params), TRUE);
  $rows = [];
  for ($i = 1; $i < count($data); $i++) {
    $row = [];
    for ($j = 0; $j < count($data[$i]); $j++) {
      $row[$data[0][$j]] = $data[$i][$j];
    }
    $rows[] = $row;
  }
  return $rows;
}

function StoreSession() {
  $_COOKIE['session_data'] =
      rtrim(strtr(base64_encode(json_encode($_SESSION)), '+/', '-_'), '=');
  setcookie('session_data', $_COOKIE['session_data'], 0, '/');
}

function LoadSession() {
  $session = json_decode(base64_decode(
      strtr($_COOKIE['session_data'], '-_', '+/')), TRUE);
  $_SESSION = is_array($session) ? $session : [];
}
