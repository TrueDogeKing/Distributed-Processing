import argparse
import asyncio
import logging
import os
import time
from PIL import Image
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor
from multiprocessing import cpu_count

# Configure logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")

def process_segment(segment_data, segment_index):
    """Processes an image segment – inverts colors to negative."""
    segment = segment_data.copy()
    width, height = segment.size
    logging.info(f"[Process {segment_index}] Started processing segment ({width}x{height})")
    
    for y in range(height):
        for x in range(width):
            r, g, b = segment.getpixel((x, y))
            segment.putpixel((x, y), (255 - r, 255 - g, 255 - b))

    logging.info(f"[Process {segment_index}] Finished processing segment")
    return segment_index, segment

def split_image(image, num_parts):
    """Splits an image into horizontal segments."""
    width, height = image.size
    segment_height = height // num_parts
    segments = []

    for i in range(num_parts):
        top = i * segment_height
        bottom = (i + 1) * segment_height if i != num_parts - 1 else height
        box = (0, top, width, bottom)
        segment = image.crop(box)
        segments.append(segment)

    return segments

def combine_segments(segments):
    """Combines segments into a single image."""
    total_height = sum(seg.size[1] for _, seg in segments)
    width = segments[0][1].size[0]
    final_image = Image.new('RGB', (width, total_height))

    y_offset = 0
    for _, segment in sorted(segments):
        final_image.paste(segment, (0, y_offset))
        y_offset += segment.size[1]

    return final_image

async def main_async(image_path, num_processes):
    image = Image.open(image_path).convert('RGB')
    width, height = image.size
    logging.info(f"[MAIN] Loaded image: {width}x{height}")

    start_time = time.time()
    segments = split_image(image, num_processes)

    loop = asyncio.get_running_loop()
    with ProcessPoolExecutor(max_workers=num_processes) as executor:
        tasks = [
            loop.run_in_executor(executor, process_segment, segment, i)
            for i, segment in enumerate(segments)
        ]
        results = await asyncio.gather(*tasks)

    final_image = combine_segments(results)
    elapsed_time = time.time() - start_time
    logging.info(f"[MAIN] Finished processing in {elapsed_time:.2f} seconds.")

    output_path = image_path.parent / ("negative_" + image_path.name)
    final_image.save(output_path)
    logging.info(f"[MAIN] Saved output to file: {output_path}")

def parse_and_run():
    parser = argparse.ArgumentParser(description="Convert image to negative using asyncio and ProcessPoolExecutor")
    parser.add_argument("image_filename", help="Name of the image file located in the same folder as this script")
    parser.add_argument("-p", "--processes", type=int, default=cpu_count(),
                        help="Number of processes to use (default: number of CPU cores)")
    args = parser.parse_args()

    script_dir = Path(__file__).resolve().parent
    image_path = script_dir / args.image_filename

    if not image_path.exists():
        logging.error(f"File does not exist: {image_path}")
        return

    asyncio.run(main_async(image_path, args.processes))

if __name__ == "__main__":
    parse_and_run()
