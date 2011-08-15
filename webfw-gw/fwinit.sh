#!/bin/bash
#-----------------------------------------------------------------------
#       WebFW Netfilter Firewall Init Script
#       Author: 		Andreas Schmid
#		Last Changed:	24.05.2011
#       System: 		Debian Linux
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Basic Configuration
#-----------------------------------------------------------------------

OK=" \E[32m[OK]\E[0m"

# Interfaces

EXTIF="eth0"
EXTIP=""

AUTHIF="eth1"
AUTHIP="192.168.123.1"
AUTHMASK="255.255.255.0"
AUTHNET="$AUTHIP"/"$AUTHMASK"
AUTHSERVER="192.168.123.2"
AUTHPORTSTCP="80 443"

INTIF="eth2"
INTIP="10.0.0.1"
INTMASK="255.255.255.0"
INTNET="$INTIP"/"$INTMASK"

DMZIF="eth3"
DMZIP="192.168.0.1"
DMZMASK="255.255.255.0"
DMZNET="$INTIP"/"$INTMASK"

# Network Objects

echo -n "Initializing Firewall...."
DNSSERVER=`cat /etc/resolv.conf | grep '^nameserver' | sed 's/nameserver//'`
echo -e "$OK"

for DNS in $DNSSERVER
do
        echo "Found DNS SERVER: $DNS"
done

# Initialize Network
#------------------------
echo -n "Initializing Network interfaces..."
ifconfig "$AUTHIF" "$AUTHIP" netmask "$AUTHMASK" up
ifconfig "$INTIF" "$INTIP" netmask "$INTMASK" up
ifconfig "$DMZIF" "$DMZIP" netmask "$DMZMASK" up
echo -e "$OK"
# Define Spoofing Groups
#-------------------------

spoof_eth1="
	"192.168.123.0/24"
"

spoof_eth2="
	"10.0.0.0/24"
"

spoof_eth3="
	"192.168.0.0/24"
"

admin_clients="
	""
"

case "$1" in
'start')

#-----------------------------------------------------------------------
# Firewall Policy
#-----------------------------------------------------------------------

echo -n "Flushing INPUT, FORWARD and OUTPUT chains..."
# Delete existing Rules
iptables -F INPUT
iptables -F FORWARD
iptables -F OUTPUT
echo -e "$OK"

echo -n "Setting Policy to DROP"
# DROP any package
iptables -P INPUT DROP
iptables -P FORWARD DROP
iptables -P OUTPUT DROP
echo -e "$OK"

# Anti Spoofing Rules
#--------------------

echo -n "Setting anti-spoofing rules...."
for net in $spoof_eth1
do
	iptables -A INPUT ! -i eth1 -s "$net" -j LOG --log-prefix "Spoofing on eth1: "
	iptables -A INPUT ! -i eth1 -s "$net" -j DROP
	iptables -A FORWARD ! -i eth1 -s "$net" -j LOG --log-prefix "Spoofing on eth1: "
	iptables -A FORWARD ! -i eth1 -s "$net" -j DROP
done

for net in $spoof_eth2
do
	iptables -A INPUT ! -i eth2 -s "$net" -j LOG --log-prefix "Spoofing on eth2: "
	iptables -A INPUT ! -i eth2 -s "$net" -j DROP
	iptables -A FORWARD ! -i eth2 -s "$net" -j LOG --log-prefix "Spoofing on eth2: "
	iptables -A FORWARD ! -i eth2 -s "$net" -j DROP
done

for net in $spoof_eth3
do
	iptables -A INPUT ! -i eth3 -s "$net" -j LOG --log-prefix "Spoofing on eth3: "
	iptables -A INPUT ! -i eth3 -s "$net" -j DROP
	iptables -A FORWARD ! -i eth3 -s "$net" -j LOG --log-prefix "Spoofing on eth3: "
	iptables -A FORWARD ! -i eth3 -s "$net" -j DROP
done

