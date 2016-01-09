# Create private/public key pair
CAROOT=$(pwd)/ca
openssl genrsa -out server.key 2048

# Create Certificate Signing Request
openssl req -new -key server.key \
                 -out server.csr

# Sign key
openssl ca -config ${CAROOT}/ca.conf   \
           -in server.csr              \
           -cert ${CAROOT}/ca.crt      \
           -keyfile ${CAROOT}/ca.key   \
           -out server.crt
