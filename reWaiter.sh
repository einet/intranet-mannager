#!/bin/bash
netstat -nlp | grep :8099 | awk '{print $7}' | awk -F"/" '{ print $1 }' |xargs kill -9
sleep 1s
netstat -nlp | grep :8099 | awk '{print $7}' | awk -F"/" '{ print $1 }' |xargs kill -9
sleep 5s
/home/zlx/NLPNEW/Waiter/Waiter 0.0.0.0 224.0.0.18 8099 32000 /home/zlx/NLPNEW/Waiter/page/

