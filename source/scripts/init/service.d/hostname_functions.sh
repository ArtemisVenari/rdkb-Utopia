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

HOSTS_FILE=/etc/hosts
HOSTNAME_FILE=/etc/hostname
source /etc/device.properties


prepare_lanhost_hostname()
{
   LANHOST_COUNT=`dmcli eRT getv Device.Hosts.HostNumberOfEntries |  grep -w value: | awk '{print$5}' | sed 's/^ *//g'`
   count=1
   reload=0
   while [ $count -le $LANHOST_COUNT ]
   do
      LANHOST_IPv6_LLA=`dmcli eRT getv Device.Hosts.Host.$count.IPv6Address.2.IPAddress |  grep -w value: | awk '{print$5}' | sed 's/^ *//g'`                               
      if [ "$LANHOST_IPv6_LLA" != "" ]; then
         LANHOST_HOSTNAME=`dmcli eRT getv Device.Hosts.Host.$count.HostName |  grep -w value: | awk '{print$5}' | sed 's/^ *//g'` 
         if [ "$LANHOST_HOSTNAME" != "" ]; then
            LANHOST_PHYADDRESS=`dmcli eRT getv Device.Hosts.Host.$count.PhysAddress |  grep -w value: | awk '{print$5}' | sed 's/^ *//g'` 
            if [ "$LANHOST_HOSTNAME" != "$LANHOST_PHYADDRESS" ]; then
               LANHOST_HOSTNAME_APPEND=`cat $HOSTS_FILE | grep -wn "$LANHOST_HOSTNAME" | cut -d: -f1`
               LANHOST_IPv6_APPEND=`cat $HOSTS_FILE | grep -wn "$LANHOST_IPv6_LLA" | cut -d: -f1`
               if [ "$LANHOST_HOSTNAME_APPEND" == "" ]  && [ "$LANHOST_IPv6_APPEND" == "" ] ; then  
                  echo "$LANHOST_IPv6_LLA   $LANHOST_HOSTNAME" >> $HOSTS_FILE
                  reload=1
               fi  
            fi   
         fi
      fi    
      count=$((count + 1))
   done
   if [ "$reload" -eq 1 ]; then
      kill -HUP `cat /var/run/dnsmasq.pid`   
   fi  
}

#-----------------------------------------------------------------
# set the hostname files
#-----------------------------------------------------------------
prepare_hostname () {
   HOSTNAME=`syscfg get hostname`
   LAN_IPADDR=`sysevent get current_lan_ipaddr`
   SYSEVT_lan_ipaddr_v6=`sysevent get lan_ipaddr_v6`
   LOCDOMAIN_NAME=`syscfg get SecureWebUI_LocalFqdn`
   SECUREWEBUI_ENABLED=`syscfg get SecureWebUI_Enable`
   IPV6_BRLAN0_ADDRESS=`sysevent get lan_ipaddr_v6`
   lan_ipaddr_v6_lla=`syscfg get LLA_default_value`
   partnerID=`syscfg get PartnerID`

   if [ "" != "$HOSTNAME" ] ; then
      if [ "$MODEL_NUM" == "PX5001B" ] && [ "$SECUREWEBUI_ENABLED" == "true" ]; then
          if [[ $HOSTNAME != *-bci* ]] ; then
              HOSTNAME=$HOSTNAME"-bci"
              syscfg set hostname $HOSTNAME
              syscfg commit
          fi
      fi
      echo "$HOSTNAME" > $HOSTNAME_FILE
      hostname $HOSTNAME
   fi
       
   if [ "" != "$HOSTNAME" ] ; then
      echo "$LAN_IPADDR     $HOSTNAME" > $HOSTS_FILE
      echo "$lan_ipaddr_v6_lla  $HOSTNAME" >> $HOSTS_FILE
      if [ "$partnerID" != "telekom-de-test" ] \
      && [ "$partnerID" != "telekom-dev-de" ] \
      && [ "$partnerID" != "telekom-de" ]; then
          if [ "$IPV6_BRLAN0_ADDRESS" != "" ]; then
              echo "$IPV6_BRLAN0_ADDRESS  $HOSTNAME" >> $HOSTS_FILE
          fi
      fi
   else
      echo -n > $HOSTS_FILE
   fi
       
   echo "127.0.0.1       localhost" >> $HOSTS_FILE
   echo "::1             localhost" >> $HOSTS_FILE
   if [ "$SECUREWEBUI_ENABLED" = "true" ]; then
       if [ ! -z $LOCDOMAIN_NAME ]; then
           if [ ! -z $LAN_IPADDR ]; then
               echo "$LAN_IPADDR""         ""$LOCDOMAIN_NAME"  >> $HOSTS_FILE
           fi
           if [ ! -z $SYSEVT_lan_ipaddr_v6 ]; then
               echo "$SYSEVT_lan_ipaddr_v6""         ""$LOCDOMAIN_NAME"  >> $HOSTS_FILE
           fi
       fi
   fi

   # The following lines are desirable for IPv6 capable hosts
   echo "::1             ip6-localhost ip6-loopback" >> $HOSTS_FILE
   echo "fe00::0         ip6-localnet" >> $HOSTS_FILE
   echo "ff00::0         ip6-mcastprefix" >> $HOSTS_FILE
   echo "ff02::1         ip6-allnodes" >> $HOSTS_FILE
   echo "ff02::2         ip6-allrouters" >> $HOSTS_FILE
   echo "ff02::3         ip6-allhosts" >> $HOSTS_FILE
   
   prepare_lanhost_hostname
}

