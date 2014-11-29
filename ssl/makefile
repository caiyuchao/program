.PHONY: all

all: pri.key pub.key server.key server.crt 1 2 3 4 5

pri.key: 
	openssl genrsa -out pri.key 1024

pub.key: pri.key
	openssl rsa  -in pri.key -pubout -out pub.key

server.key:
server.crt:
	openssl req -new -x509 -days 30 -keyout server.key -out server.crt -subj '/CN=yubo.org'
	openssl req -new -x509 -days 30 -sha1 -nodes -newkey rsa:1024 -keyout server.key -out server.crt -subj '/o=seccure/ou=seccure labs/CN=yubo.org'
	cp server.* /etc/httpd/conf

#对称加解密
1:
	openssl enc -des3 -k password -in file -out 1.file.encode
	openssl enc -des3 -d -k password -in 1.file.encode -out 1.file.decode

#非对称加解密
2:
	openssl  rsautl -in file -out 2.file.encode  -inkey pub.key -pubin -encrypt
	openssl rsautl -in 2.file.encode -out 2.file.decode -inkey pri.key -decrypt

#数字签名
3:
	openssl  rsautl -in file -out 3.file.encode  -inkey pri.key -sign
	openssl rsautl -in 3.file.encode -out 3.file.decode -inkey pub.key -pubin -verify

#随机数+非对称加密
4:
	openssl rand -base64 20 > 4.session.password
	openssl rsautl -in 4.session.password -out 4.session.encode.0  -inkey pub.key -pubin -encrypt
	openssl enc -des3 -k `cat 4.session.password` -in file -out 4.file.encode.0
	tar czvf 4.tgz 4.session.encode.0 4.file.encode.0
	rm -f  4.session.encode.0 4.file.encode.0
	tar xzvf 4.tgz 
	openssl rsautl -in 4.session.encode.0 -out 4.session.password.1 -inkey pri.key -decrypt
	openssl enc -des3 -d -k `cat 4.session.password.1` -in 4.file.encode.0 -out 4.file.decode

#hash指纹+数字签名
5:
	md5sum file | awk '{print $$1}' > 5.md5sum
	openssl rsautl -in 5.md5sum -out 5.sign.encode  -inkey pri.key -sign
	tar czvf 5.tgz file 5.sign.encode
	tar xzvf 5.tgz
	openssl rsautl -in 5.sign.encode -out 5.md5sum.1 -inkey pub.key -pubin -verify
	md5sum file
	cat 5.md5sum.1


clean:
	rm -f 1.* 2.* 3.* 4.* 5.*
