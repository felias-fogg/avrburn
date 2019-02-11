#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import sys
import re

if len(sys.argv) != 2:
	print 'Usage: build_signatures.py <path to AVR Studio devices directory>'
	sys.exit(1)

atmel_location = sys.argv[1]

sys.stdout = open('mcus.h', 'w')

all = []

def process_atdf(path):
    global all
    with open(path) as f:
        s = f.read()
        if (s.find('<interface name="ISP"') >= 0):
            name = re.search('<device name="(.*?)"',s).group(1)
            sig = re.search('property name="SIGNATURE1" value="0x(.{2})"',s).group(1) + \
                  re.search('property name="SIGNATURE2" value="0x(.{2})"',s).group(1)
            flashps = re.search('name="FLASH" pagesize="0x(.*?)"',s).group(1)
            flashsz = re.search('size="0x(.*?)"[^>]*?name="FLASH"',s).group(1)
            eepps = re.search('name="EEPROM" pagesize="0x(.*?)"',s).group(1)
            eepsz = re.search('size="0x(.*?)"[^>]*?name="EEPROM"',s).group(1)
            fusesz = re.search('name="fuses"[^>]*?size="0x000(.)"',s).group(1)
            low = re.search('name="LOW"[^>]*?initval="0x(.*?)"',s)
            if (low): low = low.group(1)
            else: low = "00";
            high = re.search('name="HIGH"[^>]*?initval="0x(.*?)"',s)
            if (high): high = high.group(1)
            else: high = "00";
            extended = re.search('name="EXTENDED"[^>]*?initval="0x(.*?)"',s)
            if (extended): extended = extended.group(1)
            else: extended = "00";
            all = all + [(name, sig, fusesz, low, high, extended, flashsz, flashps, eepsz, eepps)]

for filename in os.listdir(atmel_location):
    if filename.endswith(".atdf"):
        path = os.path.join(atmel_location, filename)
        process_atdf(path)

all.sort()

print('typedef struct mcuItem')
print('{')
print('   uint16_t name;')
print('   uint16_t signature;')
print('   uint8_t fuses;')
print('   uint8_t lowFuse;')
print('   uint8_t highFuse;')
print('   uint8_t extendedFuse;')
print('   uint32_t flashSize;')
print('   uint16_t flashPS;')
print('   uint16_t eepromSize;')
print('   uint8_t eepromPS;')
print('} mcuItem;\n')
print 'const char strNull[] PROGMEM = "No MCU";'
for el in all:
    print 'const char str%s[] PROGMEM = "%s";' % (el[0], el[0]);
print('')
print('static const mcuItem mcuList[] PROGMEM =')
print('{')
print('   { (uint16_t)strNull, 0, 0, 0, 0, 0, 0, 0, 0, 0},')
for el in all:
    print('   { (uint16_t)str%s, 0x%s, %s, 0x%s, 0x%s, 0x%s, 0x%sL, 0x%s, 0x%s, 0x%s },' % el)
print('};')
