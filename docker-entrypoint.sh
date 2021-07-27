#!/bin/sh
set -e

if [ "$1" = '/ucentral/ucentralsec' -a "$(id -u)" = '0' ]; then
    chown -R "$UCENTRALSEC_USER": "$UCENTRALSEC_ROOT" "$UCENTRALSEC_CONFIG"
    exec su-exec "$UCENTRALSEC_USER" "$@"
fi

exec "$@"
