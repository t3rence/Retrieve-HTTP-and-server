#!/bin/bash
echo 'Building Files'
./build.sh
echo 'Starting server'
./server 1305 &
echo 'Connecting to http://www.play-hookey.com/htmltest/'
./receiver http://www.play-hookey.com/htmltest/
echo ''
echo ''
echo 'Connecting to http://127.0.0.1:1305/test.html'
./receiver http://127.0.0.1:1305/test.html
echo ''
echo ''
echo 'Connecting to http://127.0.0.1:1305/NotFound.html'
./receiver http://127.0.0.1:1305/NotFound
echo ''
echo ''
echo 'Connecting to http://127.0.0.1:1305/SecretFile.html'
./receiver http://127.0.0.1:1305/SecretFile.html
echo ''
echo ''
echo 'Connecting to http://127.0.0.1:1305/../../etc/passwd'
./receiver http://127.0.0.1:1305/../../etc/passwd
echo ''
echo ''
echo 'Connecting to http://127.0.0.1:1305/SecretFile.html with BAD request'
./receiver http://127.0.0.1:1305/SecretFile.html true
echo ''
echo 'Killing server'
kill $(ps | grep server | cut -d ' ' -f 1)
