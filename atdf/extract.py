#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import sys
import re

if len(sys.argv) != 2:
    print 'Usage: extract.py <path to AVR Studio devices directory with atdf files>'
    print 'Generates mcus.h for avrburn' 
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
            else: extended = "00"
            epoll = re.search('name="IspChipErase_pollMethod" value="(.*?)"',s).group(1)
            edelay = re.search('name="IspChipErase_eraseDelay" value="(.*?)"',s).group(1)
            fmode = re.search('name="IspProgramFlash_cmd2" value="(.*?)"',s).group(1)
            fdelay = re.search('name="IspProgramFlash_delay" value="(.*?)"',s).group(1)
            fpoll = re.search('IspProgramFlash_pollVal1" value="(.*?)"',s).group(1)
            eemode = re.search('name="IspProgramEeprom_cmd2" value="(.*?)"',s).group(1)
            eedelay = re.search('name="IspProgramEeprom_delay" value="(.*?)"',s).group(1)
            eepoll = re.search('name="IspProgramEeprom_pollVal1" value="(.*?)"',s).group(1)
            all = all + [(name, sig, fusesz, low, high, extended, epoll, edelay, flashsz, flashps, fmode, fdelay, fpoll, eepsz, eepps, eemode, eedelay, eepoll)]

for filename in os.listdir(atmel_location):
    if filename.endswith(".atdf"):
        path = os.path.join(atmel_location, filename)
        process_atdf(path)

all.sort()

print('typedef struct mcuItem')
print('{')
print('   const char name[32];')
print('   uint16_t signature;')
print('   uint8_t fuses;')
print('   uint8_t lowFuse;')
print('   uint8_t highFuse;')
print('   uint8_t extendedFuse;')
print('   boolean erasePoll;')
print('   uint8_t eraseDelay;')
print('   uint32_t flashSize;')
print('   uint16_t flashPS;')
print('   uint8_t flashMode;')
print('   uint8_t flashDelay;')
print('   uint8_t flashPoll;')
print('   uint16_t eepromSize;')
print('   uint8_t eepromPS;')
print('   uint8_t eepromMode;')
print('   uint8_t eepromDelay;')
print('   uint8_t eepromPoll;')
print('} mcuItem;\n')
print('')
print('static const mcuItem mcuList[] =')
print('{')
print('   { "No MCU", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},')
for el in all:
    print('   { "%s", 0x%s, %s, 0x%s, 0x%s, 0x%s, %s, %s, 0x%sL, 0x%s, %s, %s, %s, 0x%s, 0x%s, %s, %s, %s },' % el)
print('};')
