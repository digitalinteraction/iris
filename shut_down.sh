for ip in "172.16.0.12" "172.16.0.101" "172.16.0.105" "172.16.0.107"
do
	echo "Shutting down $ip"
	sshpass -p "raspberry" ssh -o ConnectTimeout=10 pi@$ip 'sudo i2cset -y 1 0x6B 0 0xcc'
	sleep 5s
	sshpass -p "raspberry" ssh -o ConnectTimeout=10 pi@$ip 'sudo shutdown -h now'

done
