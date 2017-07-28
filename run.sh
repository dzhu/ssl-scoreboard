#!/bin/bash
./build.sh

killall rcon_forward scoreboard
#bin/rcon_forward &
bin/scoreboard -a 2
