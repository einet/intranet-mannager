#!/bin/bash
opt=$1
echo "Monitor $opt"
echo "172.16.10.11:" `/home/monitor/check_mp -H 172.16.10.111 -t $opt`
echo "172.16.10.12:" `/home/monitor/check_mp -H 172.16.10.112 -t $opt`
echo "172.16.10.14:" `/home/monitor/check_mp -H 172.16.10.113 -t $opt`
echo "172.16.10.14:" `/home/monitor/check_mp -H 172.16.10.114 -t $opt`
echo "172.16.10.15:" `/home/monitor/check_mp -H 172.16.10.115 -t $opt`
echo "172.16.10.16:" `/home/monitor/check_mp -H 172.16.10.116 -t $opt`
echo "172.16.10.17:" `/home/monitor/check_mp -H 172.16.10.117 -t $opt`
echo "172.16.10.18:" `/home/monitor/check_mp -H 172.16.10.118 -t $opt`


