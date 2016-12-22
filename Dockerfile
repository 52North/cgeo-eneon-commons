FROM drupal:7.53-apache
MAINTAINER Henning Bredel <h.bredel@52north.org>

# Install xdebug to drupal instance
# Installation intructions from https://xdebug.org/wizard.php


RUN apt-get update -qq && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -qq -y apt-utils
RUN DEBIAN_FRONTEND=noninteractive apt-get install -qq -y drush wget mysql-client \
	## Install needed drush dependencies
	&& pear install --alldeps Console_table \
	## Install xDebug
	&& wget --quiet http://xdebug.org/files/xdebug-2.4.1.tgz -O /tmp/xdebug.tgz \
	&& tar xf /tmp/xdebug.tgz -C /tmp \
	&& cd /tmp/xdebug-2.4.1 \
	&& phpize > /dev/null \
	&& ./configure > /dev/null \
	&& make > /dev/null \
	&& cp modules/xdebug.so /usr/local/lib/php/extensions/no-debug-non-zts-20151012/ \
	&& echo "zend_extension = /usr/local/lib/php/extensions/no-debug-non-zts-20151012/xdebug.so" >> /usr/local/etc/php/php.ini \
	&& docker-php-ext-install sockets

#
# Configure xdebug
#
RUN echo "" >> /usr/local/etc/php/php.ini \
	&& echo "xdebug.remote_enable = 1" >> /usr/local/etc/php/php.ini \ 
	&& echo "xdebug.remote_connect_back = 1" >> /usr/local/etc/php/php.ini \
	&& echo "xdebug.remote_handler = dbgp" >> /usr/local/etc/php/php.ini \
	&& echo "xdebug.remote_port = 9000" >> /usr/local/etc/php/php.ini \
	&& echo "xdebug.remote_host = 172.17.0.1" >> /usr/local/etc/php/php.ini \
	&& echo "xdebug.scream = 0" >> /usr/local/etc/php/php.ini \
	&& echo "xdebug.show_local_vars = 1" >> /usr/local/etc/php/php.ini \
	&& echo "xdebug.idekey = ECLIPSE_DBGP" >> /usr/local/etc/php/php.ini

COPY config/launch.sh /usr/local/bin/startup

RUN chmod +x /usr/local/bin/startup

# run w/o json array: http://stackoverflow.com/a/27615958/2299448
# prefix script w/ bash: https://github.com/docker/compose/issues/3065#issuecomment-191489901
CMD bash startup
