echo off
mkdir images
cls
taskkill /im ssl_wrapper.exe
taskkill /im sql.exe
start ssl_wrapper ssl://443:cert.pem localhost:8080
sql -title "&#21311;&#21517;&#29256;" -spell Qwerty0731 -tpp 10 -database test.db -NOGPFAULTERRORBOX