import argparse
import logging
import time
from threading import Thread
from PIL import Image
import numpy as np
from pathlib import Path

logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")

def process_segment(array, start_row, end_row, index):
    segment_shape = array[start_row:end_row].shape
    total_pixels = np.prod(segment_shape)

    logging.info(
        f"[Thread {index}] Started processing segment with shape {segment_shape} "
        f"from row {start_row} to {end_row}, total pixels: {total_pixels}"
    )

    array[start_row:end_row] = 255 - array[start_row:end_row]

    logging.info(f"[Thread {index}] Finished processing segment")

def main(image_path: Path, num_threads: int):
    logging.info("[MAIN] Opening image...")
    image = Image.open(image_path).convert('RGB')
    array = np.array(image)
    height = array.shape[0]
    step = height // num_threads
    threads = []

    logging.info("[MAIN] Starting processing...")
    start_time = time.time()

    for i in range(num_threads):
        start = i * step
        end = (i + 1) * step if i != num_threads - 1 else height
        thread = Thread(target=process_segment, args=(array, start, end, i))
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()

    duration = time.time() - start_time
    logging.info(f"[MAIN] Finished processing in {duration:.4f} seconds.")

    result_image = Image.fromarray(array)
    output_path = image_path.parent / f"negative_threads_{image_path.name}"
    result_image.save(output_path)
    logging.info(f"[MAIN] Saved output to {output_path}")

def parse_args():
    parser = argparse.ArgumentParser(description="In-place image negative using threading.")
    parser.add_argument("image_filename", help="Input image filename.")
    parser.add_argument("-t", "--threads", type=int, default=4,
                       help="Number of threads to use (default: 4).")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    script_dir = Path(__file__).resolve().parent
    image_path = script_dir / args.image_filename

    if not image_path.exists():
        logging.error(f"File not found: {image_path}")
    else:
        main(image_path, args.threads)