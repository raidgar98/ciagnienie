#!/usr/bin/python3
import sys
assert len(sys.argv) >= 6

d_in = float(sys.argv[1].replace(",", ".")) #float(input("Podaj średnicę wejściową [mm]: ").replace(",", "."))
assert d_in > 0.0, "Średnica wejściowa musi być większa od 0"
d_out = float(sys.argv[2].replace(",", ".")) #float(input("Podaj średnicę wyjściową [mm]: ").replace(",", "."))
assert d_in > d_out, "Średnica wyjściowa musi być mniejsza niż średnica wejściowa"
p_min = float(sys.argv[3].replace(",", "."))/100.0 #float(input("Podaj minimalny ubytek [%]: ").replace(",", "."))/100.0
assert p_min > 0.0 and p_min < 100.0, "Minimalny ubyte musi być większy od 0 i mniejszy od 100"
p_max = float(sys.argv[4].replace(",", "."))/100.0 #float(input("Podaj maksymalny ubytek [%]: ").replace(",", "."))/100.0
assert p_max > p_min and p_max <= 100.0, "Maksymalny ubytek musi być mniejszy od 100% i większy od mnimalnego ubytku"

def calculate( ub : float ):
	D_in = d_in
	global d_out
	op_ub = 1.0 - ub
	res = list()
	while True:
		if D_in * op_ub < d_out:
			res.append(((D_in - d_out)/D_in));
			return res;
		else:
			res.append(ub);
			D_in = D_in * op_ub;

k_max = len(calculate(p_min))
k_min = len(calculate(p_max))
k = int(sys.argv[5].replace(",", ".")) #int(input("Podaj ilość kroków w jakiej chcesz osiągnąć średnicę {} mm w zakresie < {}; {} >: ".format(d_out, k_min, k_max)).replace(",", "."))
assert k >= k_min and k <= k_max, "Liczba kroków poza zakresem <{}; {}>".format(k_min, k_max)

pre_p = ((p_max - p_min) * float((k-k_min)/(k_max-k_min))) + p_min
pre_p_min = p_min
pre_p_max = p_max#pre_p*1.25
search_delta = 10**(-8)

import threading as th

threads = []
wyniki = []
th_lck = th.Lock()

def incrementer(_pre_p_max, _act_ratio):
	global search_delta
	if _act_ratio + search_delta >= _pre_p_max:
		return False
	else:
		return True

def find(range_min, range_max):
	global th_lck
	a_r = range_min 
	r = 1
	p_r = 2
	while incrementer(range_max, a_r):
		a_r += search_delta
		temp = d_in
		for _ in range(k):
			temp *= (1.0 - a_r)
		r = abs(d_out - temp)
		if r > p_r:
			th_lck.acquire()
			wyniki.append(a_r)
			th_lck.release()
			return
		else:
			p_r = r

import math
per_one_core = (((pre_p_max-pre_p_min))/4.0)
for i in range(4):
	threads.append(th.Thread(target=find, args=(pre_p_min + (i*per_one_core), pre_p_min + ((i+1)*per_one_core),)))
	# # print([pre_p_min + (i*per_one_core), pre_p_min + ((i+1)*per_one_core)])

# print("Calculating...")

for th in threads:
	th.run()


for th in threads:
	if th.is_alive():
		th.join()

wyn  = 20000000000000.0	
act_ratio = 1
th_lck.acquire()
for a_r in wyniki:
	temp = d_in
	for _ in range(k):
		temp *= (1.0 - a_r)
	r = abs(d_out - temp)
	if r < wyn:
		act_ratio = a_r	
		wyn = r

# def wyswietl(rati):
# 	temp = d_in
# 	print("LP\t|\tq[%]\t|\td[mm]")
# 	print("-----------------------------------------")
# 	for i in range(k):
# 		temp = (temp * (1.0 - rati)) #if (i != k-1) else (d_out)
# 		print("{}\t|  {:.4f} %\t|  {:.4f} mm".format(i+1, rati*100.0, temp))


f = open("/tmp/__liczonko", 'w')
f.write(("{:.8f}".format(act_ratio)).replace(".", ","))
f.close()

# for var in wyniki:
# 	print(var,":\n")
# 	wyswietl(var)
# 	print("####"*5)