import numpy as np
import matplotlib.pyplot as plt
from decimal import Decimal

with open("results.txt") as results_file:
    results_data = results_file.read()

results_data = results_data.split('\n')
del results_data[-1]

results_data = [float(x) for x in results_data]

plt.hist(results_data)
plt.show()
