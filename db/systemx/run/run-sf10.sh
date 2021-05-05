#!/bin/bash
<<COM
sudo systemctl stop mssql-server
cd /mnt/data/sql/data
sudo rm -rf hatrickbench_dataContainer/
sudo rm hatrickbench.mdf
sudo rm hatrickbench_log.ldf
sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
sudo systemctl start mssql-server
sleep 60
COM
sqlcmd -S localhost -U SA -P 'H4TrickB3nch' -i db/systemx/initDB.sql
./HATrickBench -init -dsn 'ssdsn'  -usr 'SA' -pwd 'H4TrickB3nch' -pa '/mnt/data/csv-sf10/' -db systemx
./HATrickBench -frontier -dsn 'ssdsn' -usr 'SA' -pwd 'H4TrickB3nch' -wd 120 -td 180 -db systemx -t sp
