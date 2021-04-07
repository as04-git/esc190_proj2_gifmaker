@echo off
echo Compiling...
gcc -fopenmp crop.c seamcarving.c c_img.c -o gif_maker -lm
gif_maker %1
python make_gif.py %2
pause
