for x in `cat end_of_file`; do
    printf $x
    sleep 0.1
done | nc 127.0.0.1 $1 