#!/bin/bash
# Copyright (C) 2016
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
# 
# If the program is linked with libraries which are licensed under one of
# the following licenses, the combination of the program with the linked
# library is not considered a "derivative work" of the program:
# 
#     - Apache License, version 2.0
#     - Apache Software License, version 1.0
#     - GNU Lesser General Public License, version 3
#     - Mozilla Public License, versions 1.0, 1.1 and 2.0
#     - Common Development and Distribution License (CDDL), version 1.0
# 
# Therefore the distribution of the program linked with libraries licensed
# under the aforementioned licenses, is permitted by the copyright holders
# if the distribution is compliant with both the GNU General Public
# License version 2 and the aforementioned licenses.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
echo -e "\nWaiting for mysql\n"
#sleep 6
until mysql -h eneon_db -P 3306 -u root -pmysql &> /dev/null
do
  printf "."
  sleep 1
done

echo -e "\nmysql is ready\n"
#
# check for settings php and print error message if not found => requires
# initial configuration
#
if [ ! -e "/var/www/html/sites/default/settings.php" ]
then
  echo -e "[!]\n[!]\n[!] Please perform the initial installation of drupal and restart the container\n[!]\n[!]"
else
  #
  # install required dependencies   
  #
  echo -e "download required drupal exensions\n"
  modules="entity \
	entityreference \
	views \
	devel \
	ctools \
	module_filter \
	imagecache_external \
	link \
	" 
  
  drush dl "$modules skeletontheme"
  drush -y en $modules
  # enable own modules after dependencies are active 
  drush -y en eneon skeletontheme eneon_theme
  drush -y dis overlay
  drush vset theme_debug 1
  drush vset theme_default eneon_theme
  drush cc all
  drush cron
  touch /var/www/html/sites/default/files/eneon_networks.json
  chown www-data.www-data /var/www/html/sites/default/files/eneon_networks.json
fi
#
# update host ip for apache xdebug allowed ips
#
IP=$(/sbin/ip route|awk '/default/ { print $3 }')
echo -e "\n\nAdding Docker Host IP: '$IP' to '/usr/local/etc/php/php.ini'\n"
sed -i "s/172.17.0.1/$IP/g" /usr/local/etc/php/php.ini
echo -e "> $(cat /usr/local/etc/php/php.ini | grep -n host)\n\n"
#
# finally launch apache
#
/usr/local/bin/apache2-foreground
