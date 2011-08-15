<?php
try {
        $db = new SQLiteDatabase('/mnt/db/.htfirewall.sdb');
}
catch (Exception $e){
        echo 'Could not connect to database: ',  $e->getMessage(), "\n";
        exit(1);
}
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
	if (!isset($_POST['rowid'])){
		$query = 'select * from users where username="'.$_POST['username'].'"';
		if (0 == $db->query($query)->numRows()) {
			echo 'The User '.$_POST['username'].' does not exist!';
		}
		else {
			$query = 'insert into user_policy (username,dst_ip,dst_port,prot) values ("'.$_POST['username'].'", "'.$_POST['dst_ip'].'", "'.$_POST['dst_port'].'", "'.$_POST['prot'].'")';
			$db->queryExec($query);
			echo "Added new rule to policy";
		}
	}
	else {
		$query = 'select * from users where username="'.$_POST['username'].'"';
		if (0 == $db->query($query)->numRows()) {
			echo 'The User '.$_POST['username'].' does not exist!';
		}
		else {
			$query = 'update user_policy set
				username="'.$_POST['username'].'",
				dst_ip="'.$_POST['dst_ip'].'",
				dst_port="'.$_POST['dst_port'].'",
				prot="'.$_POST['prot'].'" where rowid='.$_POST['rowid'];
			$db->queryExec($query);
		}
	}
}
echo '<table id="policy"><thead><tr><td>User</td><td>Destination</td><td>Protocol</td><td>Service</td></tr></thead><tbody>';
$query = "select * from user_policy";
if (0 == $db->query($query)->numRows()) {
        echo '<tr><td colspan="5">No policy defined yet!<td></tr>';
}
else {
   	$results = $db->query($query);
    while ($row = $results->fetch()){
    	$i++;
        echo '<tr class="d'.($i & 1).'"><form method="POST" action="?section=userpolicy">';
        echo '<input type="hidden" name="rowid" value="'.$row['rowid'].'">';
        echo '<td><input type="text" size="15" name="username" value="'.$row['username'].'"></td>';
        echo '<td><input type="text" size="15" name="dst_ip" value="'.$row['dst_ip'].'"></td>';
        echo '<td><input type="text" size="15" name="prot" value="'.$row['prot'].'"></td>';
        echo '<td><input type="text" size="15" name="dst_port" value="'.$row['dst_port'].'">';
        echo '<a href="delete.php?target=policy&rowid='.$row['rowid'].'"><img src="static/delete.png" border="0" id="delete"></a>';
        echo '<input type="image" id="update" name="update" value="update" src="static/refresh.png" alt="Submit">';
        echo '</td></form></tr>';
    }
}
$i++;
echo '<tr class="d'.($i & 1).'"><form method="POST" action="?section=userpolicy">';
echo '<td><input type="text" size="15" name="username" value="Username" onFocus="clearText(this)" onBlur="clearText(this)"></td>';
echo '<td><input type="text" size="15" name="dst_ip" value="Destination" onFocus="clearText(this)" onBlur="clearText(this)"></td>';
echo '<td><input type="text" size="15" name="prot" value="Protocol" onFocus="clearText(this)" onBlur="clearText(this)"></td>';
echo '<td><input type="text" size="15" name="dst_port" value="Port" onFocus="clearText(this)" onBlur="clearText(this)">';
echo '<input type="image" id="add" name="addnew" value="addnew" src="static/add.png" alt="Submit"></td></form></tr>';
echo "</tbody></table>";
?>