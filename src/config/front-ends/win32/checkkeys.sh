fgrep MKV_ /ardi/executor/src/include/rsys/keyboard.h | awk '{print $2}' |
while read key
do isthere=`fgrep $key vk_to_mkv.h`
   if [ "$isthere" = "" ]; then
	echo $key
   fi
done
