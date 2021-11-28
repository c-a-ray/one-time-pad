#!/bin/bash
gcc -std=gnu99 -o keygen keygen.c

gcc -std=gnu99 -c util.c
gcc -std=gnu99 -c socket_io.c
gcc -std=gnu99 -c enc_client.c
gcc -std=gnu99 -c enc_server.c
gcc -std=gnu99 -c dec_client.c
gcc -std=gnu99 -c dec_server.c

gcc -std=gnu99 -o enc_client enc_client.o util.o socket_io.o
gcc -std=gnu99 -o enc_server enc_server.o util.o socket_io.o
gcc -std=gnu99 -o dec_client dec_client.o util.o socket_io.o
gcc -std=gnu99 -o dec_server dec_server.o util.o socket_io.o

rm -f util.o socket_io.o enc_client.o enc_server.o dec_client.o dec_server.o