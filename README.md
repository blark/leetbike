leetbike
========

A repository for my avr (attiny) based bike light code


Compile Notes
=============

example:
avr-gcc -mmcu=attiny84 -Os cylon.c -o cylon.elf
avr-objcopy -j .text -j .data -O ihex cylon.elf cylon.hex

Programming
===========

pinout for avrisp mkii: http://www.equinox-tech.com/products/details.asp?ID=362#f622

example:
avrdude -c avrispmkII -p t84 -P usb -e -U flash:w:cylon.hex

