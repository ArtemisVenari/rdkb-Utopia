#!/bin/sh
##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2015 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

#######################################################################
#   Copyright [2014] [Cisco Systems, Inc.]
# 
#   Licensed under the Apache License, Version 2.0 (the \"License\");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an \"AS IS\" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#######################################################################

#------------------------------------------------------------------
# This script is used to start ssh daemon
# $1 is the calling event (sshd-restart, lan-status, wan-status, etc)
#------------------------------------------------------------------

source /etc/utopia/service.d/ulog_functions.sh
source /etc/utopia/service.d/log_capture_path.sh
source /etc/device.properties

if [ "$BOX_TYPE" = "HUB4" ] || [ "$BOX_TYPE" = "SR300" ]; then
   CMINTERFACE="erouter0"
elif ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ]); then
   CMINTERFACE="erouter0"
else
   if [ -f "/nvram/ETHWAN_ENABLE" ];then
   	CMINTERFACE="erouter0"
   else
   	CMINTERFACE="wan0"
   fi
fi

pwd=`cat /etc/shadow | grep root | cut -d: -f2`
[ -z $pwd ] && echo -e "DTelek0m\nDTelek0m" | passwd root

SSHPORT=`syscfg get mgmt_wan_sshport`
    
SERVICE_NAME="sshd"
SELF_NAME="`basename $0`"

PID_FILE=/var/run/dropbear
if [ -f /etc/mount-utils/getConfigFile.sh ];then
      mkdir -p /tmp/.dropbear
     . /etc/mount-utils/getConfigFile.sh
fi

#Determine which IP addresses on wan0 to listen on (ipv4 and ipv6 if present)
get_listen_params() {
    LISTEN_PARAMS=""
    #Get IPv4 address of wan0
    CM_IP4=`ip -4 addr show dev wan0 scope global | awk '/inet/{print $2}' | cut -d '/' -f1`
    #Get IPv6 address of wan0
    CM_IP6=`ip -6 addr show dev wan0 scope global | awk '/inet/{print $2}' | cut -d '/' -f1`
    if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ]); then
        CM_IP4=`ip -4 addr show dev $CMINTERFACE scope global | awk '/inet/{print $2}' | cut -d '/' -f1`
    fi
    if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ]); then
        CM_IP6=`ip -6 addr show dev $CMINTERFACE scope global | awk '/inet/{print $2}' | cut -d '/' -f1`
    fi
    if [ "$CM_IP4" != "" ] ; then
        LISTEN_PARAMS="-p [${CM_IP4}]:22"
    fi
    if [ "$CM_IP6" != "" ] ; then
        LISTEN_PARAMS="$LISTEN_PARAMS -p [${CM_IP6}]:22"
    fi
    #If there is no ipv4 or ipv6 address to listen on, bind to local address only
    if [ "$LISTEN_PARAMS" = "" ] ; then
        LISTEN_PARAMS="-p [127.0.0.1]:22"
        if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ]); then
            echo_t "utopia: dropbear not started with valid erouter0 IPv4 or IPv6 address $LISTEN_PARAMS"
        fi
    fi
}

do_start() {
   #DIR_NAME=/tmp/home/admin
   #if [ ! -d $DIR_NAME ] ; then
      # in order to use user admin for ssh we need to give it a home directory
      # echo "[utopia] Creating ssh user admin" > /dev/console
      #mkdir -p $DIR_NAME
      #chown admin $DIR_NAME
      #chgrp admin $DIR_NAME
      #chmod 755 $DIR_NAME
   #fi

    if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ] || [ "$MODEL_NUM" = "INTEL_PUMA" ]) ;then
    	get_listen_params
            CMINTERFACE="erouter0"
    fi

    CM_IP=""
    if ([ "$BOX_TYPE" = "rpi" ]) ;then
        #for Raspberry-pi, use the ipv4 address as default for ssh
        CM_IP=`ifconfig ${CMINTERFACE} | grep "inet addr" | awk '/inet/{print $2}'  | cut -f2 -d:`
    else
        #for other devices, use the ipv6 address for ssh, if available
        CM_IP=`ifconfig ${CMINTERFACE} | grep inet6 | grep Global | awk '/inet6/{print $3}' | cut -d '/' -f1`
    fi

   # start a ssh daemon
   # echo "[utopia] Starting SSH daemon" > /dev/console
