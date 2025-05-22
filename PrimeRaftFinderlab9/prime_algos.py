import time
import random

def is_prime_trial(n):
    if n < 2:
        return False
    for i in range(2, int(n**0.5) + 1):
        if n % i == 0:
            return False
    return True

def find_prime_trial_division(start):
    while True:
        if is_prime_trial(start):
            return start
        start += 1

def find_prime_fermat(start, k=5):
    def is_probably_prime(n):
        if n <= 1:
            return False
        if n <= 3:
            return True  # 2 i 3 są pierwsze, unikamy pustego zakresu
        for _ in range(k):
            a = random.randint(2, n - 2)
            if pow(a, n - 1, n) != 1:
                return False
        return True

    while True:
        if is_probably_prime(start):
            return start
        start += 1


def find_prime_miller_rabin(start, k=5):
    def is_probable_prime(n):
        if n <= 1:
            return False
        if n <= 3:
            return True  # obsługa małych liczb
        r, s = 0, n - 1
        while s % 2 == 0:
            r += 1
            s //= 2
        for _ in range(k):
            a = random.randrange(2, n - 1)
            x = pow(a, s, n)
            if x in (1, n - 1):
                continue
            for _ in range(r - 1):
                x = pow(x, 2, n)
                if x == n - 1:
                    break
            else:
                return False
        return True

    while True:
        if is_probable_prime(start):
            return start
        start += 1
