#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import sys
import re

if len(sys.argv) != 2:
    print('Usage: extract.py <path to AVR Studio devices directory with all atdf files>')
    print('Generates mcus.h for avrburn' )
    sys.exit(1)

atmel_location = sys.argv[1]


assoc = {
    'EXIT' : (0, 'EXIT', 1),
    'N1V8' : (1, 'BOD 1.8Vcc', 1),
    'N1V9' : (2, 'BOD 1.9Vcc', 1), 
    'N2V0' : (3, 'BOD 2.0Vcc', 1), 
    'N2V1' : (4, 'BOD 2.1Vcc', 1), 
    'N2V2' : (5, 'BOD 2.2Vcc', 1), 
    'N2V3' : (6, 'BOD 2.3Vcc', 1), 
    'N2V4' : (7, 'BOD 2.4Vcc', 1),
    'N2V5' : (8, 'BOD 2.5Vcc', 1),
    'N2V6' : (9, 'BOD 2.6Vcc', 1),
    'N2V7' : (10, 'BOD 2.7Vcc', 1),
    'N2V8' : (11, 'BOD 2.8Vcc', 1),
    'N2V9' : (12, 'BOD 2.9Vcc', 1),
    'N3V0' : (13, 'BOD 3.0Vcc', 1),
    'N3V4' : (14, 'BOD 3.4Vcc', 1),
    'N3V5' : (15, 'BOD 3.5Vcc', 1),
    'N3V6' : (16, 'BOD 3.6Vcc', 1),
    'N3V7' : (17, 'BOD 3.7Vcc', 1),
    'N3V8' : (18, 'BOD 3.8Vcc', 1),
    'N3V9' : (19, 'BOD 3.9Vcc', 1),
    'N4V0' : (20, 'BOD 4.0Vcc', 1),
    'N4V1' : (21, 'BOD 4.1Vcc', 1),
    'N4V2' : (22, 'BOD 4.2Vcc', 1),
    'N4V3' : (23, 'BOD 4.3Vcc', 1),
    'N4V4' : (24, 'BOD 4.4Vcc', 1), 
    'N4V5' : (25, 'BOD 4.5Vcc', 1), 
    'BODEN' : (26, 'BOD enabled', 1),
    'DISABLED' : (27, "BOD disabled", 1),
    'BOD_SAMPLED' : (28, "BOD sampled", 1),
    'BOOTRST' : (29, "Boot reset active", 1),
    'N128W_1F80' : (30, "Boot flash 128 W", 1),
    'N256W_1F00' : (31, "Boot flash 256 W", 1),
    'N512W_1E00' : (32, "Boot flash 512 W", 1),
    'N1024W_1C00' : (33, "Boot flash 1024 W", 1),
    'N2048W_1800' : (34, "Boot flash 2048 W", 1),
    'N4096W_7000'  : (35, "Boot flash 4096 W", 1), 
    'SPIEN' : (36, "SPI prog. enabled", 0),
    'RSTDISBL' : (37, "RESET disabled", 0),
    'DWEN' : (38, "debugWire enabled", 0),
    'OCDEN' : (39, "OnChip debug enable", 0),
    'JTAGEN' : (40, "JTAG int. enabled", 0),
    'DUVRDINIT' : (41, "DUVR mode on", 0),
    'SELFPRGEN' : (42, "Self prog enabled", 1),
    'HWBE' : (43, "HW boot enable", 1),
    'WDTON' : (44, "Watchdog always on", 1),
    'EESAVE' : (45, "Preserve EEPROM", 1),
    'NDEFAULT' : (46, "Osc select:default", 0),
    'CKDIV8' : (47, "Divide CLK by 8", 1),
    'CKOUT' : (48, "CLK output on port", 1),
    'PSCRB' : (49, "PSC reset behavior", 1),
    'PSCRVA' : (50, "PSCOUTnA reset val", 1),
    'PSCRVB' : (51, "PSCOUTnB reset val", 1),
    'PSC2RB' : (52, "PSC2 reset behavior", 1),
    'PSC1RB' : (53, "PSC1 reset behavior", 1),
    'PSC0RB' : (54, "PSC0 reset behavior", 1),
    'PSCRV' : (55, "PSCOUT reset val", 1),
    'PSCINRB' : (56, "PSC2/0in RST behv", 1),
    'CKOPT' : (57, "CKOPT fuse", 0),
    'INTRCOSC' : (58, "Int oscillator", 1),
    'INTRCOSC_8MHZ_6CK_14CK_0MS' : (59, "Int 8MHz 0ms up", 1),
    'INTRCOSC_8MHZ_6CK_14CK_4MS' : (60, "Int 8MHz 4ms up", 1),
    'INTRCOSC_8MHZ_6CK_14CK_64MS' : (61, "Int 8MHz 64ms up", 1),
    'INTRCOSC_6CK_0MS' : (62, 'Int osc 0ms up', 1),
    'INTRCOSC_6CK_4MS1' : (63, 'Int osc 4ms up', 1),
    'INTRCOSC_6CK_65MS' : (64, 'Int osc 64ms up',1 ),
    'INTRCOSC_1MHZ_6CK_0MS' : (65, "Int 1MHz 0ms up", 1),
    'INTRCOSC_1MHZ_6CK_4MS' : (66, "Int 1MHz 4ms up", 1),
    'INTRCOSC_1MHZ_6CK_64MS' :(67, "Int 1MHz 64ms up", 1),
    'INTRCOSC_2MHZ_6CK_0MS' : (68, "Int 2MHz 0ms up", 1),
    'INTRCOSC_2MHZ_6CK_4MS' : (69, "Int 2MHz 4ms up", 1),
    'INTRCOSC_2MHZ_6CK_64MS' :(70, "Int 2MHz 64ms up", 1),
    'INTRCOSC_4MHZ_6CK_0MS' : (71, "Int 4MHz 0ms up", 1),
    'INTRCOSC_4MHZ_6CK_4MS' : (72, "Int 4MHz 4ms up", 1),
    'INTRCOSC_4MHZ_6CK_64MS' : (73, "Int 4MHz 64ms up", 1),
    'INTRCOSC_128KHZ_6CK_0MS' : (74, "Int 128kHz 0ms up", 1),
    'INTRCOSC_128KHZ_6CK_4MS' : (75, "Int 128kHz 4ms up", 1),
    'INTRCOSC_128KHZ_6CK_64MS' : (76, "Int 128kHz 64ms up", 1),
    'INTULPOSC_32KHZ_6CK_14CK_0MS' : (77, "Int 32kHz 0ms up", 1),
    'INTULPOSC_32KHZ_6CK_14CK_4MS1' : (78, "Int 32kHz 4ms up", 1),
    'INTULPOSC_32KHZ_6CK_14CK_65MS' : (79, "Int 32kHz 64ms up", 1),
    'INTRCOSC_8MHZ_6CK_16CK_16MS' : (80, "Int 8MHz 16ms up", 1),
    'INTULPOSC_32KHZ_6CK_16CK_16MS' : (81, "Int 32kHz 16ms up", 1),
    'INTRCOSC_4MHZ8_14CK_0MS' : (82, "Int 4.8MHz 0ms up", 1),
    'INTRCOSC_4MHZ8_14CK_4MS' : (83, "Int 4.8MHz 4ms up", 1),
    'INTRCOSC_4MHZ8_14CK_64MS' : (84, "Int 4.8MHz 64ms up", 1),
    'INTRCOSC_9MHZ6_14CK_0MS' : (85, "Int 9.6MHz 0ms up", 1),
    'INTRCOSC_9MHZ6_14CK_4MS' : (86, "Int 9.6MHz 4ms up", 1),
    'INTRCOSC_9MHZ6_14CK_64MS' : (87, "Int 9.6MHz 64ms up", 1),
    'PLLCLK_16MHZ_1KCK_14CK_0MS' : (88, "PLLi 16MHz 1K/0ms" , 1),
    'PLLCLK_16MHZ_1KCK_14CK_4MS1' : (89, "PLLi 16MHz 1K/4ms" , 1),
    'PLLCLK_16MHZ_1KCK_14CK_65MS' : (90, "PLLi 16MHz 1K/64ms", 1),
    'PLLCLK_16MHZ_16KCK_14CK_0MS' : (91, "PLLi 16MHz 16K/0ms", 1),
    'PLLCLK_1KCK_14CK_4MS' : (92, "PLLInt 1K/4ms up", 1),
    'PLLCLK_16KCK_14CK_4MS' : (93, "PLLInt 16K/4ms up", 1),
    'PLLCLK_1KCK_14CK_64MS' : (94, "PLLInt 1K/64ms up", 1),
    'PLLCLK_16KCK_14CK_64MS' : (95, "PLLInt 16K/64ms up", 1),
    'ULPOSC_32KHZ' : (96, "Int low pwr 32kHz", 1),
    'ULPOSC_64KHZ' : (97, "Int low pwr 64kHz", 1),
    'ULPOSC_128KHZ' : (98, "Int low pwr 128kHz", 1),
    'ULPOSC_256KHZ' : (99, "Int low pwr 256kHz", 1),
    'ULPOSC_512KHZ' : (100, "Int low pwr 512kHz", 1),
    'WDOSC_128KHZ_16KCK_14CK_0MS' : (101, "Int 128kHz 16K/0ms", 1),
    'N14CK_0MS' : (102, "Startup 14Ck+0ms", 1),
    'N14CK_4MS' : (103, "Startup 14Ck+4ms", 1),
    'N14CK_8MS' : (104, "Startup 14Ck+8ms", 1),
    'N14CK_16MS' : (105, "Startup 14Ck+16ms", 1),
    'N14CK_32MS' : (106, "Startup 14Ck+32ms", 1),
    'N14CK_64MS' : (107, "Startup 14Ck+64ms", 1),
    'N14CK_128MS' : (108, "Startup 14Ck+128ms", 1),
    'N14CK_256MS' : (109, "Startup 14Ck+256ms", 1),
    'N14CK_512MS' : (110, "Startup 14Ck+512ms", 1),
    'N6CK_14CK_4MS' : (111, "Start 6C/14C+4ms", 1),
    'N6CK_14CK_8MS' : (112, "Start 6C/14C+8ms", 1),
    'N6CK_14CK_16MS' : (113, "Start 6C/14C+16ms", 1),
    'N6CK_14CK_32MS' : (114, "Start 6C/14C+32ms", 1),
    'N6CK_14CK_64MS' : (115, "Start 6C/14C+64ms", 1),
    'N6CK_14CK_128MS' : (116, "Start 6C/14C+128ms", 1),
    'N6CK_14CK_256MS' : (117, "Start 6C/14C+256ms", 1),
    'N6CK_14CK_512MS' : (118, "Start 6C/14C+512ms", 1),
    'PLLCLK_1KCK_14CK_8MS' : (119, "PLLi 16MHz 1K/8ms", 1),
    'PLLCLK_16KCK_14CK_8MS' : (120, "PLLi 16MHz 16K/8ms", 1),
    'PLLCLK_1KCK_14CK_68MS' : (121, "PLLi 16MHz 1K/68ms", 1),
    'PLLCLK_16KCK_14CK_68MS' : (122, "PLLi 16MHz 16K/68ms", 1),
    'PLLCLK_DIV4_PLLIN_RC_8MHZ_1KCK_14CK_0MS' : (123, "PLLi 8MHz 0ms", 1),
    'PLLCLK_DIV4_PLLIN_RC_8MHZ_1KCK_14CK_4MS' : (124, "PLLi 8MHz 4ms", 1),
    'PLLCLK_DIV4_PLLIN_RC_8MHZ_1KCK_14CK_64MS' : (125, "PLLi 8MHz 64ms", 1),
    'VQUICKPWR' : (126, "PWR very quickly", 1),
    'QUICKPWR' :  (127, "PWR quickly", 1),
    'SLOWPWR' : (128,  "PWR slowly", 1),
    'CFD' : (129, "XTAL failure detec", 1),
    'M103C' : (130, "Mega103 comp mode", 1),
    'S8515C' : (131, "AT90S4414 comp mode", 1),
    'S8535C' : (132, "AT90S4434 comp mode", 1),
    'M161C' : (133, "Mega161 comp mode", 1),
    'OSCSEL0' : (134, "Oscillator select", 0),
    'TA0SEL' : (135, "Reserved fot tests", 0),
    'EXTXTALCRES' : (136, "Ext XTAL/Res", 0),
    'EXTLOFXTAL' : (137, "Ext low freq XTAL", 0),
    'EXTRCOSC' : (138, "Ext RC osc", 0),
    'EXTCLK' : (139, "Ext CLK", 0),
    'EXTCLK_6CK_14CK_0MS' : (140, "Ext CLK 0ms up", 0),
    'EXTCLK_6CK_14CK_4MS1': (141, "Ext CLK 4ms up", 0),
    'EXTCLK_6CK_14CK_65MS': (142, "Ext CLK 64ms up", 0),
    'EXTCLK_PLLIN_RC_8MHZ_6CK_14CK_0MS': (143, "PLLe 8MHz 0ms", 0),
    'EXTCLK_PLLIN_RC_8MHZ_6CK_14CK_4MS1' : (144, "PLLe 8MHz 4ms", 0),
    'EXTCLK_PLLIN_RC_8MHZ_6CK_14CK_65MS' : (145, "PLLe 8MHz 64ms", 0),
    'PLLCLK_DIV4_PLLIN_EXTCLK_16KCK_14CK_0MS' : (146, "PLL ext clk/4 0ms", 0),
    'PLLCLK_DIV4_PLLIN_EXTCLK_16KCK_14CK_4MS' : (147, "PLL ext clk/4 4ms", 0),
    'PLLCLK_DIV4_PLLIN_EXTCLK_16KCK_14CK_64MS' : (148, "PLL ext clk/4 64ms", 0),
    'EXTCLK_6CK_16CK_16MS' : (149, "Ext CLK 16ms up", 0),
    'EXTLOFXTAL_1KCK_0MS' : (150, "Low F XTAL 1K/0ms", 0),
    'EXTLOFXTAL_1KCK_4MS1' : (151, "Low F XTAL 1K/4ms", 0),
    'EXTLOFXTAL_1KCK_65MS' : (152, "Low F XTAL 1K/64ms", 0),
    'EXTLOFXTAL_32KCK_0MS' : (153, "Low F XTAL 32K/0ms", 0),
    'EXTLOFXTAL_32KCK_4MS1' : (154, "Low F XTAL 32K/4ms", 0),
    'EXTLOFXTAL_32KCK_65MS' : (155, "Low F XTAL 32K/64ms", 0),
    'FSOSC_258CK_65MS_CRES_SLOWPWR' : (156, "FS XTAL 258/64ms", 0),
    'FSOSC_1KCK_0MS_CRES_BODEN' : (157, "FS XTAL 1K/0ms", 0),
    'FSOSC_1KCK_4MS1_CRES_FASTPWR' :  (158, "FS XTAL 1K/4ms", 0),
    'FSOSC_1KCK_65MS_CRES_SLOWPWR' : (159,  "FS XTAL 1K/64ms", 0),
    'FSOSC_16KCK_0MS_XOSC_BODEN' : (160, "FS XTAL 16K/0ms", 0),
    'FSOSC_16KCK_4MS1_XOSC_FASTPWR' : (161, "FS XTAL 16K/4ms", 0),
    'FSOSC_16KCK_65MS_XOSC_SLOWPWR' : (162, "FS XTAL 64K/4ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_258CK_4MS1' : (163, "XTAL.4-.9MHz258/4ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_258CK_65MS' : (164, "XTAL.4-.9MHz258/16ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_1KCK_0MS' : (165, "XTAL.4-.9MHz1K/0ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_1KCK_4MS1' : (166, "XTAL.4-.9MHz1K/4ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_1KCK_65MS' : (167, "XTAL.4-.9MHz1K/64ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_16KCK_0MS' :  (168, "XTAL.4-.9MHz16K/0ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_16KCK_4MS1' : (169, "XTAL.4-.9MHz16K/4ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_16KCK_65MS' : (170, "XTAL.4-.9MHz16K/64ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_258CK_4MS1' :  (171, "XTAL.9-3MHz258/4ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_258CK_65MS' :  (172, "XTAL.9-3MHz258/64ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_1KCK_0MS' :   (173, "XTAL.9-3MHz 1K/0ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_1KCK_4MS1' :  (174, "XTAL.9-3MHz 1K/4ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_1KCK_65MS' : (175, "XTAL.9-3MHz 1K/64ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_16KCK_0MS' : (176, "XTAL.9-3MHz16K/0ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_16KCK_4MS1' : (177, "XTAL.9-3MHz16K/4ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_16KCK_65MS' : (178, "XTAL.9-3MHz16K/64ms", 0),
    'EXTXOSC_3MHZ_8MHZ_258CK_4MS1' : (179, "XTAL 3-8MHz258/0ms", 0),
    'EXTXOSC_3MHZ_8MHZ_258CK_65MS' : (180, "XTAL 3-8MHz258/65ms", 0),
    'EXTXOSC_3MHZ_8MHZ_1KCK_0MS' : (181, "XTAL 3-8MHz 1K/0ms", 0),
    'EXTXOSC_3MHZ_8MHZ_1KCK_4MS1' : (182, "XTAL 3-8MHz 1K/4ms", 0),
    'EXTXOSC_3MHZ_8MHZ_1KCK_65MS' : (183, "XTAL 3-8MHz 1K/64ms", 0),
    'EXTXOSC_3MHZ_8MHZ_16KCK_0MS' : (184, "XTAL 3-8MHz16K/0ms", 0),
    'EXTXOSC_3MHZ_8MHZ_16KCK_4MS1' : (185, "XTAL 3-8MHz16K/4ms", 0),
    'EXTXOSC_3MHZ_8MHZ_16KCK_65MS' :  (186, "XTAL 3-8MHz16K/64ms", 0),
    'EXTXOSC_8MHZ_XX_258CK_4MS1' :  (187, "XTAL 8-MHz 258/4ms", 0),
    'EXTXOSC_8MHZ_XX_258CK_65MS' : (188,  "XTAL 8-MHz 258/64ms", 0),
    'EXTXOSC_8MHZ_XX_1KCK_0MS' :  (189,  "XTAL 8-MHz 1K/0ms", 0),
    'EXTXOSC_8MHZ_XX_1KCK_4MS1' : (190,  "XTAL 8-MHz 1K/4ms", 0),
    'EXTXOSC_8MHZ_XX_1KCK_65MS' : (191,  "XTAL 8-MHz 1K/64ms", 0),
    'EXTXOSC_8MHZ_XX_16KCK_0MS' : (192,  "XTAL 8-MHz 16K/0ms", 0),
    'EXTXOSC_8MHZ_XX_16KCK_4MS1' : (193,  "XTAL 8-MHz 16K/4ms", 0),
    'EXTXOSC_8MHZ_XX_16KCK_65MS' : (194,  "XTAL 8-MHz 16K/64ms", 0),
    'INTRCOSC_4MHZ_6CK_14CK_64MS' : (195,  "Int 6.4MHz 64ms up", 1),
    'INTRCOSC_4MHZ_6CK_14CK_4MS' : (196,  "Int 6.4MHz 4ms up", 1),
    'INTRCOSC_4MHZ_6CK_14CK_0MS' : (197,  "Int 6.4MHz 0ms up", 1),
    'EXTFSXTAL_258CK_14CK_4MS1' : (198, "FS XTAL 258/4ms up", 0),
    'EXTFSXTAL_258CK_14CK_65MS' : (199, "FS XTAL 258/64ms up", 0),
    'EXTFSXTAL_1KCK_14CK_0MS' : (200, "FS XTAL 1K/0ms up", 0),
    'EXTFSXTAL_1KCK_14CK_4MS1' : (201, "FS XTAL 1K/4ms up", 0),
    'EXTFSXTAL_1KCK_14CK_65MS' : (202, "FS XTAL 1K/64ms up", 0),
    'EXTFSXTAL_16KCK_14CK_0MS' : (203, "FS XTAL 16K/0ms up", 0),
    'EXTFSXTAL_16KCK_14CK_4MS1' : (204, "FS XTAL 16K/4ms up", 0),
    'EXTFSXTAL_16KCK_14CK_65MS' : (205, "FS XTAL 16K/64ms up", 0),
    'EXTXOSC_0MHZ4_0MHZ9_14CK_4MS1' : (206, "XTAL .4-.9MHz 4ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_14CK_65MS' : (207, "XTAL .4-.9MHz 64ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_14CK_0MS' : (208, "XTAL .4-.9MHz 0ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_14CK_4MS1' : (209, "XTAL .9-3MHz 4ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_14CK_65MS' : (210, "XTAL .9-3MHz 64ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_14CK_0MS' :  (211, "XTAL .9-3MHz 0ms", 0),
    'EXTXOSC_3MHZ_8MHZ_14CK_4MS1' :  (212, "XTAL 3-8MHz 4ms", 0),
    'EXTXOSC_3MHZ_8MHZ_14CK_65MS' :  (213, "XTAL 3-8MHz 64ms", 0),
    'EXTXOSC_3MHZ_8MHZ_14CK_0MS' : (214, "XTAL 3-8MHz 0ms", 0),
    'EXTXOSC_8MHZ_XX_14CK_4MS1' : (215, "XTAL 8- MHz 4ms", 0),
    'EXTXOSC_8MHZ_XX_14CK_65MS' : (216, "XTAL 8- MHz 64ms", 0),
    'EXTXOSC_8MHZ_XX_14CK_0MS' :  (217, "XTAL 8- MHz 0ms", 0),
    'EXTCRES_0MHZ4_0MHZ9_258CK_14CK_4MS1' : (218, "Res.4-.9MHz258/4ms", 0),
    'EXTCRES_0MHZ4_0MHZ9_258CK_14CK_65MS' : (219, "Res.4-.9MHz258/64ms", 0),
    'EXTCRES_0MHZ4_0MHZ9_1KCK_14CK_0MS' : (220, "Res.4-.9MHz 1K/0ms", 0),
    'EXTCRES_0MHZ4_0MHZ9_1KCK_14CK_4MS1' : (221, "Res.4-.9MHz 1K/4ms", 0),
    'EXTCRES_0MHZ4_0MHZ9_1KCK_14CK_65MS' : (222, "Res.4-.9MHz 1K/64ms", 0),
    'EXTCRES_0MHZ9_3MHZ_258CK_14CK_4MS1' : (223, "Res.9-3MHz258/4ms", 0),
    'EXTCRES_0MHZ9_3MHZ_258CK_14CK_65MS' : (224, "Res.9-3MHz258/64ms", 0),
    'EXTCRES_0MHZ9_3MHZ_1KCK_14CK_0MS' : (225, "Res.9-3MHz 1K/0ms", 0),
    'EXTCRES_0MHZ9_3MHZ_1KCK_14CK_4MS1' : (226, "Res.9-3MHz 1K/4ms", 0),
    'EXTCRES_0MHZ9_3MHZ_1KCK_14CK_65MS' : (227, "Res.9-3MHz 1K/64ms", 0),
    'EXTCRES_3MHZ_8MHZ_258CK_14CK_4MS1' : (228, "Res 3-8MHz258/4ms", 0),
    'EXTCRES_3MHZ_8MHZ_258CK_14CK_65MS' : (229, "Res 3-8MHz258/64ms", 0),
    'EXTCRES_3MHZ_8MHZ_1KCK_14CK_0MS' : (230, "Res 3-8MHz 1K/0ms", 0),
    'EXTCRES_3MHZ_8MHZ_1KCK_14CK_4MS1' : (231, "Res 3-8MHz 1K/4ms", 0),
    'EXTCRES_3MHZ_8MHZ_1KCK_14CK_65MS' :  (232, "Res 3-8MHz 1K/64ms", 0),
    'EXTCRES_8MHZ_16MHZ_258CK_14CK_4MS1' : (233, "Res8-16MHz258/4ms", 0),
    'EXTCRES_8MHZ_16MHZ_258CK_14CK_65MS' : (234, "Res8-16MHz258/64ms", 0),
    'EXTCRES_8MHZ_16MHZ_1KCK_14CK_0MS' :  (235, "Res8-16MHz 1K/0ms", 0),
    'EXTCRES_8MHZ_16MHZ_1KCK_14CK_4MS1' : (236, "Res8-16MHz 1K/4ms", 0),
    'EXTCRES_8MHZ_16MHZ_1KCK_14CK_65MS' : (237, "Res8-16MHz 1K/64ms", 0),
    'EXTXOSC_8MHZ_16MHZ_16KCK_14CK_0MS' : (238, "XTAL8-16MHz16K/0ms", 0),
    'EXTXOSC_8MHZ_16MHZ_16KCK_14CK_4MS1' : (239, "XTAL8-16MHz16K/4ms", 0),
    'EXTXOSC_8MHZ_16MHZ_16KCK_14CK_65MS' : (240, "XTAL8-16MHz16K/64ms", 0),
    'EXTRCOSC_XX_0MHZ9_18CK_0MS' : (241, "ExtRC-.9MHz 18/0ms", 0),
    'EXTRCOSC_XX_0MHZ9_18CK_4MS' : (242, "ExtRC-.9MHz 18/4ms", 0),
    'EXTRCOSC_XX_0MHZ9_18CK_64MS' : (243, "ExtRC-.9MHz 18/64ms", 0),
    'EXTRCOSC_XX_0MHZ9_6CK_4MS' : (244, "ExtRC-.9MHz 6/4ms", 0),
    'EXTRCOSC_0MHZ9_3MHZ_18CK_0MS' : (245, "ExtRC.9-3MHz18/0ms", 0),
    'EXTRCOSC_0MHZ9_3MHZ_18CK_4MS' : (246, "ExtRC.9-3MHz18/4ms", 0),
    'EXTRCOSC_0MHZ9_3MHZ_18CK_64MS' : (247, "ExtRC.9-3MHz18/64ms", 0),
    'EXTRCOSC_0MHZ9_3MHZ_6CK_4MS' : (248, "ExtRC.9-3MHz 6/4ms", 0),
    'EXTRCOSC_8MHZ_12MHZ_18CK_0MS' : (249, "ExtRC8-12MHz18/0ms", 0),
    'EXTRCOSC_8MHZ_12MHZ_18CK_4MS' : (250, "ExtRC8-12MHz18/4ms", 0),
    'EXTRCOSC_8MHZ_12MHZ_18CK_64MS' : (251, "ExtRC8-12MHz18/64ms", 0),
    'EXTRCOSC_8MHZ_12MHZ_6CK_4MS' : (252, "ExtRC8-12MHz 6/4ms", 0),
    'EXTLOFXTALRES_258CK_4MS' : (253, "Low F XTAL 258/4ms", 0),
    'EXTLOFXTALRES_258CK_64MS' : (254, "Low F XTAL 258/64ms", 0),
    'EXTLOFXTALRES_1KCK_0MS' : (255, "Low F XTAL 1K/0ms", 0),
    'EXTLOFXTALRES_1KCK_4MS' : (256, "Low F XTAL 1K/4ms", 0),
    'EXTLOFXTALRES_1KCK_64MS' : (257, "Low F XTAL 1K/64ms", 0),
    'EXTLOFXTALRES_16KCK_0MS' : (258, "Low F XTAL 16K/0ms", 0),
    'EXTLOFXTALRES_16KCK_4MS' : (259, "Low F XTAL 16K/4ms", 0),
    'EXTLOFXTALRES_16KCK_64MS' : (260, "Low F XTAL 16K/64ms", 0),
    'EXTMEDFXTALRES_1KCK_0MS' :  (261, "Med F XTAL 1K/0ms", 0),
    'EXTMEDFXTALRES_1KCK_4MS' :  (262, "Med F XTAL 1K/4ms", 0),
    'EXTMEDFXTALRES_1KCK_64MS' :  (263, "Med F XTAL 1K/64ms", 0),
    'EXTMEDFXTALRES_16KCK_0MS' :  (264, "Med F XTAL 16K/0ms", 0),
    'EXTMEDFXTALRES_16KCK_4MS' :  (265, "Med F XTAL 16K/4ms", 0),
    'EXTMEDFXTALRES_16KCK_64MS' :  (266, "Med F XTAL 16K/64ms", 0),
    'EXTHIFXTALRES_258CK_4MS' :  (267, "HighF XTAL 258/0ms", 0),
    'EXTHIFXTALRES_258CK_64MS' : (268, "HighF XTAL 258/64ms", 0),
    'EXTHIFXTALRES_1KCK_0MS' : (269, "HighF XTAL 1K/0ms", 0),
    'EXTHIFXTALRES_1KCK_4MS' : (270, "HighF XTAL 1K/4ms", 0),
    'EXTHIFXTALRES_1KCK_64MS' : (271, "HighF XTAL 1K/64ms", 0),
    'EXTHIFXTALRES_16KCK_0MS' : (272, "HighF XTAL 16K/0ms", 0),
    'EXTHIFXTALRES_16KCK_4MS' : (273, "HighF XTAL 16K/4ms", 0),
    'EXTHIFXTALRES_16KCK_64MS' : (274, "HighF XTAL 16K/64ms", 0),
    'EXTRCOSC_3MHZ_8MHZ_18CK_0MS' : (275, "ExtRC3-8MHz18/0ms", 0),
    'EXTRCOSC_3MHZ_8MHZ_18CK_4MS' : (276, "ExtRC3-8MHz18/4ms", 0),
    'EXTRCOSC_3MHZ_8MHZ_18CK_64MS' : (277, "ExtRC3-8MHz18/64ms", 0),
    'EXTRCOSC_3MHZ_8MHZ_6CK_4MS' : (278, "ExtRC3-8MHz 6/4ms", 0),
    'EXTMEDFXTALRES_258CK_4MS' : (279, "Med F XTAL 256/4ms", 0),
    'EXTMEDFXTALRES_258CK_64MS' : (280, "Med F XTAL 256/64ms", 0),
    'FSOSC_258CK_4MS1_CRES_FASTPWR' : (281, "FS XTAL 256/4ms", 0),
    'PLLCLK_PLLIN_EXTCLK_6KCK_14CK_0MS' : (282, "PLLe/4 6K/0ms up", 0),
    'PLLCLK_PLLIN_EXTCLK_6KCK_14CK_4MS' : (283, "PLLe/4 6K/4ms up", 0),
    'PLLCLK_PLLIN_EXTCLK_6KCK_14CK_64MS' : (284, "PLLe/4 6K/64ms up", 0),
    'PLLCLK_PLLIN_EXTXOSC_1KCK_14CK_0MS' : (285, "PLLeXTAL/4 1K/0ms", 0),
    'PLLCLK_PLLIN_EXTXOSC_1KCK_14CK_4MS' : (286, "PLLeXTAL/4 1K/4ms", 0),
    'PLLCLK_PLLIN_EXTXOSC_1KCK_14CK_64MS' : (287, "PLLeXTAL/4 1K/64ms", 0),
    'PLLCLK_PLLIN_EXTXOSC_16KCK_14CK_0MS' : (288, "PLLeXTAL/4 16K/0ms", 0),
    'PLLCLK_PLLIN_EXTXOSC_16KCK_14CK_4MS' : (289, "PLLeXTAL/4 16K/4ms", 0),
    'PLLCLK_PLLIN_EXTXOSC_16KCK_14CK_64MS' : (290, "PLLeXTAL/4 16K/64ms", 0),
    'EXTXOSC_PLLIN_EXTXOSC_1KCK_14CK_0MS' : (291, "XTAL PLL 1K/0ms up", 0),
    'EXTXOSC_PLLIN_EXTXOSC_1KCK_14CK_4MS' : (292, "XTAL PLL 1K/4ms up", 0),
    'EXTXOSC_PLLIN_EXTXOSC_16KCK_14CK_4MS' : (293, "XTAL PLL 16K/4ms up", 0),
    'EXTXOSC_PLLIN_EXTXOSC_16KCK_14CK_64MS' : (294, "XTAL PLL 16K/64ms up", 0),
    'EXTLOFXTAL_32KCK_0MS_INTCAP' : (295, "LowFXTALiCap32K/0ms", 0),
    'EXTLOFXTAL_32KCK_4MS1_INTCAP' : (296, "LowFXTALiCap32K/4ms", 0),
    'EXTLOFXTAL_32KCK_65MS_INTCAP' : (297, "LowFXTALiCap32K/64ms", 0),
    'EXTLOFXTAL_1KCK_0MS_INTCAP' : (298, "LowFXTALiCap1K/0ms", 0),
    'EXTLOFXTAL_1KCK_4MS1_INTCAP' : (299, "LowFXTALiCap1K/4ms", 0),
    'EXTLOFXTAL_1KCK_65MS_INTCAP' : (300, "LowFXTALiCap1K/64ms", 0),
    'TOSC_258CK_4MS1' : (301, "TX Osc 258/4ms up", 0),
    'TOSC_258CK_65MS' : (302, "TX Osc 258/64ms up", 0),
    'TOSC_1KCK_0MS' : (303, "TX Osc 1K/0ms up", 0),
    'TOSC_1KCK_4MS1' : (304, "TX Osc 1K/4ms up", 0),
    'TOSC_1KCK_65MS' : (305, "TX Osc 1K/64ms up", 0),
    'TOSC_16KCK_0MS' : (306, "TX Osc 16K/0ms up", 0),
    'TOSC_16KCK_4MS1' : (307, "TX Osc 16K/4ms up", 0),
    'TOSC_16KCK_65MS' : (308, "TX Osc 16K/64ms up", 0),
    'EXTXOSC_8MHZ_16MHZ_258CK_4MS1' : (309, "XTAL8-16MHz258/4ms", 0),
    'EXTXOSC_8MHZ_16MHZ_258CK_65MS' : (310, "XTAL8-16MHz258/64ms", 0),
    'EXTXOSC_8MHZ_16MHZ_1KCK_0MS' : (311, "XTAL8-16MHz1K/0ms", 0),
    'EXTXOSC_8MHZ_16MHZ_1KCK_4MS1' : (312, "XTAL8-16MHz1K/4ms", 0),
    'EXTXOSC_8MHZ_16MHZ_1KCK_65MS' : (313, "XTAL8-16MHz1K/64ms", 0),
    'EXTCRES_8MHZ_XX_258CK_14CK_4MS1' : (314, "Res8-Hz 256/4ms", 0),
    'EXTCRES_8MHZ_XX_258CK_14CK_65MS' : (315, "Res8-Hz 256/64ms", 0),
    'EXTCRES_8MHZ_XX_1KCK_14CK_0MS' :  (316, "Res8-Hz 1K/0ms", 0),
    'EXTCRES_8MHZ_XX_1KCK_14CK_4MS1' : (317, "Res8-Hz 1K/4ms", 0),
    'EXTCRES_8MHZ_XX_1KCK_14CK_65MS' : (318, "Res8-Hz 1K/64ms", 0),
    'XOSC_PLLIN_RC_8MHZ_258CK_14CK_4MS1' : (319, "XTAL-3M/PLLi258/4ms", 0),
    'XOSC_PLLIN_RC_8MHZ_258CK_14CK_65MS' : (320, "XTAL-3M/PLLi258/64ms", 0),
    'XOSC_PLLIN_RC_8MHZ_1KCK_14CK_0MS' : (321, "XTAL-3M/PLLi1K/0ms", 0),
    'XOSC_PLLIN_RC_8MHZ_1KCK_14CK_4MS1' : (322, "XTAL-3M/PLLi1K/4ms", 0),
    'XOSC_PLLIN_RC_8MHZ_1KCK_14CK_65MS' : (323, "XTAL-3M/PLLi1K/64ms", 0),
    'XOSC_PLLIN_RC_8MHZ_16KCK_14CK_0MS' : (324, "XTAL-3M/PLLi16K/0ms", 0),
    'XOSC_PLLIN_RC_8MHZ_16KCK_14CK_4MS1' : (325, "XTAL-3M/PLLi16K/4ms", 0),
    'XOSC_PLLIN_RC_8MHZ_16KCK_14CK_65MS' : (326, "XTAL-3M/PLLi16K/64ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_258CK_14CK_4MS1' : (327, "XTAL3-8M/Pi258/4ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_258CK_14CK_65MS' : (328, "XTAL3-8M/Pi258/64ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_1KCK_14CK_0MS' : (329, "XTAL3-8M/Pi1K/0ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_1KCK_14CK_4MS1' : (330, "XTAL3-8M/Pi1K/4ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_1KCK_14CK_65MS' : (331, "XTAL3-8M/Pi1K/64ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_16KCK_14CK_0MS' : (332, "XTAL3-8M/Pi16K/0ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_16KCK_14CK_4MS1' : (333, "XTAL3-8M/Pi16K/4ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_RC_8MHZ_16KCK_14CK_65MS' : (334, "XTAL3-8M/Pi16K/64ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_258CK_14CK_4MS1' : (335, "XTAL8-M/Pi256/4ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_258CK_14CK_65MS' : (336, "XTAL8-M/Pi256/64ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_1KCK_14CK_0MS' : (337, "XTAL8-M/Pi1K/0ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_1KCK_14CK_4MS1' : (338, "XTAL8-M/Pi1K/4ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_1KCK_14CK_65MS' : (339, "XTAL8-M/Pi1K/64ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_16KCK_14CK_0MS' : (340, "XTAL8-M/Pi16K/0ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_16KCK_14CK_4MS1' : (341, "XTAL8-M/Pi16K/4ms", 0),
    'XOSC_8MHZ_16MHZ_PLLIN_RC_8MHZ_16KCK_14CK_65MS' : (342, "XTAL8-M/Pi16K/64ms", 0),
    'PLLCLK_DIV4_PLLIN_XOSC_1KCK_14CK_0MS' : (343, "PLLeXTAL/4 1K/0ms up", 0),
    'PLLCLK_DIV4_PLLIN_XOSC_1KCK_14CK_4MS' : (344, "PLLeXTAL/4 1K/4ms up", 0),
    'PLLCLK_DIV4_PLLIN_XOSC_1KCK_14CK_64MS' : (345, "PLLeXTAL/4 1K/64ms up", 0),
    'PLLCLK_DIV4_PLLIN_XOSC_16KCK_14CK_0MS' : (346, "PLLeXTAL/4 16K/0ms up", 0),
    'EXTLOFXTAL_1KCK_16CK_16MS' : (347, "Low F XTAL 1K/16ms", 0),
    'EXTCRES_0MHZ4_0MHZ9_258CK_16CK_16MS' : (348, "Res.4-.9MHz258/16ms", 0),
    'EXTCRES_0MHZ4_0MHZ9_1KCK_16CK_16MS' :  (349, "Res.4-.9MHz 1K/16ms", 0),
    'EXTCRES_0MHZ9_3MHZ_258CK_16CK_16MS' : (350, "Res.9-3MHz 258/16ms", 0),
    'EXTCRES_0MHZ9_3MHZ_1KCK_16CK_16MS' :  (351, "Res.9-3MHz  1K/16ms", 0),
    'EXTCRES_3MHZ_8MHZ_258CK_16CK_16MS' : (352, "Res3-8MHz 256/16ms", 0),
    'EXTCRES_3MHZ_8MHZ_1KCK_16CK_16MS' : (353, "Res3-8MHz  1K/16ms", 0),
    'EXTCRES_8MHZ_XX_258CK_16CK_16MS' : (354, "Res8-MHz 256/16ms", 0),
    'EXTCRES_8MHZ_XX_1KCK_16CK_16MS' : (355, "Res8-MHz  1K/16ms", 0),
    'EXTXOSC_0MHZ4_0MHZ9_16KCK_16CK_16MS' : (356, "XTAL.4-.9MHz16K/16ms", 0),
    'EXTXOSC_0MHZ9_3MHZ_16KCK_16CK_16MS' : (357, "XTAL.9-3MHz16K/16ms", 0),
    'EXTXOSC_3MHZ_8MHZ_16KCK_16CK_16MS' :  (358, "XTAL3-8MHz16K/16ms", 0),
    'EXTXOSC_8MHZ_XX_16KCK_16CK_16MS' :  (359, "XTAL8-MHz16K/16ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_16KCK_14CK_65MS' : (360, "XTAL3-8M/Pe16K/64ms", 0),
    'EXTLOFXTAL_32KCK_14CK_16MS' : (361, "Low F XTAL 32K/16ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_258CK_14CK_4MS1' : (362, "XTAL3-8M/Pe258/4ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_258CK_14CK_65MS' : (363, "XTAL3-8M/Pe258/64ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_1KCK_14CK_0MS' : (364, "XTAL3-8M/Pe1K/0ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_1KCK_14CK_4MS1' : (365, "XTAL3-8M/Pe1K/4ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_1KCK_14CK_65MS' : (366, "XTAL3-8M/Pe1K/64ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_16KCK_14CK_0MS' : (367, "XTAL3-8M/Pe16K/0ms", 0),
    'XOSC_3MHZ_8MHZ_PLLIN_XOSC_16KCK_14CK_4MS1' : (368, "XTAL3-8M/Pe16K/4ms", 0),
    'LBPROG_VER_DISABLED' : (369, "No prog/no verif", 1),
    'LBPROG_DISABLED' : (370, "No porgramming", 1),
    'LBNO_LOCK' : (371, "No prog. lock", 1),
    'BLB0LPM_SPM_DISABLE' : (372, "No LPM&SPM to APP area", 1),
    'BLB0LPM_DISABLE' : (373, "No LPM to APP area", 1),
    'BLB0SPM_DISABLE' : (374, "No SPM to APP area", 1),
    'BLB0NO_LOCK' : (375, "No lock on APP area", 1),
    'BLB1LPM_SPM_DISABLE' : (376, "No LPM&SPM to BOOT area", 1),
    'BLB1LPM_DISABLE' : (377, "No LPM to BOOT area", 1),
    'BLB1SPM_DISABLE' : (378, "No SPM to BOOT area", 1),
    'BLB1NO_LOCK' : (379, "No lock on BOOT area", 1),
    'WDOSC_128KHZ_1KCK_14CK_65MS' : (380, "Int 128kHz 1K/64ms", 1),
    'WDOSC_128KHZ_1KCK_14CK_4MS1' : (381, "Int 128kHz 1K/4ms", 1),
    'WDOSC_128KHZ_1KCK_14CK_0MS' : (382, "Int 128kHz 1K/0ms", 1),
    }

mapsto = {
    'EXTLOFXTAL_32CK_64MS' : 'EXTLOFXTAL_32KCK_65MS',
    'EXTLOFXTAL_1CK_64MS' : 'EXTLOFXTALRES_1KCK_64MS',
    'EXTLOFXTAL_1CK_4MS' : 'EXTLOFXTALRES_1KCK_4MS',
    'TRXOSC_258CK_4MS1' : 'TOSC_258CK_4MS1',
    'TRXOSC_258CK_65MS' : 'TOSC_258CK_65MS',
    'TRXOSC_1KCK_0MS'   : 'TOSC_1KCK_0MS',
    'TRXOSC_1KCK_4MS1'  : 'TOSC_1KCK_4MS1',
    'TRXOSC_1KCK_65MS'  :'TOSC_1KCK_65MS',
    'TRXOSC_16KCK_0MS'  :  'TOSC_16KCK_0MS',
    'TRXOSC_16KCK_4MS1' :   'TOSC_16KCK_4MS1',
    'TRXOSC_16KCK_65MS' :  'TOSC_16KCK_65MS',
    'EXTXOSC_3MHZ_8MHZ_16KCK_14CK_0MS' :  'EXTXOSC_3MHZ_8MHZ_16KCK_0MS',
    'EXTXOSC_3MHZ_8MHZ_16KCK_14CK_4MS1' : 'EXTXOSC_3MHZ_8MHZ_16KCK_4MS1',
    'EXTXOSC_3MHZ_8MHZ_16KCK_14CK_65MS' : 'EXTXOSC_3MHZ_8MHZ_16KCK_65MS',
    'EXTXOSC_0MHZ9_3MHZ_16KCK_14CK_0MS' :  'EXTXOSC_0MHZ9_3MHZ_14CK_0MS',
    'EXTXOSC_0MHZ9_3MHZ_16KCK_14CK_4MS1' : 'EXTXOSC_0MHZ9_3MHZ_14CK_4MS1',
    'EXTXOSC_0MHZ9_3MHZ_16KCK_14CK_65MS' : 'EXTXOSC_0MHZ9_3MHZ_14CK_65MS',
    'EXTLOFXTAL_32KCK_14CK_65MS' : 'EXTLOFXTAL_32KCK_65MS',
    'EXTLOFXTAL_32KCK_14CK_4MS1' : 'EXTLOFXTAL_32KCK_4MS1',
    'EXTLOFXTAL_32KCK_14CK_0MS' : 'EXTLOFXTAL_32KCK_0MS',
    'EXTLOFXTAL_1KCK_14CK_65MS' : 'EXTLOFXTAL_1KCK_65MS',
    'EXTLOFXTAL_1KCK_14CK_0MS' :  'EXTLOFXTAL_1KCK_0MS',
    'EXTLOFXTAL_1KCK_14CK_4MS1' : 'EXTLOFXTAL_1KCK_4MS1',
    'EXTLOFXTAL_1KCK_14CK_4MS' :  'EXTLOFXTAL_1KCK_4MS1',
    'EXTLOFXTAL_32KCK_14CK_64MS' : 'EXTLOFXTAL_32KCK_65MS',
    'EXTLOFXTAL_1KCK_4MS' : 'EXTLOFXTAL_1KCK_4MS1',
    'EXTLOFXTAL_1KCK_64MS' :  'EXTLOFXTAL_1KCK_65MS',
    'EXTLOFXTAL_32KCK_64MS' : 'EXTLOFXTAL_32KCK_65MS',
    'EXTXOSC_0MHZ4_0MHZ9_258CK_14CK_4MS1' : 'EXTXOSC_0MHZ4_0MHZ9_258CK_4MS1',
    'EXTXOSC_0MHZ4_0MHZ9_258CK_14CK_65MS' : 'EXTXOSC_0MHZ4_0MHZ9_258CK_65MS',
    'EXTXOSC_0MHZ4_0MHZ9_1KCK_14CK_0MS' : 'EXTXOSC_0MHZ4_0MHZ9_1KCK_0MS',
    'EXTXOSC_0MHZ4_0MHZ9_1KCK_14CK_4MS1' : 'EXTXOSC_0MHZ4_0MHZ9_1KCK_4MS1',
    'EXTXOSC_0MHZ4_0MHZ9_1KCK_14CK_65MS' : 'EXTXOSC_0MHZ4_0MHZ9_1KCK_65MS',
    'EXTXOSC_0MHZ4_0MHZ9_16KCK_14CK_0MS' : 'EXTXOSC_0MHZ4_0MHZ9_16KCK_0MS',
    'EXTXOSC_0MHZ4_0MHZ9_16KCK_14CK_4MS1' : 'EXTXOSC_0MHZ4_0MHZ9_16KCK_4MS1',
    'EXTXOSC_0MHZ4_0MHZ9_16KCK_14CK_65MS' : 'EXTXOSC_0MHZ4_0MHZ9_16KCK_65MS',
    'EXTXOSC_0MHZ9_3MHZ_258CK_14CK_4MS1' : 'EXTXOSC_0MHZ9_3MHZ_258CK_4MS1',
    'EXTXOSC_0MHZ9_3MHZ_258CK_14CK_65MS' : 'EXTXOSC_0MHZ9_3MHZ_258CK_65MS',
    'EXTXOSC_0MHZ9_3MHZ_1KCK_14CK_0MS' : 'EXTXOSC_0MHZ9_3MHZ_1KCK_0MS',
    'EXTXOSC_0MHZ9_3MHZ_1KCK_14CK_4MS1' : 'EXTXOSC_0MHZ9_3MHZ_1KCK_4MS1',
    'EXTXOSC_0MHZ9_3MHZ_1KCK_14CK_65MS' : 'EXTXOSC_0MHZ9_3MHZ_1KCK_65MS',
    'EXTXOSC_0MHZ9_3MHZ_16KCK_14CK_0MS' : 'EXTXOSC_0MHZ9_3MHZ_16KCK_0MS',
    'EXTXOSC_0MHZ9_3MHZ_16KCK_14CK_4MS1' : 'EXTXOSC_0MHZ9_3MHZ_16KCK_4MS1',
    'EXTXOSC_0MHZ9_3MHZ_16KCK_14CK_65MS' : 'EXTXOSC_0MHZ9_3MHZ_16KCK_65MS',
    'EXTXOSC_3MHZ_8MHZ_258CK_14CK_4MS1' : 'EXTXOSC_3MHZ_8MHZ_258CK_4MS1',
    'EXTXOSC_3MHZ_8MHZ_258CK_14CK_65MS' : 'EXTXOSC_3MHZ_8MHZ_258CK_65MS',
    'EXTXOSC_3MHZ_8MHZ_1KCK_14CK_0MS' : 'EXTXOSC_3MHZ_8MHZ_1KCK_0MS',
    'EXTXOSC_3MHZ_8MHZ_1KCK_14CK_4MS1' : 'EXTXOSC_3MHZ_8MHZ_1KCK_4MS1',
    'EXTXOSC_3MHZ_8MHZ_1KCK_14CK_65MS' : 'EXTXOSC_3MHZ_8MHZ_1KCK_65MS',
    'EXTXOSC_3MHZ_8MHZ_16KCK_14CK_0MS' : 'EXTXOSC_3MHZ_8MHZ_16KCK_0MS',
    'EXTXOSC_3MHZ_8MHZ_16KCK_14CK_4MS1' : 'EXTXOSC_3MHZ_8MHZ_16KCK_4MS1',
    'EXTXOSC_3MHZ_8MHZ_16KCK_14CK_65MS' : 'EXTXOSC_3MHZ_8MHZ_16KCK_65MS',
    'EXTXOSC_8MHZ_XX_258CK_14CK_4MS1' : 'EXTXOSC_8MHZ_XX_258CK_4MS1',
    'EXTXOSC_8MHZ_XX_258CK_14CK_65MS' : 'EXTXOSC_8MHZ_XX_258CK_65MS',
    'EXTXOSC_8MHZ_XX_1KCK_14CK_0MS' : 'EXTXOSC_8MHZ_XX_1KCK_0MS',
    'EXTXOSC_8MHZ_XX_1KCK_14CK_4MS1' : 'EXTXOSC_8MHZ_XX_1KCK_4MS1',
    'EXTXOSC_8MHZ_XX_1KCK_14CK_65MS' : 'EXTXOSC_8MHZ_XX_1KCK_65MS',
    'EXTXOSC_8MHZ_XX_16KCK_14CK_0MS' : 'EXTXOSC_8MHZ_XX_16KCK_0MS',
    'EXTXOSC_8MHZ_XX_16KCK_14CK_4MS1' : 'EXTXOSC_8MHZ_XX_16KCK_4MS1',
    'EXTXOSC_8MHZ_XX_16KCK_14CK_65MS' : 'EXTXOSC_8MHZ_XX_16KCK_65MS',
    'EXTCLK_14CK_64MS' : 'EXTCLK_6CK_14CK_65MS',
    'EXTCLK_14CK_4MS' :  'EXTCLK_6CK_14CK_4MS1',
    'EXTCLK_6CK_14CK_4MS' :  'EXTCLK_6CK_14CK_4MS1',
    'EXTCLK_6CK_14CK_64MS' : 'EXTCLK_6CK_14CK_65MS',
    'EXTCLK_6CK_0MS' : 'EXTCLK_6CK_14CK_0MS',
    'EXTCLK_6CK_4MS1' :  'EXTCLK_6CK_14CK_4MS1',
    'EXTCLK_6CK_65MS' :  'EXTCLK_6CK_14CK_65MS',
    'EXTCLK_6CK_4MS' : 'EXTCLK_6CK_14CK_4MS1',
    'EXTCLK_6CK_64MS' : 'EXTCLK_6CK_14CK_65MS',
    'EXTCLK_14CK_0MS' :   'EXTCLK_6CK_14CK_0MS',
    'EXTCLK_14CK_4MS1' : 'EXTCLK_6CK_14CK_4MS1',
    'EXTCLK_14CK_65MS' : 'EXTCLK_6CK_14CK_65MS',
    'CompMode' : 'M103C',
    'PSC2RBA' : 'PSC2RB',
    'PLLCLK_1KCK_0MS' : 'PLLCLK_16MHZ_1KCK_14CK_0MS',
    'PLLCLK_1KCK_4MS' : 'PLLCLK_16MHZ_1KCK_14CK_4MS1',
    'PLLCLK_1KCK_64MS' : 'PLLCLK_16MHZ_1KCK_14CK_65MS',
    'PLLCLK_16KCK_64MS' : 'PLLCLK_16MHZ_16KCK_14CK_0MS',
    'RC_8MHZ_PLLIN_RC_8MHZ_6CK_14CK_65MS' : 'INTRCOSC_8MHZ_6CK_14CK_64MS',
    'RC_8MHZ_PLLIN_RC_8MHZ_6CK_14CK_4MS1' : 'INTRCOSC_8MHZ_6CK_14CK_4MS',
    'RC_8MHZ_PLLIN_RC_8MHZ_6CK_14CK_0MS' : 'INTRCOSC_8MHZ_6CK_14CK_0MS',
    'RC_1MHZ_1KCK_14CK_65MS' :  'INTRCOSC_1MHZ_6CK_64MS',
    'RC_1MHZ_1KCK_14CK_4MS1' : 'INTRCOSC_1MHZ_6CK_4MS',
    'RC_1MHZ_1KCK_14CK_0MS' : 'INTRCOSC_1MHZ_6CK_0MS',
    'WDOSC_128KHZ_6CK_14CK_64MS' : 'INTRCOSC_128KHZ_6CK_64MS',
    'WDOSC_128KHZ_6CK_14CK_4MS' : 'INTRCOSC_128KHZ_6CK_4MS',
    'WDOSC_128KHZ_6CK_14CK_0MS' : 'INTRCOSC_128KHZ_6CK_0MS',
    'INTRCOSC_1MHZ_6CK_64MS_DEFAULT' : 'INTRCOSC_1MHZ_6CK_64MS',
    'INTRCOSC_8MHZ_6CK_14CK_65MS_DEFAULT' : 'INTRCOSC_8MHZ_6CK_14CK_64MS',
    'INTRCOSC_128KHZ_14CK_64MS' : 'INTRCOSC_128KHZ_6CK_64MS',
    'INTRCOSC_128KHZ_14CK_4MS' : 'INTRCOSC_128KHZ_6CK_4MS',
    'INTRCOSC_128KHZ_14CK_0MS' : 'INTRCOSC_128KHZ_6CK_0MS',
    'INTRCOSC_8MHZ_14CK_65MS' : 'INTRCOSC_8MHZ_6CK_14CK_64MS',
    'INTRCOSC_8MHZ_14CK_4MS1' : 'INTRCOSC_8MHZ_6CK_14CK_4MS',
    'INTRCOSC_8MHZ_14CK_0MS' : 'INTRCOSC_8MHZ_6CK_14CK_0MS',
    'INTRCOSC_4MHZ_14CK_65MS' : 'INTRCOSC_4MHZ_6CK_64MS',
    'INTRCOSC_4MHZ_14CK_4MS1' : 'INTRCOSC_4MHZ_6CK_4MS',
    'INTRCOSC_4MHZ_14CK_0MS' : 'INTRCOSC_4MHZ_6CK_0MS',
    'INTRCOSC_8MHZ_6CK_14CK_64MS_DEFAULT' : 'INTRCOSC_8MHZ_6CK_14CK_64MS',
    'INTRCOSC_4MHZ_1CK_14CK_0MS' : 'INTRCOSC_4MHZ_6CK_0MS',
    'INTRCOSC_128KHZ_6CK_14CK_65MS' : 'INTRCOSC_128KHZ_6CK_64MS',
    'INTRCOSC_128KHZ_6CK_14CK_4MS1' : 'INTRCOSC_128KHZ_6CK_4MS',
    'INTRCOSC_128KHZ_6CK_14CK_0MS' : 'INTRCOSC_128KHZ_6CK_0MS',
    'INTRCOSC_128KHZ_6CK_4MS1' : 'INTRCOSC_128KHZ_6CK_4MS',
    'INTRCOSC_128KHZ_6CK_65MS' : 'INTRCOSC_128KHZ_6CK_64MS',
    'BOD_ENABLED' : 'BODEN',
    'BOD_DISABLED' : 'DISABLED',
    'INTRCOSC_8MHZ_6CK_14CK_65MS' : 'INTRCOSC_8MHZ_6CK_14CK_64MS',
    'INTRCOSC_8MHZ_6CK_14CK_4MS1' : 'INTRCOSC_8MHZ_6CK_14CK_4MS', 
    'INTRCOSC_8MHZ_6CK_0MS' : 'INTRCOSC_8MHZ_6CK_14CK_0MS',
    'INTRCOSC_8MHZ_6CK_4MS' : 'INTRCOSC_8MHZ_6CK_14CK_4MS',
    'INTRCOSC_8MHZ_6CK_64MS' : 'INTRCOSC_8MHZ_6CK_14CK_64MS',
    'N512W_7E00' : 'N512W_1E00',
    'N1024W_7C00' : 'N1024W_1C00',
    'N2048W_7800' : 'N2048W_1800',
    'N256W_3F00' : 'N256W_1F00',
    'N512W_3E00' : 'N512W_1E00',
    'N1024W_3C00' : 'N1024W_1C00',
    'N2048W_3800' : 'N2048W_1800',
    'N256W_F00' : 'N256W_1F00',
    'N512W_E00' : 'N512W_1E00',
    'N1024W_C00' : 'N1024W_1C00',
    'N2048W_800' : 'N2048W_1800',
    'N128W_0F80' : 'N128W_1F80',
    'N256W_0F00' : 'N256W_1F00',
    'N512W_0E00' : 'N512W_1E00',
    'N1024W_0C00' : 'N1024W_1C00',
    'N512W_FE00' : 'N512W_1E00',
    'N1024W_FC00' : 'N1024W_1C00',
    'N2048W_F800' : 'N2048W_1800',
    'N4096W_F000' : 'N4096W_7000',
    'N512W_1FE00' : 'N512W_1E00',
    'N1024W_1FC00' : 'N1024W_1C00',
    'N2048W_1F800' : 'N2048W_1800',
    'N4096W_1F000' : 'N4096W_7000',
    'N4096W_3000' : 'N4096W_7000',
    'N512W_7F00' : 'N512W_1E00',
    'N1024W_7E00' : 'N1024W_1C00',
    'N2408W_7C00' : 'N2048W_1800',
    'N4096W_7800' : 'N4096W_7000',
}

mcuTypeString = '''
typedef struct mcuItem { 
  char name[MAX_MCU_NAME_LENGTH+1];
  uint16_t signature;
  uint8_t fuses;
  uint8_t lowFuse;
  uint8_t highFuse;
  uint8_t extendedFuse;
  boolean erasePoll;
  uint8_t eraseDelay;
  uint32_t flashSize;
  uint16_t flashPS;
  uint8_t flashMode;
  uint8_t flashDelay;
  uint8_t flashPoll;
  uint16_t eepromSize;
  uint8_t eepromPS;
  uint8_t eepromMode;
  uint8_t eepromDelay;
  uint8_t eepromPoll;
} mcuItem;
static const mcuItem mcuList[] =
{
   { "No MCU", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},''';

fuseTypeString = '''
typedef struct fuseItems
{
  const uint16_t mes;
  const uint8_t addr;
  const uint8_t mask;
  const uint8_t value;
  const boolean danger;
} fuseItems;

typedef struct fuseMenu {
  const uint16_t sig;
  const fuseItems fuseList[MAX_FUSE_PROPS];
} fuseMenu;
''';

def process_atdf(path):
    with open(path) as f:
        s = f.read()
        if (s.find('<interface name="ISP"') < 0): return None
        return(extract_mcuprops(s), extract_fuseprops(s), extract_lockprops(s))

def extract_mcuprops(s):
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
    return (name, sig, fusesz, low, high, extended, epoll, edelay, flashsz, flashps, fmode, fdelay, fpoll, eepsz, eepps, eemode, eedelay, eepoll)

def extract_fuseprops(s):
    lock = re.search('(?s)register caption="" name="LOCKBIT".*?>(.*?)</register>',s)
    if (lock): lock = lock.group(1)
    sig = re.search('property name="SIGNATURE1" value="0x(.{2})"',s).group(1) + \
        re.search('property name="SIGNATURE2" value="0x(.{2})"',s).group(1)
    extfuse = re.search('(?s)register caption="" name="EXTENDED".*?>(.*?)</register>',s)
    if (extfuse): extfuse = extfuse.group(1)
    highfuse = re.search('(?s)register caption="" name="HIGH".*?>(.*?)</register>',s)
    if (highfuse): highfuse = highfuse.group(1) 
    lowfuse = re.search('(?s)register caption="" name="LOW".*?>(.*?)</register>',s)
    if (lowfuse): lowfuse = lowfuse.group(1)
    return( [sig] + bitfields(extfuse,2,s) + bitfields(highfuse,1,s) + bitfields(lowfuse,0,s) + bitfields(lock,3,s))

def bitfields(fields, addr, s):
    entries = []
    if not fields:
        return entries
    for el in re.findall('(?s)<bitfield (.*?)/>',fields):
        if el.find('values="') < 0:
            entries.append((normalize_name(re.search('name="(.*?)"',el).group(1)), addr, re.search('mask="(.*?)"',el).group(1), "0x00"))
        else:
            entries.extend(valuefields(normalize_name(re.search('name="(.*?)"',el).group(1)),re.search('values="(.*?)"',el).group(1),
                                       re.search('mask="(.*?)"',el).group(1), addr, s))
    return entries

def valuefields(name, valname, mask, addr, s):
    entries = []
    fields = re.search('(?s)<value-group caption="" name="' + valname + '">(.*?)</value-group>',s).group(1)
    for (n,v) in re.findall('<value caption=".*?" name="(.*?)" value="(.*?)"/>',fields):
        if (addr == 3): n = name + n
        entries.append((normalize_name(n), addr, mask, normalize_val(mask,v)))
    return entries

def normalize_name(n):
    if n[0].isdigit() or n == "DEFAULT":
        n = "N" + n
    if n in mapsto:
        n = mapsto[n]
    return n

def normalize_val(mask, val):
    mask = int(mask,0)
    val = int(val,0)
    shift = 0
    while (True):
        if mask%2: break
        mask >>= 1
        shift += 1
    val <<= shift
    return '0x{:02x}'.format(val)

def extract_lockprops(s):
    return []

def output_mcus(mculist, fuselist):
    mculist.sort()
    closedlist = set()
    maxnamelen = 0
    for el in mculist:
        maxnamelen = max(maxnamelen, len(el[0]))
    maxfuseprop = 0
    for el in fuselist:
        maxfuseprop = max(maxfuseprop, len(el))
    with open('mcus.h', 'w') as m:
        print('#define MAX_MCU_NAME_LENGTH', maxnamelen, file=m)
        print('#define MAX_FUSE_PROPS', maxfuseprop, file=m)
        print(mcuTypeString, file=m)
        for el in mculist:
            if el[1] not in closedlist:
                print('   { "%s", 0x%s, %s, 0x%s, 0x%s, 0x%s, %s, %s, 0x%sL, 0x%s, %s, %s, %s, 0x%s, 0x%s, %s, %s, %s },' % el, file=m)
                closedlist.add(el[1])
        print('};', file=m)
        print(fuseTypeString,file=m)
        
        # highest index in assoc so far
        lastprop = 0
        for (_,el) in assoc.items():
            lastprop = max(lastprop, el[0])
        # check for new properties and insert
        for el in fuselist:
            for prop in el[1:]:
                if prop[0] not in assoc:
                    print(prop[0], "is unknown!")
                    lastprop += 1
                    assoc.update({prop[0] : (lastprop, prop[0], 0)})
        toprint = 0
        for const in sorted(assoc.items(),key=(lambda x: x[1])):
            if toprint != const[1][0]:
                print("Item", toprint, "missing!")
                exit()
            print("#define", const[0], const[1][0], file=m)
            toprint += 1
        print("\nconst char * fuseProps[] = {", file=m)
        for const in sorted(assoc.items(),key=(lambda x: x[1])):
            print('  "' + const[1][1] + '",', file=m)
        print("};",file=m)
        print("\nconst fuseMenu fuseMenuList[] = {\n  { 0, { } },",file=m)
        for el in fuselist:
            print("  { 0x" + el[0] + ", {",file=m)
            for prop in el[1:]:
                print("     { %s, %d, %s, %s, %s }, " % (prop[0], prop[1], prop[2], prop[3], "false" if assoc[prop[0]][2] else "true"),file=m)
            print("     { EXIT, 0, 0, 0, false } } },", file=m)
        print("};\n",file=m);
                

mculist = []
fuselist = []
locklist = []

for filename in os.listdir(atmel_location):
    if filename.endswith(".atdf"):
        path = os.path.join(atmel_location, filename)
        result = process_atdf(path)
        if result:
            (mcuspec, fusespec, lockspec) = result
            mculist.append(mcuspec)
            fuselist.append(fusespec)
            locklist.append(lockspec)
            
output_mcus(mculist,fuselist)