#   dropbear -d /etc/dropbear_dss_host_key  -r /etc/dropbear_rsa_host_key
#   /etc/init.d/dropbear start
   #dropbear -r /etc/rsa_key.priv
   #dropbear -E -s -b /etc/sshbanner.txt -s -a -p [$CM_IP]:22

   #wan0 should be in v4
   CM_IP="${CM_IP} `ifconfig ${CMINTERFACE} | grep "inet addr" | awk '/inet/{print $2}'  | cut -f2 -d:`"
   #LAN side
   #LAN Side SSH blocked
   #LANINTERFACE="brlan0"
   #CM_IP="${CM_IP} `ifconfig ${LANINTERFACE} | grep inet6 | grep Global | awk '/inet6/{print $3}' | cut -d '/' -f1`"
   #CM_IP="${CM_IP} `ifconfig ${LANINTERFACE} | grep "inet addr" | awk '/inet/{print $2}'  | cut -f2 -d:`"

   for i in $CM_IP; do
   DROPBEAR_PARAMS_1="/tmp/.dropbear/dropcfg1$$"
   DROPBEAR_PARAMS_2="/tmp/.dropbear/dropcfg2$$"
   getConfigFile $DROPBEAR_PARAMS_1
   getConfigFile $DROPBEAR_PARAMS_2

   if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ] || [ "$MODEL_NUM" = "INTEL_PUMA" ]) ;then
   	dropbear -E -s -b /etc/sshbanner.txt -a -r $DROPBEAR_PARAMS_1 -r $DROPBEAR_PARAMS_2 $LISTEN_PARAMS -P $PID_FILE 2>/dev/null
    if [ "$LISTEN_PARAMS" = "" ] ; then
        echo_t "[utopia]: dropbear was not started for erouter0 interface with valid params."
    fi
    CM_IP4=`ip -4 addr show dev $CMINTERFACE scope global | awk '/inet/{print $2}' | cut -d '/' -f1`
    if [ -n "$CM_IP4" ]; then
      echo_t "[utopia]: dropbear was started on erouter0 IPv4 $CM_IP4 interface."
    else
      echo_t "utopia: dropbear could not be started on erouter0 IPv4 interface."
    fi
    CM_IP6=`ip -6 addr show dev $CMINTERFACE scope global | awk '/inet/{print $2}' | cut -d '/' -f1`
    if [ -n "$CM_IP6" ]; then
      echo_t "[utopia]: dropbear was started on erouter0 IPv6 $CM_IP6 interface."
    else
      echo_t "utopia: dropbear could not be started on erouter0 IPv6 interface."
    fi
   else
        dropbear -R -E -a -r $DROPBEAR_PARAMS_1 -r $DROPBEAR_PARAMS_2 -p [$i]:$SSHPORT -P ${PID_FILE}_${i}.pid -B
   fi

   # The PID_FILE created after demonize the process. So added delay for 1 sec.
   sleep 1
   if [ ! -f "${PID_FILE}_${i}.pid" ] ; then
      echo_t "[utopia] ${PID_FILE}_${i}.pid file is not created"
      #Create the pid file in case if it not created by dropbear
      ps | grep 'dropbear -E -s -b /etc/sshbanner.txt' | head -n1 |  awk '{print $1;}' > $PID_FILE
      echo_t "[utopia] $PID_FILE file is created explicitly"
   else
      echo_t "[utopia] $PID_FILE file is created. PID : `cat ${PID_FILE}_${i}.pid`"
   fi
   done
   sysevent set ssh_daemon_state up
}

do_stop() {
   # echo "[utopia] Stopping SSH daemon" > /dev/console
   sysevent set ssh_daemon_state down
#   kill -9 dropbear
   if [ ! -f "$PID_FILE" ] ; then
     tmp_filename="/tmp/dropbear.pid"
     #Get the dropbear IPV6 process IDs. There could be more than 1 dropbear IPV6 process running based on number of open ssh connections
     ps | grep 'dropbear -E -s -b /etc/sshbanner.txt' | sed '/grep/d' > $tmp_filename
     while read -r line; do
       pid=`echo $line | head -n1 |  awk '{print $1;}'`
       kill -9 $pid
       done < "$tmp_filename"
     rm $tmp_filename  
   else
     kill -9 `cat $PID_FILE*`
     rm -f $PID_FILE*
     pidlist=`pidof dropbear`
     for i in $pidlist; do
       kill -9 $i
     done
   fi
#    /etc/init.d/dropbear stop
}

