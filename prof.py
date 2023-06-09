import glob
import os.path
import time
import subprocess
from collections import OrderedDict
import matplotlib.pyplot as plt
import sys

file_prefix = sys.argv[1]

file_paths = glob.glob(file_prefix+".*.heap")

heap_dict = {}
min_time = float('inf')

for file_path in file_paths:
    with open(file_path, "r") as file:
        content = file.read()

        # print(f"File: {file_path}")
        # print(content)

        creation_time = os.path.getctime(file_path)
        min_time = min(min_time, creation_time)

for file_path in file_paths:
    with open(file_path, "r") as file:
        content = file.read()

        # print(f"File: {file_path}")
        # print(content)

        creation_time = os.path.getctime(file_path)

        # formatted_creation_time = time.ctime(creation_time)
        # print("File creation time:", creation_time)
        # print("")
        command = "go tool pprof -traces main.o " + file_path
        # print(command)
        result = subprocess.run(command, shell=True, capture_output=True, text=True)
        res_arr = result.stdout.split("-----------+-------------------------------------------------------")
        for i in range(1, len(res_arr)-1):
            print("res_arr["+str(i)+"]: ", res_arr[i].strip())
            single_stack = res_arr[i].strip().split("\n")
            # print("single_stack[0]: ", single_stack[0].strip())
            size = single_stack[0].strip().split(": ")[1]
            call_stack = single_stack[1].strip().split(" ")[1].strip()
            alloc_time = creation_time-min_time
            for call_stack_element in single_stack[2:]:
                call_stack += " " + call_stack_element.strip()
            # print("size: ", size)
            # print("call_stack: ", call_stack)
            if size.endswith("kB"):
                size = float(size[:-2])*1024
            elif size.endswith("MB"):
                size = float(size[:-2])*1024*1024
            elif size.endswith("GB"):
                size = float(size[:-2])*1024*1024*1024
            else:
                size = float(size[:-1])
            if call_stack in heap_dict:
                heap_dict[call_stack][alloc_time] = size
            else:
                od = OrderedDict()
                od[alloc_time] = size
                heap_dict[call_stack] = od

print("heap_dict: ", heap_dict)
index = 0
legend_arr = []
for key, value in heap_dict.items():
    od1 = OrderedDict(sorted(value.items()))
    x = []
    y = []
    for key1, value1 in od1.items():
        x.append(key1)
        y.append(value1)


    plt.xlabel('Time (sec)')
    plt.ylabel('Size (bytes)')
    plt.plot(x, y, linestyle='--', marker='o', label=index)
    
    print(str(index) + ": ", key)
    legend_arr.append((str(index) + ": "+ key))
    index+=1
    
    # plt.savefig(key+".png")

plt.legend(loc='center left', bbox_to_anchor=(1, 0.5))
plt.savefig(file_prefix+".png")

file_path1 = file_prefix+"_legend.txt"
with open(file_path1, 'w') as file:
    # Write each item from the list to the file
    for item in legend_arr:
        file.write(str(item) + '\n')
        