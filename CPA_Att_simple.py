import numpy as np

# Define the path to the trace file
base_dir = '/Users/jeries/Downloads/dataset2/'
cleartext_file = f'{base_dir}cleartext.txt'

# Load cleartext and power trace for trace0
cleartexts = np.loadtxt(cleartext_file, dtype=np.uint8)

# Define the AES S-box
aes_sbox = np.array([
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
])

# Hamming weight lookup table (remains the same)
hamming_weights = np.array([bin(x).count('1') for x in range(256)])


# Improved correlation function (remains the same)
def correlate(x, y):
    # Pearson's correlation coefficient implementation
    cov_matrix = np.corrcoef(x, y)
    return cov_matrix[0, 1] if cov_matrix.shape == (2, 2) else 0


def find_edges(clock_trace):
    # Find rising or falling edges; here, we find rising edges
    edges = np.where(np.diff(clock_trace) > 0.5)[0] + 1  # +1 to correct index post-diff
    return edges





# Function to perform the CPA attack
def cpa_attack(tr_data, plaintexts, start, end):
    num_traces = tr_data.shape[0]
    print("num traces = ", num_traces)
    best_guess = 0
    best_correlation = 0
    # Array to hold the maximum correlation for each key guess
    max_correlations = np.zeros(256)
    
    # Focus only on the window of interest
    windowed_trace_data = tr_data[:, start:end]
    #print("tr_data traces = ", tr_data.shape[0])
    # Loop over each key guess
    for key_guess in range(256):        
        # Generate the hypothetical power consumption model for each plaintext
        hypothetical_power = np.array([hamming_weights[aes_sbox[pt ^ key_guess]] for pt in plaintexts])
        
        # Compute the correlation for this key guess within the window
        correlations = np.array([correlate(hypothetical_power, windowed_trace_data[:, i])
                                 for i in range(windowed_trace_data.shape[1])
                                 if not np.isnan(windowed_trace_data[:, i]).any()])
        
        # Find the maximum correlation for this key guess
        max_corr = np.max(np.abs(correlations))
        max_correlations[key_guess] = max_corr
        
        # Update the best guess and correlation if this is the highest we've seen so far
        if max_corr > best_correlation:
            best_correlation = max_corr
            best_guess = key_guess

    return best_guess, best_correlation


# Define the window of interest
start_sample = 0
end_sample = 50000

# Perform CPA on byte 0 of the key
full_key = []
for i in range(16):
    trace_file = f'{base_dir}trace{i}.txt'
    clock_file = f'{base_dir}clock{i}.txt'

    clock_data = np.loadtxt(clock_file, dtype=np.float32)
    trace_data = np.loadtxt(trace_file, dtype=np.float32)  # Make sure the data type matches the contents of the trace file

    print("current data shape is: ",  trace_data.shape[0])

    aligned_traces = np.array([align_trace3(trace_data, find_edges(clock_data[i])) for i in range(len(trace_data))])

    key_byte, correlation = cpa_attack(aligned_traces, cleartexts[:, i], 0, 5000)
    full_key.append(key_byte)
    print(f"Byte {i}: Key Guess = {key_byte}, Correlation = {correlation:.4f}")

# Checksum validation
key_checksum = sum(full_key)
print(f"Calculated checksum: {key_checksum}")
expected_checksum = 1434  # Change this to the checksum provided for your dataset
if key_checksum == expected_checksum:
    print("Checksum validation passed: The recovered key is likely correct.")
else:
    print(f"Checksum validation failed: The calculated checksum does not match the expected checksum of {expected_checksum}.")