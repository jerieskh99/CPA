import numpy as np

base_dir = '/Users/jeries/Downloads/dataset2/'


def load_data(file_name):
    """Load data from a file."""
    return np.loadtxt(file_name, dtype=np.float32)


def filter_trace_data2(trace_data, clock_data, low_threshold=0.2, high_threshold=0.8):
    """Filter trace data points based on clock signal stability, removing unstable points.

    Args:
        trace_data (np.ndarray): Array of trace data, where each row corresponds to a trace.
        clock_data (np.ndarray): Array of clock data, where each row corresponds to a clock trace.
        low_threshold (float): Threshold below which a clock value is considered unstable.
        high_threshold (float): Threshold above which a clock value is considered stable.

    Returns:
        list: List of numpy arrays, each containing filtered trace data with unstable points removed.
    """
    filtered_traces = []

    # Processing each trace line
    for i in range(clock_data.shape[0]):
        stable_indices = np.where((clock_data[i] <= low_threshold) | (clock_data[i] >= high_threshold))[0]
        # Appending only stable data points to the list
        filtered_traces.append(trace_data[i, stable_indices])

    return filtered_traces


def filter_trace_data(trace_data, clock_data, low_threshold=0.2, high_threshold=0.8):
    """Filter trace data points based on clock signal stability.

    Args:
        trace_data (np.ndarray): Array of trace data, where each row corresponds to a trace.
        clock_data (np.ndarray): Array of clock data, where each row corresponds to a clock trace.
        low_threshold (float): Threshold below which a clock value is considered unstable.
        high_threshold (float): Threshold above which a clock value is considered stable.

    Returns:
        np.ndarray: Filtered trace data, where unstable points are removed.
    """
    # Filtered traces initialization with None values or could use np.nan for identification
    filtered_traces = np.full_like(trace_data, None, dtype=np.float32)

    # Processing each trace line
    for i in range(clock_data.shape[0]):
        stable_indices = np.where((clock_data[i] <= low_threshold) | (clock_data[i] >= high_threshold))[0]
        filtered_traces[i, stable_indices] = trace_data[i, stable_indices]

    return filtered_traces


for index in range(16):
    # Example usage
    trace_file = f'{base_dir}trace{index}.txt'  # Replace N with actual file index
    clock_file = f'{base_dir}clock{index}.txt'  # Replace N with actual file index

    trace_data = load_data(trace_file)
    clock_data = load_data(clock_file)

    # Filter the trace data based on clock data
    filtered_trace_data = filter_trace_data(trace_data, clock_data)


    # Optionally, save the filtered data
    np.savetxt(f'{base_dir}filtered_trace{index}.txt', filtered_trace_data, fmt='%f')