# On local interface
iptables -A INPUT ! -i lo -s 127.0.0.1 -j LOG --log-prefix "Loopback interface spoofed: "
iptables -A INPUT ! -i lo -s 127.0.0.1 -j DROP
iptables -A FORWARD ! -i lo -s 127.0.0.1 -j LOG --log-prefix "Loopback interface spoofed: "
iptables -A FORWARD ! -i lo -s 127.0.0.1 -j DROP

# On external interface
iptables -A INPUT ! -i lo -s "$EXTIP" -j LOG --log-prefix "External interface spoofed: "
iptables -A INPUT ! -i lo -s "$EXTIP" -j DROP
iptables -A FORWARD ! -i lo -s "$EXTIP" -j LOG --log-prefix "External interface spoofed: "
iptables -A FORWARD ! -i lo -s "$EXTIP" -j DROP

echo -e "$OK"

# Reject Ident
#------------------
echo -n "Setting ant-ident rules...."
iptables -A INPUT -i "$EXTIF" -p tcp --dport 113 -j LOG --log-prefix "ident req: "
iptables -A INPUT -i "$EXTIF" -p tcp --dport 113 -j REJECT
echo -e "$OK"

# Drop NetBIOS packets early to avoid high load
#------------------------------------------------
echo -n "Setting anti-Netbios rules...."
iptables -A INPUT -p UDP -s 10.0.0.0/24 --sport 137:139 -j DROP
iptables -A INPUT -p UDP -s 10.0.0.0/24 --dport 137:139 -j DROP
iptables -A INPUT -p TCP -s 10.0.0.0/24 --sport 137:139 -j DROP
iptables -A INPUT -p TCP -s 10.0.0.0/24 --dport 137:139 -j DROP

iptables -A FORWARD -p UDP -s 10.0.0.0/24 --sport 137:139 -j DROP
iptables -A FORWARD -p UDP -s 10.0.0.0/24 --dport 137:139 -j DROP
iptables -A FORWARD -p TCP -s 10.0.0.0/24 --sport 137:139 -j DROP
iptables -A FORWARD -p TCP -s 10.0.0.0/24 --dport 137:139 -j DROP

echo -e "$OK"

# ICMP Rules
#------------
echo -n "Setting ICMP Rules rules...."
iptables -A INPUT -p icmp --icmp-type 0 -j ACCEPT
iptables -A INPUT -p icmp --icmp-type 3 -j ACCEPT
iptables -A INPUT -p icmp --icmp-type 8 -j ACCEPT
iptables -A INPUT -p icmp --icmp-type 11 -j ACCEPT
iptables -A INPUT -p icmp --icmp-type 12 -j ACCEPT
iptables -A OUTPUT -p icmp -j ACCEPT

echo -e "$OK"

# Statefull Filtering
#-------------------
echo -n "Setting rules for stateful filtering...."
iptables -A INPUT -m state --state ESTABLISHE,RELATED -j ACCEPT
iptables -A OUTPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
iptables -A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT

# local packets
#----------------

iptables -A INPUT -i lo -j ACCEPT
iptables -A OUTPUT -o lo -j ACCEPT

echo -e "$OK"

#-----------------------------------------------------------------
# Interface specific rules
#-----------------------------------------------------------------

iptables -N ext-in
iptables -N ext-fw
iptables -N ext-out

iptables -N int-in
iptables -N int-fw
iptables -N int-out

iptables -N dmz-in
iptables -N dmz-out
iptables -N ext2dmz
iptables -N dmz2ext
iptables -N dmz2int
iptables -N int2dmz

iptables -N auth-in
iptables -N auth-out
iptables -N ext2auth
iptables -N auth2ext

