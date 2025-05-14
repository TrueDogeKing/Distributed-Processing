from PIL import Image
import numpy as np
import multiprocessing
import time
import sys

# Negative of part of the image
def processNegative(chunk):
    return 255 - chunk

# Processes on image
def processImage(imageArray, processNumber):
    # Division into horizontal chanks
    chunks = np.array_split(imageArray, processNumber, axis=0)

    # Create pool process
    with multiprocessing.Pool(processNumber) as pool:
        chunkResult = pool.map(processNegative, chunks)

    # Sum all chunks
    return np.vstack(chunkResult)

if __name__ == "__main__":
    
    if len(sys.argv) < 3:
        print("Uzycie: py main.py <PLIK_OTWIERANY> <PLIK_DOCELOWY> <LICZBA_PROCESÓW>")
        sys.exit(1)

    fileInput = sys.argv[1] 
    fileOutput = sys.argv[2]
    processNumber = int(sys.argv[3])
     
    print(f"[Info] Wczytywanie obrazu z {fileInput}")
    image = Image.open(fileInput).convert("RGB")
    imageArray = np.array(image)

    print("[Info] Rozpoczynam przetwarzanie...")
    timeStart = time.time()

    negativeArray = processImage(imageArray, processNumber)

    timeEnd = time.time()
    timeDuration = timeEnd - timeStart
    print(f"[Sukces] Przetwarzanie zakończone w {timeDuration:.2f} sekundy")

    imageResult = Image.fromarray(negativeArray.astype(np.uint8))
    imageResult.save(fileOutput)
    print(f"[Info] Zapisano wynik do {fileOutput}")