@ECHO OFF
@REM ###########################################
@REM # Script file to run the flow 
@REM # 
@REM ###########################################
@REM #
@REM # Command line for ngdbuild
@REM #
ngdbuild -p xc7z020clg484-1 -nt timestamp -bm JustButtons.bmm "U:/Desktop/cpre488AudioTracking/system_just_button/implementation/JustButtons.ngc" -uc JustButtons.ucf JustButtons.ngd 

@REM #
@REM # Command line for map
@REM #
map -o JustButtons_map.ncd -w -pr b -ol high -timing -detail JustButtons.ngd JustButtons.pcf 

@REM #
@REM # Command line for par
@REM #
par -w -ol high JustButtons_map.ncd JustButtons.ncd JustButtons.pcf 

@REM #
@REM # Command line for post_par_trce
@REM #
trce -e 3 -xml JustButtons.twx JustButtons.ncd JustButtons.pcf 

