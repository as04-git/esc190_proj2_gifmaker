import os
import glob
import re
import shutil

# try:
#     import ffmpeg
# except ImportError:
#     print("ffmpeg-python bindings not installed. Installing...")
#     os.system("pip3 install ffmpeg-python")

import ffmpeg

# try:
#     from PIL import Image
# except ImportError:
#     print("Pillow not installed. Installing...")
#     os.system("pip3 install Pillow")

from PIL import Image

import sys

png_dir = os.path.join(os.getcwd(), "cropped_images", "img%d.bmp")

# Get dimensions.
im = Image.open(os.path.join(os.getcwd(), "cropped_images", "img0.bmp"))
width, height = im.size

if len(sys.argv) != 2:
	name = input("name of output file? (include extension) [Leave blank for 'output.mp4']: ")
else:
	name = sys.argv[1]
if not name:
    name = "output.mp4"

print("Making mp4 / whatever using FFMPEG...")

(
    ffmpeg
    .input(png_dir, pattern_type='sequence', framerate=30)
    .output(name, r=30, **{'filter:v': f"scale='min({width},iw)':min'({height},ih)':force_original_aspect_ratio=decrease,pad={width}:{height}:(ow-iw)/2:(oh-ih)/2"}, **{'frames:v': width})
    .run()
)

print(f"Done! See {name} for result")

# option = input("Delete the cropped_images directory? 'y' for Yes, 'n' for no.\n")
# if option == 'y':
#     shutil.rmtree("cropped_images")
# else:
#     print("Ok, not deleting it.")
