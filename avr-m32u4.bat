@echo off
REM Programar ATmega32U4 com avrdude
REM Para colocar o ATmega32U4 no modo bootloader, abre/fecha a porta COM a 1200bps, faz o truque
set PATH=%PATH%;C:\Program Files (x86)\Arduino\hardware\tools\avr\bin\;
mode COM51:1200,N,8,1,P && waitfor /T 2 pause 2>null || avrdude -C./avrdude.conf -cavr109 -PCOM50 -b57600 -pm32u4