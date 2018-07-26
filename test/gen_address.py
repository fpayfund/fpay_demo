
import sys
import string
import random

for i in range(0, 1000):
    print ''.join(random.sample((string.ascii_uppercase+string.digits) * 2, 64))
