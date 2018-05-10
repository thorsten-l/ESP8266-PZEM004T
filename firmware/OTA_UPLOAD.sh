#!/bin/sh
$HOME/.platformio/penv/bin/python \
  $HOME/.platformio/packages/tool-espotapy/espota.py \
  --debug --progress -i "power-meter-1.local" \
  --auth="otapass" -f "firmware-1.0.0.bin"
