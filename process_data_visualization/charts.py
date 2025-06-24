import pandas as pd
import  sys

import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt

# Προσοχη θελει installation pandas


def readcsv(filename):
    df=pd.read_csv(filename)

    tuples=df.apply(lambda row: tuple(row), axis=1)
    return tuples

def creatediagram(x_axis,y_axis,PID,title,x_label,y_label):
    bars = plt.bar(x_axis, y_axis, color='red')
    i = 0
    for bar in bars:
        plt.text(bar.get_x() + bar.get_width() / 2, bar.get_height(),
                 "pid: " + str(PID[i]), ha='center', va='bottom', fontsize=12)

        i = i + 1

    plt.xlabel(x_label)
    plt.ylabel(y_label)

    plt.show()


filename=sys.argv[1]

read=readcsv(filename)
processID=[]
init_values=[] #always in x axis

#always in y axis
turnaround_time=[]
wait_time=[]
computation_time=[]
response_time=[]

for i in range (len(read)):
    processID.append(read[i][0])
    init_values.append(float(read[i][1]))
    turnaround_time.append(float(read[i][2]))
    wait_time.append(float(read[i][3]))
    computation_time.append(float(read[i][4]))
    response_time.append(float(read[i][5]))




#create diagrams

creatediagram(init_values,turnaround_time,processID,"check Turnaround","init values","turnaround time")

creatediagram(init_values,wait_time,processID,"Wait timing","init values","wait_time")

creatediagram(init_values,computation_time,processID,"Computation","init values","Computation_time")

creatediagram(init_values,response_time,processID,"Response","init values","response_time")


