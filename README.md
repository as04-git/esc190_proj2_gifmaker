Sales Pitch
====
Ever been bored and frustrated that the gif_maker code is horribly slow thanks to Python being insufferable? Ever wanted to use that cool project 2 you did for ESC190 to make a good quality seam carving GIF but couldn't because Python is just terribly terrible?

Well, fear no more because I used an ill-spent day googling to make it faster and eventually modified / used / made a converter to BMP in C (to allow it to be read by FFMPEG) and some FFMPEG python binding code for the gif_maker to try to make the gif_maker as fast as it could get (w/o resorting to SIMD). Here's the code if you want to try it out yourself.

It should theoretically be cross-platform as I just modified Brandon's stuff. [*1]

PS: I even made a progress indicator. :D

Instructions
====
1. Install ffmpeg. (https://ffmpeg.org/download.html)

2. Make sure python is in your path (Windows) or python3 (Linux/MacOS). If it's named something else you'd have to rename the call in the relevant `run` shell script. Also ensure that you have `gcc` in your path, too.

3. (if pip is in your path; typically Windows) `pip install Pillow ffmpeg-python`
(if pip3 is in your path; typically *nix) `pip3 install Pillow ffmpeg-python`

4. (optional) Put your images inside the same directory or inside a subdirectory like `img/`. Makes it much easier than having to give absolute or complicated relative paths prone to breakage.

5. Use the png2bin.py to convert your images into the `.bin` format understood by the code. [*2]

6. Paste in your `seamcarving.c` file into the same directory. Obviously not given here to avoid academic dishonesty issues.

7. Use the run command - syntax for Windows and Linux / MacOS given respectively below.

Syntax: `./run [path/to/file.bin] [filename.mp4]` or `bash run.sh [path/to/file.bin] [filename.mp4]` [*3]
Note: Both parameters are optional, the program will ask you for them if you don't provide them.

UPDATE:
1. It also now supports direct passing of the filepath and output name.
2. It now has some light multithreading in the C code itself, thanks to OpenMP! [*4] :DD
3. Refactored the algorithm for efficiency a bit too. (removed the need for an extra for-loop for BMP padding)

Notes
====
[*1] I *did* run it on Ubuntu via WSL and fixed some issues, so it *should* be fine although you never know.

[*2] The input file needs to be a `.bin` which is essentially just raw RGB24 (3 bytes per pixel) in a binary format with no padding or anything. [**] Use the `png2bin.py` file to convert an image to the binary format. It works with most common image formats, not just binary.

[*3] You can use any other extension than mp4 for the output as well as long as FFMPEG supports it.

[*4] FFMPEG is obviously both multithreaded and uses fancy instruction sets like SIMD, AVX, MMX, and whatever else you can name.

[**] The code should be able to be modified relatively simply to work with RGB32 and even RGBA stuff too.
