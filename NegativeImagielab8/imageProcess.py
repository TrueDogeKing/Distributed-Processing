import argparse
import logging
import time
from multiprocessing import Process, shared_memory, cpu_count
from PIL import Image
import numpy as np
from pathlib import Path

logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")

def process_segment(shm_name, shape, dtype, start_row, end_row, index):
    shm = shared_memory.SharedMemory(name=shm_name)
    array = np.ndarray(shape, dtype=dtype, buffer=shm.buf)
    segment_shape = array[start_row:end_row].shape
    total_pixels = np.prod(segment_shape)

    logging.info(
        f"[Process {index}] Started processing segment with shape {segment_shape} "
        f"from row {start_row} to {end_row}, total pixels: {total_pixels}"
    )

    array[start_row:end_row] = 255 - array[start_row:end_row]

    logging.info(f"[Process {index}] Finished processing segment")
    shm.close()


def main(image_path: Path, num_processes: int):
    logging.info("[MAIN] Opening image...")
    image = Image.open(image_path).convert('RGB')
    array = np.array(image)
    shape = array.shape
    dtype = array.dtype

    shm = shared_memory.SharedMemory(create=True, size=array.nbytes)
    shared_array = np.ndarray(shape, dtype=dtype, buffer=shm.buf)
    np.copyto(shared_array, array)

    height = shape[0]
    step = height // num_processes
    processes = []

    logging.info("[MAIN] Starting processing...")
    start_time = time.time()

    for i in range(num_processes):
        start = i * step
        end = (i + 1) * step if i != num_processes - 1 else height
        p = Process(target=process_segment, args=(shm.name, shape, dtype, start, end, i))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()

    duration = time.time() - start_time
    logging.info(f"[MAIN] Finished processing in {duration:.2f} seconds.")

    result_image = Image.fromarray(shared_array)
    output_path = image_path.parent / f"negative_{image_path.name}"
    result_image.save(output_path)
    logging.info(f"[MAIN] Saved output to {output_path}")

    shm.close()
    shm.unlink()

def parse_args():
    parser = argparse.ArgumentParser(description="In-place image negative using shared memory and multiprocessing.")
    parser.add_argument("image_filename", help="Input image filename (must be in the same directory as script).")
    parser.add_argument("-p", "--processes", type=int, default=cpu_count(),
                        help="Number of processes to use (default: number of CPU cores).")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    script_dir = Path(__file__).resolve().parent
    image_path = script_dir / args.image_filename

    if not image_path.exists():
        logging.error(f"File not found: {image_path}")
    else:
        main(image_path, args.processes)