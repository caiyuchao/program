All:	db mysql

db:	db.c
	gcc -g -ldb -lpthread -o db db.c

mysql:	mysql.c
	gcc -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient -lz -o mysql mysql.c
