import asyncio
import hashlib
import logging
import sys
import time
from concurrent.futures import ThreadPoolExecutor

# Konfiguracja logowania
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")

CHARSET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()"

def generate_passwords(length, start_index, step):
    charset_list = list(CHARSET)
    charset_len = len(charset_list)

    total_combinations = charset_len ** length

    for idx in range(start_index, total_combinations, step):
        password = ""
        n = idx
        for _ in range(length):
            password = charset_list[n % charset_len] + password
            n //= charset_len
        yield password

def crack_worker(hash_to_crack, length, start_index, step):
    """
    Worker funkcja do łamania hasła.
    """
    logging.info(f"Thread {start_index} started working.")  # --- LOG  THREAD START  ---

    for password in generate_passwords(length, start_index, step):
        if hashlib.sha256(password.encode()).hexdigest() == hash_to_crack:
            logging.info(f"Thread {start_index} found password: {password}")  # --- LOG PASSWORD FOUND ---
            return password

    logging.info(f"Thread {start_index} finnished with no success.")  # --- LOG END OF THREAD  ---
    return None

async def main():
    if len(sys.argv) != 3:
        print(f"USES: python3 {sys.argv[0]} <HASH> <DŁUGOŚĆ_HASŁA>")
        sys.exit(1)

    hash_to_crack = sys.argv[1]
    length = int(sys.argv[2])

    start_time = time.time()

    loop = asyncio.get_running_loop()

    with ThreadPoolExecutor(max_workers=8) as executor:
        tasks = [
            loop.run_in_executor(executor, crack_worker, hash_to_crack, length, i, 8)
            for i in range(8)
        ]

        done, pending = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)

        for task in done:
            password = task.result()
            if password:
                end_time = time.time()
                print(f"Password forced : {password}")
                print(f"Time of forcing: {end_time - start_time:.2f} s")

                # Anuluj pozostałe taski
                for p in pending:
                    p.cancel()
                return

    print("Password not found.")

if __name__ == "__main__":
    asyncio.run(main())

