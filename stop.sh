#!/usr/bin/env bash

pidfilename="${UCENTRALSEC_ROOT}/data/pidfile"

if [[ -f "${pidfilename}" ]]
then
  kill -9 $(cat ${pidfilename})
fi
