# 
# This file will return a signature given an m and a fault (or lack of one)
# We will likely hardcode the p and the q for now but allow for it to be inputed later

# we will use the signer functions in C using the following import
from ctypes import *


# the main function that will be called
def oracle (m, fault, p, q, N):
    p = p
    q = q
    N = N
    s_f = processFaults(fault, m, p, N)
    s_p = calculatePartial(m, p, N)
    s_q = calculatePartial(m, q, N)

    # calculate signature using actual signer
    s = 0
    # calculate faulty signature using actual signer
    s_f = 0
    # generate full signature given the fault
    return (s, s_f)

# helper function to process each fault to change the relevant variable
def processFaults(fault, m, p, N):
    # we will likely want to test multiple different faults so we will process them here given a fault
    
    # parse for fault
    match fault:
        case "fault1":
            p = 1; #change the relevant variable
            print ("fault1")
            
        case "fault2":
            p = 2; #change the relevant variable
            print ("fault2")
    return calculatePartial(m, p, N)

# helper function to calculate a partial signature given the variables
def calculatePartial(m, p, N):
    # will call the actual signer code
    return 0