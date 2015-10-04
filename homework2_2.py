import matplotlib.pyplot as plt
import os
import subprocess
from subprocess import Popen
import time

myPath = os.path.dirname(os.path.realpath(__file__))
numChildrenList = [5]
colors = {
    'tom_werner_hw2_problem1_grep_thread.c': 'r',
    'tom_werner_hw2_problem1_grep_proc.c': 'b'
}
fileSizes = [107, 3236749, 6473499]
fig, ax = plt.subplots()

for codeFile in ["tom_werner_hw2_problem1_grep_thread.c", "tom_werner_hw2_problem1_grep_proc.c"]:
    times = []
    for testFile in ["smallFile.txt", "mediumFile.txt", "bigFile.txt"]:
        for testString in ["for (int"]:
            for numChildren in numChildrenList:
                start = time.time() * 1000
                command = "cd " + str(myPath) + \
                               " && gcc " + codeFile + " -pthread -std=c99 && " + \
                               "./a.out " + testFile + " '" \
                               + testString + "' -N " + str(numChildren)
                result = Popen(command,shell=True, stdout=subprocess.PIPE).stdout.readlines()
                end = time.time() * 1000

                times.append(end - start)
    ax.plot(fileSizes, times, colors[codeFile], label = testFile+","+codeFile[24:])
                
                
plt.title("Grep times")
plt.xlabel('File size (bytes)')
plt.ylabel('Time in milliseconds')

# Shrink current axis by 20%
box = ax.get_position()
ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

# Put a legend to the right of the current axis
ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))

#plt.savefig(testDataSet + '.png')
plt.show()

