#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import pandas as pd
from pandasql import sqldf
import numpy as np

def autolabel(rects):
    """Attach a text label above each bar in *rects*, displaying its height."""
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')


df = pd.read_csv( 'meassurement_partition.csv' )

q = """
    select DISTINCT vectorsize
    from df;
    """
        
label = np.array(sqldf( q )).flatten()

q = """
    select time
    from df
    where algorithm = '__gnu_parallel::partition'
    order by vectorsize ASC;
    """

__gnu_parallel = np.array(sqldf( q )).flatten()/100*1000000000
__gnu_parallel[0] = __gnu_parallel[0]/1000000
__gnu_parallel[1] = __gnu_parallel[1]/10000000
__gnu_parallel[2] = __gnu_parallel[2]/100000000
__gnu_parallel = np.around(__gnu_parallel, 3)

q = """
    select time
    from df
    where algorithm = 'ppartition'
    order by vectorsize ASC;
    """

own_alg = np.array(sqldf( q )).flatten()/100*1000000000
own_alg[0] = own_alg[0]/1000000
own_alg[1] = own_alg[1]/10000000
own_alg[2] = own_alg[2]/100000000
own_alg = np.around(own_alg, 3)

q = """
    select time
    from df
    where algorithm = 'std::partition'
    order by vectorsize ASC;
    """

std = np.array(sqldf( q )).flatten()/100*1000000000
std[0] = std[0]/1000000
std[1] = std[1]/10000000
std[2] = std[2]/100000000
std = np.around(std, 3)

x = np.arange(len(label))  # the label locations
width = 0.30  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width, own_alg, width, label='impl. algorithm')
rects2 = ax.bar(x, __gnu_parallel, width, label='gnu_parallel')
rects3 = ax.bar(x + width, std, width, label='std')


# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('time per element [ns]')

ax.set_xlabel('array size')
#ax.set_xscale('log')
#ax.set_title('Scores by group and gender')
ax.set_xticks(x)
ax.set_xticklabels(label)
ax.spines['right'].set_color("none")
ax.spines['top'].set_color("none")

ax.legend(loc='center right')

autolabel(rects1)
autolabel(rects2)
autolabel(rects3)

fig.tight_layout()

plt.show()

df = pd.read_csv( 'meassurement_select.csv' )

q = """
    select DISTINCT vectorsize
    from df;
    """
        
label = np.array(sqldf( q )).flatten()

q = """
    select time
    from df
    where algorithm = '__gnu_parallel::nth_element'
    order by vectorsize ASC;
    """

__gnu_parallel = np.array(sqldf( q )).flatten()/100*1000000000
__gnu_parallel[0] = __gnu_parallel[0]/1000000
__gnu_parallel[1] = __gnu_parallel[1]/10000000
__gnu_parallel[2] = __gnu_parallel[2]/100000000
__gnu_parallel = np.around(__gnu_parallel, 3)

q = """
    select time
    from df
    where algorithm = 'pquickselect'
    order by vectorsize ASC;
    """

own_alg = np.array(sqldf( q )).flatten()/100*1000000000
own_alg[0] = own_alg[0]/1000000
own_alg[1] = own_alg[1]/10000000
own_alg[2] = own_alg[2]/100000000
own_alg = np.around(own_alg, 3)

q = """
    select time
    from df
    where algorithm = 'std::nth_element'
    order by vectorsize ASC;
    """

std = np.array(sqldf( q )).flatten()/100*1000000000
std[0] = std[0]/1000000
std[1] = std[1]/10000000
std[2] = std[2]/100000000
std = np.around(std, 3)


x = np.arange(len(label))  # the label locations
width = 0.30  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width, own_alg, width, label='impl. algorithm')
rects2 = ax.bar(x, __gnu_parallel, width, label='gnu_parallel')
rects3 = ax.bar(x + width, std, width, label='std')


# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('time per element [ns]')

ax.set_xlabel('array size')
#ax.set_xscale('log')
#ax.set_title('Scores by group and gender')
ax.set_xticks(x)
ax.set_xticklabels(label)
ax.spines['right'].set_color("none")
ax.spines['top'].set_color("none")

ax.legend(loc='center right')

autolabel(rects1)
autolabel(rects2)
autolabel(rects3)

fig.tight_layout()

plt.show()

df = pd.read_csv( 'meassurement_sort.csv' )

q = """
    select DISTINCT vectorsize
    from df;
    """
        
label = np.array(sqldf( q )).flatten()

q = """
    select time
    from df
    where algorithm = '__gnu_parallel::sort'
    order by vectorsize ASC;
    """

__gnu_parallel = np.array(sqldf( q )).flatten()/100*1000000000
__gnu_parallel[0] = __gnu_parallel[0]/1000000
__gnu_parallel[1] = __gnu_parallel[1]/10000000
__gnu_parallel[2] = __gnu_parallel[2]/100000000
__gnu_parallel = np.around(__gnu_parallel, 3)


q = """
    select time
    from df
    where algorithm = 'pquicksort'
    order by vectorsize ASC;
    """

own_alg = np.array(sqldf( q )).flatten()/100*1000000000
own_alg[0] = own_alg[0]/1000000
own_alg[1] = own_alg[1]/10000000
own_alg[2] = own_alg[2]/100000000
own_alg = np.around(own_alg, 3)

q = """
    select time
    from df
    where algorithm = 'std::sort'
    order by vectorsize ASC;
    """

std = np.array(sqldf( q )).flatten()/100*1000000000
std[0] = std[0]/1000000
std[1] = std[1]/10000000
std[2] = std[2]/100000000
std = np.around(std, 3)


x = np.arange(len(label))  # the label locations
width = 0.30  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width, own_alg, width, label='impl. algorithm')
rects2 = ax.bar(x, __gnu_parallel, width, label='gnu_parallel')
rects3 = ax.bar(x + width, std, width, label='std')


# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('time per element [ns]')

ax.set_xlabel('array size')
#ax.set_xscale('log')
#ax.set_title('Scores by group and gender')
ax.set_xticks(x)
ax.set_xticklabels(label)
ax.spines['right'].set_color("none")
ax.spines['top'].set_color("none")

ax.legend(loc='center right')

autolabel(rects1)
autolabel(rects2)
autolabel(rects3)

fig.tight_layout()

plt.show()
'''
labels = ['G1', 'G2', 'G3', 'G4', 'G5']
men_means = [20, 34, 30, 35, 27]
women_means = [25, 32, 34, 20, 25]

x = np.arange(len(labels))  # the label locations
width = 0.35  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width/2, men_means, width, label='Men')
rects2 = ax.bar(x + width/2, women_means, width, label='Women')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Scores')
ax.set_title('Scores by group and gender')
ax.set_xticks(x)
ax.set_xticklabels(labels)
ax.legend()


def autolabel(rects):
    """Attach a text label above each bar in *rects*, displaying its height."""
    for rect in rects:
        height = rect.get_height()
        ax.annotate('{}'.format(height),
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),  # 3 points vertical offset
                    textcoords="offset points",
                    ha='center', va='bottom')


autolabel(rects1)
autolabel(rects2)

fig.tight_layout()

plt.show()
'''