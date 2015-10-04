import matplotlib.pyplot as plt
import os
import subprocess
from subprocess import Popen
import time

myPath = os.path.dirname(os.path.realpath(__file__))
numChildrenList = [1, 10]
colors = {
    'grep_thread.c': 'r',
    'grep_proc.c': 'b'
}
styles = {
    'smallFile.txt': '--',
    'bigFile.txt': ':'
}
fig, ax = plt.subplots()


for testFile in ["smallFile.txt", "bigFile.txt"]:
    for testString in ["for (int"]:
        latest = None
        for codeFile in ["tom_werner_hw2_problem1_grep_thread.c", "tom_werner_hw2_problem1_grep_proc.c"]:
            times = []
            for numChildren in numChildrenList:
                start = time.time() * 1000
                command = "cd " + str(myPath) + \
                               " && gcc " + codeFile + " -pthread -std=c99 && " + \
                               "./a.out " + testFile + " '" \
                               + testString + "' -N " + str(numChildren)
                result = Popen(command,shell=True, stdout=subprocess.PIPE).stdout.readlines()
                end = time.time() * 1000

                times.append(end - start)
 
                resultSet = set([x.decode().strip() for x in result])
                if latest != None and resultSet != latest:
                    print(command, result)
                    print("Error", codeFile, testFile, ",", testString, ",", numChildren, latest, resultSet)
                latest = resultSet
            ax.plot(numChildrenList, times, colors[codeFile] + styles[testFile], label = testFile+","+codeFile)
                
                
plt.title("Grep times")
plt.xlabel('Number of children')
plt.ylabel('Time in milliseconds')

# Shrink current axis by 20%
box = ax.get_position()
ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

# Put a legend to the right of the current axis
ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))

#plt.savefig(testDataSet + '.png')
plt.show()

