<?php

require_once('api.php');

header('Content-Type: text/plain');
var_dump(FetchAssoc('/sql'));
