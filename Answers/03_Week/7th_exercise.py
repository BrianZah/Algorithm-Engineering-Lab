#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import threading

class Thread(threading.Thread):
    def __init__(self, thread_id, num_threads, num_steps):
        threading.Thread.__init__(self)
        self.thread_id = thread_id
        self.num_steps = int(num_steps/num_threads)
        self.width = 1/num_steps
    def run(self):
        print("Starting Thread " + str(self.thread_id))
        thunk(self.thread_id, self.num_steps, self.width)
        print("Exiting Thread " + str(self.thread_id))

def thunk(thread_id, num_steps, width):
    global pi_quarter, lock
    localsum = 0
    start = int(thread_id * num_steps)
    end = int((thread_id+1) * num_steps)
    
    for i in range(start, end):
        x = (i + 0.5) * width
        localsum += (1 / (1 + x*x))
        
    lock.acquire()
    pi_quarter += localsum
    lock.release()

num_threads = 8
num_steps = 1000000

pi_quarter = 0

lock = threading.Lock()
threads = []

for thread_id in range(1,num_threads):
    thread = Thread(thread_id , num_threads, num_steps)
    thread.start()
    threads.append(thread)
    
thunk(0, num_steps/num_threads, 1/num_steps)    

for thread in threads:
    thread.join()

print("Pi = " + str(4*pi_quarter/num_steps))

print("Exiting Main Thread")