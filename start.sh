#!/bin/bash
export monitordir="/home/monitor"
action=$1
action=${action##*/}


kill_process()
{
NAME=$1
echo $NAME
ID=`ps -ef | grep "$NAME" | grep -v "$0" | grep -v "grep" | awk '{print $2}'`
echo $ID
echo "---------------"
for id in $ID
do
kill -9 $id
while [[ $? != 0 ]];
do
echo "killed $id"
kill -9 $id
done
echo "killed $id"
done
echo "---------------"
}
copyfile()
{
   cd "${monitordir}"
   if([ -f "Waiter1" ]);
   then
     echo "Copy"
     cp -f  "Waiter1" "Waiter"
   else
     echo "Waiter1 No exist"
   fi
   chmod +x "Waiter"
}
start_waiter()
{
echo "startup Waiter"
fuser -k -n tcp 8099
cd "${monitordir}"
./Waiter 0.0.0.0 224.0.0.18 8099 32000 ./page
}
kill_process "Waiter"
copyfile
start_waiter
