#!/bin/bash

function is_linux
{
	if [[ "$(uname)" == "Linux" ]]; then
		echo 1
		return
	fi

	echo 0
}

SET="setmem"
GET="getmem"
CFLAGS=""
if [[ "$(is_linux)" == "1" ]]; then
	CFLAGS="-lrt"
fi

cc ${CFLAGS} -o ${SET} ${SET}.c
cc ${CFLAGS} -o ${GET} ${GET}.c

./${SET} &
sleep 1
./${GET}
if [[ "$(is_linux)" == "1" ]]; then
	echo "/dev/shm:"
	ls -l /dev/shm
fi
sleep 1

rm ${SET} ${GET}
