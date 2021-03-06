EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:TLC5940Board_V1-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L TLC5940NT U1
U 1 1 5861DAAD
P 3000 2400
F 0 "U1" H 2500 3275 50  0000 L CNN
F 1 "TLC5940NT" H 3500 3275 50  0000 R CNN
F 2 "Housings_DIP:DIP-28_W7.62mm_Socket_LongPads" H 3050 1425 50  0001 L CNN
F 3 "" H 2600 3100 50  0001 C CNN
	1    3000 2400
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 5864B909
P 1750 4800
F 0 "R2" V 1830 4800 50  0000 C CNN
F 1 "2K" V 1750 4800 50  0000 C CNN
F 2 "Resistors_THT:Resistor_Horizontal_RM7mm" V 1680 4800 50  0001 C CNN
F 3 "" H 1750 4800 50  0000 C CNN
	1    1750 4800
	1    0    0    -1  
$EndComp
$Comp
L R R1
U 1 1 5864BA40
P 1550 1000
F 0 "R1" V 1630 1000 50  0000 C CNN
F 1 "10K" V 1550 1000 50  0000 C CNN
F 2 "Resistors_THT:Resistor_Horizontal_RM7mm" V 1480 1000 50  0001 C CNN
F 3 "" H 1550 1000 50  0000 C CNN
	1    1550 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 850  3000 1400
Connection ~ 3000 850 
Wire Wire Line
	3000 5100 3000 3500
Connection ~ 3000 5100
Wire Wire Line
	2300 3100 1400 3100
Wire Wire Line
	2300 3000 1450 3000
Wire Wire Line
	2300 2300 1500 2300
Wire Wire Line
	1550 2200 2300 2200
Wire Wire Line
	1550 1150 1550 2200
Wire Wire Line
	850  2000 2300 2000
Wire Wire Line
	2300 1700 1600 1700
Wire Wire Line
	1600 1700 1600 5100
Connection ~ 1600 5100
Wire Wire Line
	600  1200 1350 1200
Wire Wire Line
	600  700  600  5100
Wire Wire Line
	2300 1800 1750 1800
Wire Wire Line
	1750 1800 1750 4650
Wire Wire Line
	1750 5100 1750 4950
Connection ~ 1750 5100
Connection ~ 1550 850 
Wire Wire Line
	2300 1900 1800 1900
Wire Wire Line
	1800 1900 1800 850 
Connection ~ 1800 850 
Wire Wire Line
	600  5100 3000 5100
$Comp
L CONN_02X04 P2
U 1 1 5867D614
P 4800 1850
F 0 "P2" H 4800 2100 50  0000 C CNN
F 1 "CN_Out1_02X04" H 4800 1600 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x04_Pitch2.54mm" H 4800 650 50  0001 C CNN
F 3 "" H 4800 650 50  0000 C CNN
	1    4800 1850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3700 1800 3800 1800
Wire Wire Line
	3800 1800 3800 1450
Wire Wire Line
	3800 1450 5100 1450
Wire Wire Line
	5100 1450 5100 1700
Wire Wire Line
	5100 1700 5050 1700
Wire Wire Line
	3700 1900 3900 1900
Wire Wire Line
	3900 1900 3900 1800
Wire Wire Line
	3900 1800 4550 1800
Wire Wire Line
	3700 2000 4000 2000
Wire Wire Line
	4000 2000 4000 1550
Wire Wire Line
	4000 1550 5200 1550
Wire Wire Line
	5200 1550 5200 1800
Wire Wire Line
	5200 1800 5050 1800
Wire Wire Line
	3700 2100 4100 2100
Wire Wire Line
	4100 2100 4100 1900
Wire Wire Line
	4100 1900 4550 1900
Wire Wire Line
	3700 2200 5150 2200
Wire Wire Line
	5150 2200 5150 1900
Wire Wire Line
	5150 1900 5050 1900
Wire Wire Line
	3700 2300 4300 2300
Wire Wire Line
	4300 2300 4300 2000
Wire Wire Line
	4300 2000 4550 2000
Wire Wire Line
	3700 2400 5250 2400
Wire Wire Line
	5250 2400 5250 2000
Wire Wire Line
	5250 2000 5050 2000
$Comp
L CONN_02X04 P3
U 1 1 5867D86F
P 4800 2900
F 0 "P3" H 4800 3150 50  0000 C CNN
F 1 "CN_Out2_02X04" H 4800 2650 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x04_Pitch2.54mm" H 4800 1700 50  0001 C CNN
F 3 "" H 4800 1700 50  0000 C CNN
	1    4800 2900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3700 2500 4550 2500
