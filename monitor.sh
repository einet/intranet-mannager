 #!/bin/bash
 startup()
{
        cd /home/monitor
        ./start.sh $1
}
  inotifywait -mrq --timefmt '%d/%m/%y %H:%M' --format  '%T %w%f %e' --event create /home/monitor/page/update | while read  date time file event
  do
      case $event in
          CREATE)
                 # echo $event'-'$file;
                if [[ $event == "CREATE" &&  $file == '/home/monitor/page/update/start.txt' ]];
                        then
                        echo "recv start.txt"
                                startup "1"
				rm -rf /home/monitor/page/update/start.txt	
                fi
              ;;

      esac
  done