service_start() {

    echo_t "[utopia] starting ${SERVICE_NAME} service"
	ulog ${SERVICE_NAME} status "starting ${SERVICE_NAME} service"

   if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ] || [ "$MODEL_NUM" = "INTEL_PUMA" ]) ;then
      CMINTERFACE=erouter0
      ifconfig $CMINTERFACE | grep Global
      ret=$?
      while [ $ret -ne 0 ]; do
        sleep 20
        ifconfig $CMINTERFACE | grep Global
        ret=$?
        if [ $? -eq 0 ] ; then
            echo_t "[utopia] erouter0 interface got an address for ${SERVICE_NAME} service."
            echo_t "[utopia] erouter0 interface is online"
            break
        fi
      done
   fi
	SSH_ENABLE=`syscfg get mgmt_wan_sshaccess`
#	CURRENT_WAN_STATE=`sysevent get wan-status`

        if [ "$SSH_ENABLE" = "1" ]; then

   if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ] || [ "$MODEL_NUM" = "INTEL_PUMA" ]) ;then
        if [ -n "$CURRENT_WAN_STATE" -a "started" = "$CURRENT_WAN_STATE" ]; then
            rm -f $PID_FILE 2>/dev/null
            do_start
        fi
   else
        PID_LIST=`cat $PID_FILE* | wc -l`
               if [ "$PID_LIST" -eq 0 ] ; then
			#while [ "started" != "$CURRENT_WAN_STATE" ]
			#do
				#sleep 1
				#CURRENT_WAN_STATE=`sysevent get wan-status`
			#done

	    	do_start
		fi
   fi
		$PMON setproc ssh dropbear $PID_FILE "/etc/utopia/service.d/service_sshd.sh sshd-restart"

		sysevent set ${SERVICE_NAME}-errinfo
		sysevent set ${SERVICE_NAME}-status "started"
		rm -rf /tmp/.dropbear/*

	fi

}

service_stop () {
   echo_t "[utopia] stopping ${SERVICE_NAME} service"
#   ulog ${SERVICE_NAME} status "stopping ${SERVICE_NAME} service" 

   PID_LIST=`cat $PID_FILE* | wc -l`
   if [ "$PID_LIST" -gt 0 ] ; then
   do_stop
   fi

   sysevent set ${SERVICE_NAME}-errinfo
   sysevent set ${SERVICE_NAME}-status "stopped"
}

#service_lanwan_status ()
#{
      #CURRENT_LAN_STATE=`sysevent get lan-status`
      #CURRENT_WAN_STATE=`sysevent get wan-status`
      #if [ "stopped" = "$CURRENT_LAN_STATE" ] && [ "stopped" == "$CURRENT_WAN_STATE" ] ; then
         #service_stop
      #else
         #service_start
      #fi
#}

service_wan_status ()
{
      CURRENT_WAN_STATE=`sysevent get wan-status`
      #if [ "stopped" = "$CURRENT_WAN_STATE" ] ; then
       #  service_stop
      #else
      if [ "started" = "$CURRENT_WAN_STATE" ] ; then
         service_stop
         service_start
      fi 
}

service_bridge_status ()
{
      CURRENT_BRIDGE_STATE=`sysevent get bridge-status`
      if [ "stopped" = "$CURRENT_BRIDGE_STATE" ] ; then
         service_stop
         service_start
      elif [ "started" = "$CURRENT_BRIDGE_STATE" ] ; then
         service_stop
         service_start
      fi
}

# Entry

echo_t "[utopia] ${SERVICE_NAME} $1 received"

CURRENT_WAN_STATUS=`sysevent get wan-status`
case "$1" in
  ${SERVICE_NAME}-start)
      service_start
      ;;
  ${SERVICE_NAME}-stop)
      service_stop
      ;;
  ${SERVICE_NAME}-restart)
      service_stop
      service_start
      ;;
   # lan-status)
      #service_lanwan_status
      #;;
 wan-status)
   if ([ "$BOX_TYPE" = "XB6" -a "$MANUFACTURE" = "Arris" ]) ;then
      echo_t "utopia: Need to handle the wan-status event."
      if [ -n "$CURRENT_WAN_STATUS" ]; then
        if [ "started" = "$CURRENT_WAN_STATUS" ]; then
            service_stop
            service_start
        fi
      fi
   fi
    ;;
 #     service_wan_status
 #    ;;
  bridge-status)
      service_bridge_status
      ;;
  *)
        echo "Usage: $SELF_NAME [${SERVICE_NAME}-start|${SERVICE_NAME}-stop|${SERVICE_NAME}-restart|ssh_server_restart|lan-status|wan-status]" >&2
        exit 3
        ;;
esac

