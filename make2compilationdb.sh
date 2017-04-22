#!/bin/sh
O=compile_commands.json
LANG=C
IFS_BK=${IFS}
IFS="
"
C=($(make -Bprn | grep -E '^g++.*?\.cpp'))
IFS=${IFS_BK}
echo "[" > ${O}
for e in "${C[@]}"
do
	f=$(echo $e|grep -o -E '\S+\.cpp')
	echo -en "{\n  \"directory\": \"${PWD}\",\n  \"file\": \"${f}\",\n  \"command\": \"${e}\"\n},\n" >> ${O}
done
echo "]" >> ${O}
