<html><head>
        <title>WebFW Landing Page</title>
		<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
        <link type="text/css" rel="stylesheet" href="/style.css" />
        <script type="text/javascript">
        function clearText(field){
                if (field.defaultValue == field.value)
                        field.value = '';
                else if (field.value == '')
                        field.value = field.defaultValue;
        }
        </script>
</head>
<body>
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
// session check
session_start();
if (!isset($_SESSION['uname'])){
        if ($_SERVER['REQUEST_METHOD'] === 'POST') {
                $user = $_POST['user'];
                $password = $_POST['password'];

                #check for user in database
                $users = $db->query("SELECT * FROM USERS WHERE USERNAME='$user'");
                if (!$users){
                       	echo '<div id="content">Wrong Username!<</div>';
                        header('refresh: 5; url=/');
                        exit(1);
                }
                while ($row = $users->fetch()){
                        $hash = $password;
                        $rounds = 4096;
        
                        for ( $i = 1; $i <= $rounds; $i++)
                        $hash = hash('sha256', $hash);
                        if ($row['password'] == $hash) {
                                echo '<div id="content"<img id="logo" src="webfw.png" border="0"><br>Welcome '.$row['username'].'<br/';
                        		insertDB($db,getIP(),$row['username']);
                                // Save the username in the Session
                                $_SESSION['uname'] = $user;
                       		 	echo "Don't close this page, or your session on the firewall will be terminated!</div>";
                		}
                        else { 
                                echo '<div id="content">Wrong Password</div>';
                        }
                        header('refresh: 5; url=/');
                        ?></body></html> <?php
                        exit(1);
                }
        } else { ?>
        <div id="content">
                <img id="logo" src="webfw.png" border="0"><br>
                Welcome to the WebFW login page. Please enter your username and  password to gain access to
            protected network services
            <form action="index.php" method="POST">
                <input type="text" name="user" value="username" onFocus="clearText(this)" onBlur="clearText(this)" /><br/>
                <input type="password" name="password" value="password" onFocus="clearText(this)" onBlur="clearText(this)" /><br/>
                <input type="submit" value="Sign in!" />
            </form>
                 </div>
        <?php } 
}
else { ?>
        <div id="content">
                <img id="logo" src="webfw.png" border="0"><br>
                Weclome 
                <?php echo $_SESSION['uname']; 
                refreshDB($db,getIP(),$_SESSION['uname']);?>
                Don't close this page, or your session on the firewall will be terminated!<br/>
                <a href="logout.php">End Session</a>
                <?php header('refresh: 60; url=/'); ?>
        </div>
<?php
}
?>
</body></html>
        