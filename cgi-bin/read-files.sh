awk 'FNR==1{print "\n<strong>=====",FILENAME,"=====</strong>\n"}; 1;' ../docs/*