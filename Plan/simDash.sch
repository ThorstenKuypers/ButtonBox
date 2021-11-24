EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 4
Title "simDash"
Date "23 mar 2014"
Rev "1"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
NoConn ~ 2300 1450
NoConn ~ 2300 1300
Wire Wire Line
	2100 1450 2300 1450
Wire Wire Line
	2100 1300 2300 1300
$Sheet
S 3000 1200 1350 450 
U 5328C89F
F0 "Dashboard" 60
F1 "Dashboard.sch" 60
$EndSheet
$Sheet
S 1000 1950 1100 250 
U 5328C6C6
F0 "ButtonBox" 60
F1 "ButtonBox.sch" 60
$EndSheet
$Sheet
S 1000 1200 1100 450 
U 5328C00D
F0 "simDash-ECU" 60
F1 "simDash-ECU.sch" 60
F2 "HC164_CLK" O R 2100 1300 60 
F3 "HC164_SI" O R 2100 1450 60 
$EndSheet
Text Notes 7500 7425 0    60   ~ 12
simDash root
$EndSCHEMATC
