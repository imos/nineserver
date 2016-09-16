<?php

require_once('api.php');

header('Content-Type', 'text/plain; charset=UTF-8');
LoadSession();

ob_start();
print_r($_SESSION);

$_SESSION['md5'] = md5($_SESSION['md5']);
$_SESSION['counter']++;
$_SESSION['time'] = date('Y-m-d H:i:s');
$_SESSION['object'] = ['array' => [1, 2, 3], 'object' => ['a' => 'b']];

print_r($_SESSION);
StoreSession();