Wire Wire Line
	4550 2500 4550 2750
Wire Wire Line
	3700 2600 5050 2600
Wire Wire Line
	5050 2600 5050 2750
Wire Wire Line
	3700 2700 4400 2700
Wire Wire Line
	4400 2700 4400 2850
Wire Wire Line
	4400 2850 4550 2850
Wire Wire Line
	3700 2800 3900 2800
Wire Wire Line
	3900 2800 3900 2550
Wire Wire Line
	3900 2550 5150 2550
Wire Wire Line
	5150 2550 5150 2850
Wire Wire Line
	5150 2850 5050 2850
Wire Wire Line
	3700 2900 4150 2900
Wire Wire Line
	4150 2900 4150 2950
Wire Wire Line
	4150 2950 4550 2950
Wire Wire Line
	3700 3000 3850 3000
Wire Wire Line
	3850 3000 3850 3350
Wire Wire Line
	3850 3350 5300 3350
Wire Wire Line
	5300 3350 5300 2950
Wire Wire Line
	5300 2950 5050 2950
Wire Wire Line
	3700 3100 4550 3100
Wire Wire Line
	4550 3100 4550 3050
Wire Wire Line
	3700 3200 5100 3200
Wire Wire Line
	5100 3200 5100 3050
Wire Wire Line
	5100 3050 5050 3050
$Comp
L C C1
U 1 1 5867DCF8
P 1350 1000
F 0 "C1" H 1375 1100 50  0000 L CNN
F 1 "0.1uF" H 1150 900 50  0000 L CNN
F 2 "Capacitors_THT:C_Disc_D6_P5" H 1388 850 50  0001 C CNN
F 3 "" H 1350 1000 50  0000 C CNN
	1    1350 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 1200 1350 1150
Wire Wire Line
	4550 1700 3700 1700
Connection ~ 1350 850 
$Comp
L CONN_02X04 P1
U 1 1 58680B81
P 1100 1750
F 0 "P1" H 1100 2000 50  0000 C CNN
F 1 "CN_SigIn_02X04" H 1100 1500 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x04_Pitch2.54mm" H 1100 550 50  0001 C CNN
F 3 "" H 1100 550 50  0000 C CNN
	1    1100 1750
	1    0    0    -1  
$EndComp
Wire Wire Line
	850  850  3000 850 
Wire Wire Line
	850  1700 600  1700
Connection ~ 600  1700
Text Notes 1000 1600 0    28   ~ 0
V+
Text Notes 1000 1700 0    28   ~ 0
Gnd
Text Notes 1000 1900 0    28   ~ 0
GSClk
Text Notes 1150 1600 0    28   ~ 0
Sin
Text Notes 1150 1700 0    28   ~ 0
SClk
Text Notes 1150 1800 0    28   ~ 0
XLat
Text Notes 1150 1900 0    28   ~ 0
Blk
Wire Wire Line
	850  2000 850  1900
Wire Wire Line
	1350 1900 1550 1900
Connection ~ 1550 1900
Wire Wire Line
	1350 1800 1500 1800
Wire Wire Line
	1500 1800 1500 2300
Wire Wire Line
	1400 3100 1400 1600
Wire Wire Line
	1400 1600 1350 1600
Wire Wire Line
	1350 1700 1450 1700
Wire Wire Line
	1450 1700 1450 3000
Wire Wire Line
	850  850  850  1600
NoConn ~ 2300 2600
$Comp
L PWR_FLAG #FLG01
U 1 1 58683203
P 700 700
F 0 "#FLG01" H 700 795 50  0001 C CNN
F 1 "PWR_FLAG" H 700 880 50  0000 C CNN
F 2 "" H 700 700 50  0000 C CNN
F 3 "" H 700 700 50  0000 C CNN
	1    700  700 
	1    0    0    -1  
$EndComp
$Comp
L PWR_FLAG #FLG02
U 1 1 58683227
P 1150 700
F 0 "#FLG02" H 1150 795 50  0001 C CNN
F 1 "PWR_FLAG" H 1150 880 50  0000 C CNN
F 2 "" H 1150 700 50  0000 C CNN
F 3 "" H 1150 700 50  0000 C CNN
	1    1150 700 
	1    0    0    -1  
$EndComp
Wire Wire Line
	700  700  600  700 
Connection ~ 600  1200
Wire Wire Line
	1150 700  1150 850 
Connection ~ 1150 850 
Wire Wire Line
	2300 3200 750  3200
Wire Wire Line
	750  3200 750  1800
Wire Wire Line
	750  1800 850  1800
$EndSCHEMATC
