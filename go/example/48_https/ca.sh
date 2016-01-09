#!/bin/sh
mkdir ca
cd ca
#生成CA自己的私钥 ca.key
openssl genrsa -out ca.key 2048
#根据CA自己的私钥生成自签发的数字证书，该证书里包含CA自己的公钥
openssl req -x509 -new -nodes -key ca.key -subj "/CN=*.yubo.org" -days 5000 -out ca.crt

#是用来生成server的私钥和数字证书（由自CA签发）
#生成ngrok服务端私钥
openssl genrsa -out server.key 2048
#生成Certificate Sign Request，CSR，证书签名请求
openssl req -new -key server.key -subj "/CN=test.yubo.org" -out server.csr
#自CA用自己的CA私钥对服务端提交的csr进行签名处理，得到服务端的数字证书server.crt
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 5000

# client
openssl genrsa -out client.key 2048
openssl req -new -key client.key -subj "/CN=test.yubo.org" -out client.csr
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 5000
cd -

