#!/bin/bash

#定位到当前shell的目录
basedir=$(cd "$(dirname "$0")"; pwd)
cd $basedir;

#定义要删除的文件父级目录
logdir="/home/dlstest/apache-tomcat-7.0.37/logs";

#定义保留的发布版本数
saveDataDays=5;

#获取指定日期以前的目录
function getDellog() {
  #进入删除文件目录
  cd ${logdir};
  #逐行列出所有的文件夹
  #筛选出名字是【8个数字】组成的文件夹
  #倒序输出文件夹
  dirline=`ls -d */ -1 | grep "^[0-9]\{8,8\}/" | sort -r`;

  dircount=0;
  delDirArray=();
  for LINE in ${dirline}
  do
    let "dircount=dircount+1";
    #判断是否超过30天
    if [ ${dircount} -gt ${saveDataDays} ];then
      #保存需要删除的文件夹
      delDirArray[${#delDirArray[@]}]=$LINE;
    fi
  done
  #回到shell目录
  cd $basedir;
}
getDellog;
#echo ${delDirArray[*]};

#删除指定日期前的文件夹
echo `date "+ %Y/%m/%d %H:%M:%S %A"`" start del"
#进入删除文件目录
cd ${logdir};
echo "cd ${logdir}";
for singledir in ${delDirArray[@]}
do
  #删除文件夹
  if [ ! -f "$singledir" ];then
    echo "del ./$singledir";
    rm -rf "./"$singledir;
  fi
done
#回到shell目录
cd $basedir;
#正常结束
echo `date "+ %Y/%m/%d %H:%M:%S %A"`" end del"
exit 0;
