import re
import os

# CPU 0 cummulative IPC: 0.491518 instructions: 100000000 cycles: 203451296
# LLC TOTAL     ACCESS:     373285  HIT:     146976  MISS:     226309
# Final PSEL: 1069	SHiP usage: 448	Multi usage: 403
d_reg = r'\d+\.?\d*'
reg1 = r'CPU \d+ cummulative IPC: \d+.\d+ instructions: \d+ cycles: \d+'
reg2 = r'LLC\s+TOTAL\s+ACCESS:\s+\d+\s+HIT:\s+\d+\s+MISS:\s+\d+'
reg3 = r'Final PSEL: \d+	SHiP usage: \d+	LRU usage: \d+'

datapts = './datapoints'

# baseline
baseline_traces = './output_adaptive'
baseline_subfolders = ['adaptive-config2', 'double_budget']

# test 2
shiplru_traces = './output_adaptive-ship-lru'
shiplru_subfolders = ['adaptive-ship-lru-config2', 'adaptive-ship-lru-m1', 'adaptive-ship-lru-m2', 'adaptive-ship-lru-m3', 'adaptive-ship-lru-m4']

# Source: https://stackoverflow.com/questions/16891340/remove-a-prefix-from-a-string
def remove_prefix(text, prefix):
    if text.startswith(prefix):
        return text[len(prefix):]
    return text

def print_stats(folder, misses, instrs, ipcs, n_ship):
    print("Average MPKI for {}\t= {}".format(folder, misses/instrs * 1000))
    print("Average IPC for {}\t= {}".format(folder, ipcs/51))
    print("Number of times final PSEL was in SHiP range = {}".format(n_ship))
    print

def process_dir(trace_name, subf_name):
    n_traces = 0
    tot_instrs = 0
    tot_misses = 0
    tot_ipc = 0
    n_ship = 0 # number of times final psel is in ship range

    datalines = {}

    # Save data points into files for use in graphing...
    writefile = open(os.path.join(datapts, subf_name), "w")
    #writefile.write("Trace\tMisses\tTotal instructions\tIPC\n")

    # Chop off prefix that states binary, config, sim stats
    # Chop off ".trace.gz.txt"
    for f in os.listdir(os.path.join(trace_name,subf_name)):
        n_traces = n_traces + 1
        s = f.find('s100000000')
        assert (s > -1)
        stripped_name = f[s+11:len(f)-13]
#stripped_name = remove_prefix(f, subf_name + "-config")[23:]
#stripped_name = stripped_name[:len(stripped_name) - 13]
        for line in open(os.path.join(trace_name,subf_name,f)):
            m1 = re.match(reg1, line)
            m2 = re.match(reg2, line)
            m3 = re.match(reg3, line)
            # IPC, num instructions
            if m1 is not None:
                nums = re.findall(d_reg, line)
                ipc = float(nums[1])
                num_instrs = nums[2]
            # misses
            if m2 is not None:
                num_misses = re.findall(d_reg, line)[2]
            # psel
            if m3 is not None:
                psel = int(re.findall(d_reg, line)[0])
                assert (psel >= 0 and psel < 2048)
                if psel >= 1024:
                    n_ship = n_ship + 1

        assert num_misses != 0
        assert num_instrs != 0
        assert ipc != 0
        tot_instrs = tot_instrs + float(num_instrs)
        tot_misses = tot_misses + float(num_misses)
        tot_ipc = tot_ipc + ipc
        datalines[stripped_name] = (int(num_misses), int(num_instrs), ipc, psel)

    ordered_keys = sorted(datalines)
    for key in ordered_keys:
        writefile.write("{}\t{:d}\t{:d}\t{}\t{}\n".format(key, datalines[key][0], datalines[key][1], datalines[key][2], datalines[key][3]))
    writefile.close()
    return n_traces, tot_instrs, tot_misses, tot_ipc, n_ship

#for root, dirs, files in os.walk('traces'):
#    print(files)

# baseline adaptive
#for subf in baseline_subfolders:
#    print("Processing folder:\t" + baseline_traces + "/" + subf)
#    n_traces, total_instrs, total_misses, total_ipc, n_ship = process_dir(baseline_traces, subf)
#    assert n_traces == 51
#    print_stats(subf, total_misses, total_instrs, total_ipc, n_ship)

# shiplru adaptive
for subf in shiplru_subfolders:
    print("Processing folder:\t" + shiplru_traces + "/" + subf)
    n_traces, total_instrs, total_misses, total_ipc, n_ship = process_dir(shiplru_traces, subf)
    assert n_traces == 51
    print_stats(subf, total_misses, total_instrs, total_ipc, n_ship)
