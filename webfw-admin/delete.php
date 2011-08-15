<?php 
try {
        $db = new SQLiteDatabase('/mnt/db/.htfirewall.sdb');
}
catch (Exception $e){
        echo 'Could not connect to database: ',  $e->getMessage(), "\n";
        exit(1);
}
switch ($_GET['target']) {
	case "policy":
		$query = 'delete from user_policy where rowid='.$_GET['rowid'];
		$db->queryExec($query);
		header("Location: /?section=userpolicy");
		break;
	case "user":
        $query = 'delete from users where username="'.$_GET['username'].'"';
		$db->queryExec($query);
		header("Location: /?section=users");
		break;
	default:
		header("Location: /");
}
?>