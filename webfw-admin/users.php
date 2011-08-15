<?php
try {
        $db = new SQLiteDatabase('/mnt/db/.htfirewall.sdb');
}
catch (Exception $e){
        echo 'Could not connect to database: ',  $e->getMessage(), "\n";
        exit(1);
}
if ($_SERVER['REQUEST_METHOD'] === 'POST') {

  	$hash = $_POST['password'];
	$rounds = 4096;
	
   	for ( $i = 1; $i <= $rounds; $i++)
      $hash = hash('sha256', $hash);

	$query = 'insert into users values ("'.$_POST['username'].'", "'.$hash.'")';
	$db->queryExec($query);
}
echo '<form method="POST" action="?section=users">';
echo '<table id="policy"><thead><tr><td>User</td></tr></thead><tbody>';
$query = "select * from users";
if (0 == $db->query($query)->numRows()) {
	echo '<tr><td colspan="5">No policy defined yet!<td></tr>';
}
else {
	$results = $db->query($query);
    while ($row = $results->fetch()){
		$i++;
        echo "<tr class=\"d".($i & 1)."\">";
        echo "<td>".$row['username'];
        echo '<a href="delete.php?target=user&username='.$row['username'].'"><img src="static/delete.png" border="0" id="delete"></a>';
        echo "</td></tr>";
    }
}
$i++;
echo "<tr class=\"d".($i & 1)."\">";
echo '<td><input type="text" size="15" name="username" value="Username" onFocus="clearText(this)" onBlur="clearText(this)">';
echo '<input type="password" size="15" name="password" value="Password" onFocus="clearText(this)" onBlur="clearText(this)">';
echo '<input type="image" id="add" src="static/add.png" alt="Submit"></td></tr>';
echo "</tbody></table>";
echo '</form>';
?>