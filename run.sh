#! /bin/bash +x

echo "Compiling..."
gcc -fopenmp crop.c seamcarving.c c_img.c -o gif_maker -lm
./gif_maker $1
python3 make_gif.py $2