iptables -A INPUT -i "$INTIF" -s "$INTNET" -j int-in
iptables -A INPUT -i "$EXTIF" -j ext-in
iptables -A INPUT -i "$AUTHIF" -j auth-in
iptables -A INPUT -i "$DMZIF" -j dmz-in
iptables -A FORWARD -i "$INTIF" -o "$EXTIF" -s "$INTNET" -j int-fw
iptables -A FORWARD -i "$INTIF" -o "$AUTHIF" -s "$INTNET" -j int-fw
iptables -A FORWARD -i "$INTIF" -o "$DMZIF" -s "$INTNET" -j int2dmz
iptables -A FORWARD -i "$DMZIF" -o "$INTIF" -j dmz2int
iptables -A FORWARD -i "$EXTIF" -o "$INTIF" -j ext-fw
iptables -A FORWARD -i "$EXTIF" -o "$AUTHIF" -j ext2auth
iptables -A FORWARD -i "$AUTHIF" -o "$EXTIF" -j auth2ext
iptables -A FORWARD -i "$EXTIF" -o "$DMZIF" -j ext2dmz
iptables -A FORWARD -i "$DMZIF" -o "$EXTIF" -j dmz2ext
iptables -A OUTPUT -o "$INTIF" -j int-out
iptables -A OUTPUT -o "$EXTIF" -j ext-out
iptables -A OUTPUT -o "$AUTHIF" -j auth-out
iptables -A OUTPUT -o "$DMZIF" -j dmz-out

# Incoming Rules
#-----------------
echo -n "Setting Management rules...."
#Management Rules
for host in $admin_clients
do
	#SSH to Gateway
	iptables -A ext-in -p tcp -s "$host" -d "$EXTIP" --dport 22 -j ACCEPT
	iptables -A ext2auth -p tcp -s "$host" -d "$AUTHSERVER" --dport 8443 -j ACCEPT
done
echo -e "$OK"

# Outgoing Rules
#-------------------
echo -n "Setting rules for Outgoing Connections...."
# communication betwen gateway and Webserver
iptables -A auth-out -p tcp --sport 1024:65535 --dport 22 -j ACCEPT
iptables -A ext-out -p tcp --sport 1024:65535 --dport 22 -j ACCEPT
iptables -A auth-out -p tcp --sport 1024:65535 --dport 443 -j ACCEPT

# apt-get install ... for auth Server and Gateway
iptables -A auth2ext -p tcp -s 192.168.123.2 --dport 80 -j ACCEPT
iptables -A ext-out -p tcp -s "$EXTIP" --dport 80 -j ACCEPT
iptables -A dmz2ext -p tcp -s 192.168.0.5 --dport 80 -j ACCEPT
# DNS Servers
#---------------------

for DNS in $DNSSERVER
do
        iptables -A ext-out -p udp -d "$DNS" --dport 53 --sport 1024:65535 -j ACCEPT
        iptables -A ext-out -p tcp -d "$DNS" --dport 53 --sport 1024:65535 -j ACCEPT
done

# Bind9 DNS Server
iptables -A auth-in -p udp --sport 1024:65535 --dport 53 -j ACCEPT
iptables -A int-in -p udp --sport 1024:65535 --dport 53 -j ACCEPT
iptables -A dmz-in -p udp --sport 1024:65535 --dport 53 -j ACCEPT

#---------------------------
# Policy for Auth Server
#---------------------------

# Allow Server Ports
#--------------------
echo -n "Setting rules for incoming Connections...."
for p in $AUTHPORTSTCP
do
	iptables -A ext2auth -p tcp -d "$AUTHSERVER" --dport "$p" -j ACCEPT
done
echo -e "$OK"

# WebFW Queue Rule
#------------------------
iptables -A ext-in -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "Sent to WebFW: "
iptables -A ext-in -s 0.0.0.0/0 -j NFQUEUE --queue-num 0
iptables -A ext-fw -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "Sent to WebFW: "
iptables -A ext-fw -s 0.0.0.0/0 -j NFQUEUE --queue-num 0
iptables -A ext2dmz -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "Sent to WebFW: "
iptables -A ext2dmz -s 0.0.0.0/0 -j NFQUEUE --queue-num 0

