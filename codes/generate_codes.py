# ------------------------------------------------------------------------------------------------ #

import numpy as np
import pyntbci

# ------------------------------------------------------------------------------------------------ #

code = pyntbci.stimulus.make_m_sequence(poly = [1, 0, 0, 0, 0, 1], base = 2, seed = [1, 1, 1, 1, 1, 1])[0, :]

# ------------------------------------------------------------------------------------------------ #

codes = np.zeros(shape = (code.size, code.size), dtype = 'uint8')

# ------------------------------------------------------------------------------------------------ #

for i in range(code.size):

    codes[i, :] = np.roll(code, i)

np.savez(file = 'mseq_61_shift.npz', codes = codes)

np.savetxt(fname = 'mseq_61_shift.txt', X = codes, fmt = '%d', delimiter = ',')

# ------------------------------------------------------------------------------------------------ #

codes = pyntbci.stimulus.make_gold_codes(poly1 = [1, 0, 0, 0, 0, 1], poly2 = [1, 1, 0, 0, 1, 1], seed1 = [1, 1, 1, 1, 1, 1], seed2 = [1, 1, 1, 1, 1, 1])

np.savez(file = 'gold_61_6521.npz', codes = codes)

np.savetxt(fname = 'gold_61_6521.txt', X = codes, fmt = '%d', delimiter = ',')

# ------------------------------------------------------------------------------------------------ #

codes = pyntbci.stimulus.modulate(codes)

np.savez(file = 'mgold_61_6521.npz', codes = codes)

np.savetxt(fname = 'mgold_61_6521.txt', X = codes, fmt = '%d', delimiter = ',')

# ------------------------------------------------------------------------------------------------ #