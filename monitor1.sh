#!/bin/bash
export monitordir="/home/monitor"
startup()
{
        cd ${monitordir}
        ./start.sh $1
}
inotifywait -mrq --timefmt '%d/%m/%y %H:%M' --format  '%T %w%f %e' --event create  /home/monitor/page/update | while read  date time file event
  do
      case $event in
          CREATE)
                 # echo $event'-'$file;

                if [[ $event == "CREATE"  &&  $file == '/home/monitor/page/update/start.txt' ]];
                        then
                        echo "recv start.txt"
                                startup "1"
				rm -f /home/monitor/page/update/start.txt
                fi
if [[ $event == "CREATE" &&  $file == '/home/dlstest/share/start.txt' ]];
                        then
                        echo "recv start.txt"
                                startup "1"
                fi
                if [[ $event == "CREATE" &&  $file == '/home/dlstest/share/end.txt' ]];
                        then
                        echo "recv action.txt"
                                startup "0"
                fi

              ;;
      esac
  done

