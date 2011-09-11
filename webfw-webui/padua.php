<?php

require_once("webfwlib.php");

#$serverip = "192.168.123.1";
$serverip =  getIP();
if (($serverip == getIP()) && (isset($_GET["src_ip"]))){
        $hostip = $_GET["src_ip"];
        try {
                $db = new SQLiteDatabase('/mnt/db/.htfirewall.sdb');
        }
        catch (Exception $e)
        {
        echo 'Could not connect to database: ',  $e->getMessage(), "\n";
        exit(1);
        }
        if (TRUE == check_for_ip($db,$hostip)){
                        echo getUser($db,$hostip);
        } else {
                echo "SYSTEM ERROR";
        }
} else {
        sleep(10);
        echo "FALSE GUESS\n";
}
?>