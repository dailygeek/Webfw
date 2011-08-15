<?php 
require_once("webfwlib.php");

try {
	$db = new SQLiteDatabase('/mnt/db/.htfirewall.sdb');
}
catch (Exception $e)
{
    echo 'Could not connect to database: ',  $e->getMessage(), "\n";
    exit(1);
}

removeDB($db,$_SESSION['uname']);

session_start();
session_destroy();

header('location: /');
?>