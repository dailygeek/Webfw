<html>
<head>
	<title>webfw Administrator Interface</title>
	<link type="text/css" rel="stylesheet" href="static/style.css" />
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
	<div id="wrapper">
		<div id="head">
			<img id="logo" src="static/webfw.png" border="0">
		</div>
		<div id="navigation">
		<ul>
			<li><a href="?section=paketfilter">Packet Filter</a></li>
			<li><a href="?section=userpolicy">User Policy</a></li>
			<li><a href="?section=users">Users</a></li>
			<li><a href="?section=networking">Network Configuration</a></li>
		<ul>
		</div>
		<div id="content">
			<?php
			$section = $_GET['section'];
			
			switch ($section){
				case 'paketfilter':
					echo "Paketfilter";
					break;
				case 'userpolicy':
					include('userpolicy.php');
					break;
				case 'users':
					include('users.php');
					break;
				case 'networking':
					echo "Network Settings";
					break;
				default:
					echo 'Welcome to the webfw Administrator Interface';
			}
			?>
		</div>
		<div id="push"></div>
	</div>
	<div id="footer">
	<p>Copyright (c) 2011</p>
	</div>
</body>
</html>