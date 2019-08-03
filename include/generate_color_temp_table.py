#!/usr/bin/env python

import math


f= open("color_temp.h","w+")
f.write("const byte color_temp_table[][3] = \n{")
f.write("//{amber, cool white, warm white} color values respectively\n")

amber_start = -256
amber_stop = 256
amber_max = 255
amber_width = 23

ww_start = 0
ww_stop = 512
ww_max = 130
ww_width = 27

cw_start = 256
cw_stop = 768
cw_max = 255
cw_width = 30

def bell_curve(start, stop, max, width, position):
    return str(int(math.floor(math.exp(-math.pow(position-((start + stop)/2),2)/(width*(stop-start))) * max)))

for i in range(512):
    line = "{"
    line += bell_curve(amber_start, amber_stop, amber_max, amber_width, i) + ", "
    line += bell_curve(cw_start, cw_stop, cw_max, cw_width, i) + ", "
    line += bell_curve(ww_start, ww_stop, ww_max, ww_width, i)
    if(i < 511):
        line += "},\n"
    else:
        line += "}\n"

    f.write(line)

f.write("};")
f.close()
