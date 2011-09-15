<?php 
try {
        $usersdb = new SQLiteDatabase('/etc/webfw-admin/etc/users.sdb');
        $db = new SQLiteDatabase('/etc/webfw-admin/etc/htfirewall.sdb');
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
                $usersdb->queryExec($query);
                system("scp -i /etc/webfw-admin/etc/id_rsa -o GlobalKnownHostsFile=/etc/webfw-admin/etc/known_hosts /etc/webfw-admin/etc/users.sdb root@192.168.123.2:/mnt/db/users.sdb");
                header("Location: /?section=users");
                break;
        default:
                header("Location: /");
}
?>