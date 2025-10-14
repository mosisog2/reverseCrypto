import oracle
import math

m = 0
p = 0
q = 0
N = 0 #public modulus is known
fault = "fault1" #example fault would be like singleflippedbit

(s, s_f) = oracle(m, fault, p, q, N)

p = math.gcd(s-s_f) % N
q = N/p
print ("p: ", p)
print ("q: ", q)
print ("N: ", N)


# Will become more complex with more fault types and perhaps trying against defenses