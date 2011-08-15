<?php

require_once("webfwlib.php");

$serverip = "192.168.123.1";

if (($serverip == getIP()) && (isset($_GET["src_ip"]))){
		//Collect connection information
		$destination = $_GET["dst_ip"];
		$dst_port = $_GET["port"];
		if ( $_GET["prot"] == "6")
        	$prot = "TCP";
		if ( $_GET["prot"] == "17")
        	$prot = "UDP";
        $hostip = $_GET["src_ip"];
        try {
                $db = new SQLiteDatabase('htfirewall.sdb');
        }
        catch (Exception $e)
        {
        echo 'Could not connect to database: ',  $e->getMessage(), "\n";
        exit(1);
        }
        if (TRUE == check_for_ip($db,$hostip)){
                // User is logged in! Next check User Policy
                if (TRUE == check_user_policy($db,getUser($db,$hostip),$destination,$dst_port,$prot))
                        echo "\nok\n";
                else {
                        echo "Policy Deny\n";
                }
        } else {
                echo "SYSTEM ERROR\n";
        }
} else {
        sleep(10);
        echo "FALSE GUESS\n";
}
?>