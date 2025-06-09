import numpy as np

def get_amplitude(timeseries: np.array):
    range = np.max(timeseries) - np.min(timeseries)
    return range

def convert_word(word, Vref=5.0, adc_res=15):
    return word/2**(adc_res-1)*Vref
    
def get_timeseries(filename):
    with open(filename, 'rb') as f:
        raw_data = f.read()  # Read all bytes

    data = np.frombuffer(raw_data, dtype='<i2') 
    decimal_data = data.astype(np.int32)
    f = lambda x: x/(2**15)*5
    volt_data = f(decimal_data) 

    return volt_data
