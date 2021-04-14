import ctypes
import imageio
# https://stackoverflow.com/questions/13294919/can-you-stream-images-to-ffmpeg-to-construct-a-video-instead-of-saving-them-t

if len(sys.argv) != 2:
	name = input("name of output file? (include extension) [Leave blank for 'output.mp4']: ")
else:
	name = sys.argv[1]
if not name:
    name = "output.mp4"


writer = imageio.get_writer('video.avi', fps=fps)
for i in range(frames_per_second * video_duration_seconds):
    img = createFrame(i)
    writer.append_data(img)
writer.close()

from PIL import Image
from PIL import ImageFile
ImageFile.LOAD_TRUNCATED_IMAGES = True

def write_image(image, filename):
    height = image.height
    width = image.width

    f = open(filename, "wb")

    f.write(height.to_bytes(2, byteorder='big'))
    f.write(width.to_bytes(2, byteorder='big'))
    img_raster = []
    for i in range(height):
        for j in range(width):
            img_raster.extend(image.getpixel((j, i))[:3])

    f.write(bytearray(img_raster))
    f.close()