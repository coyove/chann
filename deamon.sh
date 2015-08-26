until ./run-server.sh; do
	now="$(date +"%Y/%m/%d/%H:%M:%S")"
	echo "Server crashed with exit code $? at $now. Respawning.." >> ./log/crash.log
	sleep 1
done
