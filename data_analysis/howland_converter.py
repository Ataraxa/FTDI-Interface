import numpy as np

# numbers = [
#     25, 50, 75, 100,
#     225, 250, 275,
#     475, 500, 525,
#     975, 1000, 1025,
#     1475, 1500, 1525,
#     2475, 2500, 2525
# ]

numbers = [3, 3.5, 4, 4.5, 4.9999]

for number in numbers:
    number = -number
    code = ((number + 5) * 2**15)/5
    code = round(code)
    code = np.uint16(code)
    hex_16bit = f"0x{code:04X}"
    print(hex_16bit)