# Log unwanted packets
#-----------------------
echo -n "Setting Clean-UP and LOG rules...."
iptables -A INPUT -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "INPUT block: "
iptables -A INPUT -s 0.0.0.0/0 -j DROP
iptables -A OUTPUT -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "OUTPUT block: "
iptables -A OUTPUT -s 0.0.0.0/0 -j DROP
iptables -A FORWARD -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "FORWARD block: "
iptables -A FORWARD -s 0.0.0.0/0 -j DROP
iptables -A int-in -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "int-in block: "
iptables -A int-in -s 0.0.0.0/0 -j DROP
iptables -A int-out -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "int-out block: "
iptables -A int-out -s 0.0.0.0/0 -j DROP
iptables -A int-fw -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "int-fw block: "
iptables -A int-fw -s 0.0.0.0/0 -j DROP
iptables -A ext-in -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "ext-in block: "
iptables -A ext-in -s 0.0.0.0/0 -j DROP
iptables -A ext-out -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "ext-out block: "
iptables -A ext-out -s 0.0.0.0/0 -j DROP
iptables -A ext-fw -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "ext-fw block: "
iptables -A ext-fw -s 0.0.0.0/0 -j DROP
iptables -A auth-in -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "auth-in block: "
iptables -A auth-in -s 0.0.0.0/0 -j DROP
iptables -A auth-out -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "auth-out block: "
iptables -A auth-out -s 0.0.0.0/0 -j DROP
iptables -A ext2auth -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "ext2auth block: "
iptables -A ext2auth -s 0.0.0.0/0 -j DROP
iptables -A auth2ext -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "auth2ext block: "
iptables -A auth2ext -s 0.0.0.0/0 -j DROP
iptables -A dmz-in -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "dmz-in block: "
iptables -A dmz-in -s 0.0.0.0/0 -j DROP
iptables -A dmz-out -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "dmz-out block: "
iptables -A dmz-out -s 0.0.0.0/0 -j DROP
iptables -A ext2dmz -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "ext2dmz block: "
iptables -A ext2dmz -s 0.0.0.0/0 -j DROP
iptables -A dmz2ext -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "dmz2ext block: "
iptables -A dmz2ext -s 0.0.0.0/0 -j DROP
iptables -A dmz2int -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "dmz2int block: "
iptables -A dmz2int -s 0.0.0.0/0 -j DROP
iptables -A int2dmz -s 0.0.0.0/0 -j LOG --log-level notice --log-prefix "int2dmz block: "
iptables -A int2dmz -s 0.0.0.0/0 -j DROP
echo -e "$OK"

# Network Adress Translation
#-------------------------------
echo -n "Setting NAT rules...."
iptables -t nat -A POSTROUTING -o "$EXTIF" -j MASQUERADE
iptables -t nat -A POSTROUTING -s "$INTNET" -o "$AUTHIF" -j MASQUERADE
iptables -t nat -A PREROUTING -p tcp -i "$EXTIF" --dport 443 -j DNAT --to "$AUTHSERVER" #Webinterface SSL
iptables -t nat -A PREROUTING -p tcp -i "$EXTIF" --dport 21 -j DNAT --to 192.168.0.5 #ftp Server
iptables -t nat -A PREROUTING -p tcp -i "$EXTIF" --dport 8443 -j DNAT --to "$AUTHSERVER"
iptables -t nat -A PREROUTING -p tcp -i "$EXTIF" --dport 3389 -j DNAT --to 10.10.0.5


echo -e "$OK"
;;

'stop')
#-----------------------------------------------------------------------
# Stop Firewall
#-----------------------------------------------------------------------
echo "Stopping Firewall..."

echo -n "Setting Policy to Accept"
# DROP any package
iptables -P INPUT ACCEPT
iptables -P FORWARD ACCEPT
iptables -P OUTPUT ACCEPT
echo -e "$OK"

echo -n "Releasing Chains..."
iptables -F
iptables -X
iptables -t nat -F
iptables -t nat -X
iptables -t mangle -F
iptables -t mangle -X
echo -e "$OK"
echo "Goodbye"
;;
esac
