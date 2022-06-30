# Simple Webserver
Very simple webserver written in plain C++.
It requires my "network"-project (https://github.com/domso/network)
and openSSL (libssl-dev).

It only supports most simple requests and uses openSSL for https.
Each file in the www-folder is accessable.
You need to start the application inside the build-folder.

For self signed certificates run:
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -sha512

