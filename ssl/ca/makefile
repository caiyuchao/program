all: ca.crt

ca.crt:
	openssl req -config ./openssl.cnf -new -x509 -days 30000 -sha1 -newkey rsa:1024 -keyout ./private/ca.key -out ./ca.crt -subj '/O=SSLDemo/OU=my root ca'

server:
	openssl req -new -sha1 -newkey rsa:1024 -nodes -keyout server.key -out requests/request.pem -subj '/O=My Web/OU=My Web/CN=mail.baofeng.com'
	openssl ca -config ./openssl.cnf -policy policy_anything -extensions ssl_server -out requests/signed.pem -infiles requests/request.pem 
	openssl x509 -in requests/signed.pem -out server.crt

client:
	openssl req -new -sha1 -newkey rsa:1024 -keyout client.key -out ./requests/client.pem -subj '/O=client/OU=client/CN=yubo' -config ./openssl.cnf
	openssl ca -config ./openssl.cnf -policy policy_anything -extensions ssl_client -out ./requests/client_signed.pem -infiles ./requests/client.pem
