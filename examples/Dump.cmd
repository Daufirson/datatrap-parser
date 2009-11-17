@echo off
datatrap -k 5
mysql.exe --user=root --password=PASSWORD WDBDB < ./dump/wdbdata.sql
pause
