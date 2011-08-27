<?php

function getTS(){
   $tmp = gettimeofday();
   return (int)$tmp['sec'];
}

function timediff($value){
          $current=getTS();
          $tmp = $current - (int)$value;
          if ($tmp < 0) $tmp *= -1;
        return $tmp;
}

function removeDB($conn,$uname){
  $query = "delete from list where user='{$uname}'";
  if (0 == $conn->queryExec($query)) {
        die($query.";\n failed\n");
  }
}

function getListDB($conn) {
    $sql = 'SELECT ip,timestamp FROM list';
        serviceDB($conn);
    foreach ($conn->query($sql) as $row) {
                $tdiff = timediff($row['timestamp']);

//              if ($tdiff > 600) {removeDB($conn,$row['login']); continue;}

        print $tdiff . "\t";
        print $row['ip'] . "\n";
    }
}

function insertDB($conn,$ip,$username) {
          $ts=getTS();
          if( $conn->query("select * from list where ip='$ip' and user='$username'")->numRows() > 0 ){
                refreshDB($conn,$ip,$username);
      }
      else {
                $query = "insert into list (ip,timestamp,user) VALUES ";
                $query = $query . "('$ip', '$ts','$username')";
                $conn->queryExec($query);
          }
}
function refreshDB($conn,$ip,$username){
    $ts=getTS();
    $query = "update list set timestamp={$ts} ";
    $query = $query . "where ip='{$ip}' and user='{$username}'";
    if (0 == $conn->queryExec($query)) {
          die($query.";\n failed\n");
        }
}

function getIP(){
  if (isset($_SERVER["HTTP_X_FORWARDED_FOR"]))
        $ip = $_SERVER["HTTP_X_FORWARDED_FOR"];
  else $ip=$_SERVER['REMOTE_ADDR'];
  return $ip;
}

function check_user_policy($conn,$user,$destination,$dst_port,$prot){
        if( $conn->query("select * from user_policy where username='$user'
                                and dst_ip='$destination'
                                and prot='$prot'
                                and dst_port='$dst_port'")->numRows() > 0 ){
                return TRUE;
    }
    else {
        return FALSE;
    }
}

function getUser($conn,$ip){
        $results = $conn->query("select * from list where ip='{$ip}'");
    while ($row = $results->fetch()){
                return $row['user'];
    }
    return "failed";
}

function check_for_ip($conn,$ip){
  $query = "select * from list ";
  $query = $query . "where ip='{$ip}'";

  try {
     $results = $conn->query($query);
     while ($row = $results->fetch()){
        if (getTS() - $row['timestamp'] > 60)
                return FALSE;
        else
                return TRUE;
     }
  }
  catch (Exception $e){
        die($query.";\n failed\n");
        return FALSE;
  }
}
?>