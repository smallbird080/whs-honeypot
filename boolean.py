from sympy import symbols, And, Or, simplify_logic
import sys

class Term:
    def __init__(self, term_num, num_of_vars):
        self.num = term_num
        self.binary = '0'*(num_of_vars-len(bin(term_num)[2:])) + bin(term_num)[2:]
        self.used = False
        self.dont_care = False
        self.ones = self.binary.count('1')
        self.minterms = [term_num]

    def set_dont_care(self):
        self.dont_care = True

    def __str__(self):
        return str(self.binary)
    
    def __and__(self, other):
        new_term = Term(self.num, len(self.binary))
        new_term.binary = ''
        for i in range(len(self.binary)):
            if self.binary[i] == other.binary[i]:
                new_term.binary += self.binary[i]
            else:
                new_term.binary += '-'
        new_term.ones = new_term.binary.count('1')
        new_term.minterms = self.minterms + other.minterms
        new_term.used = False
        self.used = True
        other.used = True
        new_term.minterms.sort()
        return new_term
    
    def __hash__(self):
        return hash(self.binary)

    def __eq__(self, other):
        return self.binary == other.binary
    
    def difference(self, other):
        diff = 0
        for i in range(len(self.binary)):
            if self.binary[i] != other.binary[i]:
                diff += 1
        return diff
    
    def is_used(self):
        return self.used

    def to_variable(self):
        variables = ""
        for i in range(len(self.binary)):
            if self.binary[i] == '0':
                variables += chr(65+i) + "'"
            elif self.binary[i] == '1':
                variables += chr(65+i)
        return variables


def make_P_dict(minterms, prime_implicants):
    P_dict = {}
    for i in range (len(prime_implicants)):
        P_str = "P"+str(i)
        P_dict[prime_implicants[i]] = P_str
    return P_dict

def make_P(minterms, prime_implicants):
    P_dict = make_P_dict(minterms, prime_implicants)
    P_list = []
    Symbol_list = []
    for minterm in minterms:
        P_list.append([])
        for prime_implicant in prime_implicants:
            if minterm in prime_implicant.minterms:
                P_list[-1].append(P_dict[prime_implicant])
    return P_list

def count_alphabets(s):
    return sum(c.isalpha() for c in s)

num_of_vars = int(sys.argv[1])
minterms = list(map(int, sys.argv[2].split()))
dont_cares = list(map(int, sys.argv[3].split()))
minterms = [int(i) for i in minterms]
dont_cares = [int(i) for i in dont_cares]

if(len(minterms) == 0):
    print("false")
    exit()

if(len(minterms) == 2**num_of_vars):
    print("true")
    exit()

terms = [Term(i, num_of_vars) for i in minterms]
for i in range(len(dont_cares)):
    terms.append(Term(dont_cares[i], num_of_vars))
    terms[-1].set_dont_care()

for i in dont_cares:
    minterms.remove(i)

group = []
for i in range(num_of_vars+1):
    group.append([])
    for term in terms:
        if term.ones == i:
            group[i].append(term)

prime_implicants = []

while True:
    new_group = []
    for i in range(len(group)-1):
        new_group.append([])
        for term in group[i]:
            for other in group[i+1]:
                if term.difference(other) == 1:
                    new_group[i].append(term & other)
    for i in range(len(group)):
        for term in group[i]:
            if not term.is_used():
                prime_implicants.append(term)
    group = new_group
    if len(group) == 1:
        break

prime_implicants = list(set(prime_implicants))
prime_implicants.sort(key=lambda x: x.ones)
print("Prime Implicants: ")
for term in prime_implicants:
    print(term.to_variable(), end=" + ")
print()

P_dict = make_P_dict(minterms, prime_implicants)
P_list = make_P(minterms, prime_implicants)
inverted_P_dict = {value: key for key, value in P_dict.items()}

essential_PIs = []
for implicant in P_list:
    if len(implicant) == 1:
        essential_PIs.append(inverted_P_dict[implicant[0]])
essential_PIs = list(set(essential_PIs))

print("Essential Prime Implicants: ")
for implicant in essential_PIs:
    print(implicant.to_variable(), end=" , ")
print()

symbol_list = []
for strlist in P_list:
    symbol_list.append(symbols(strlist))

for item in P_dict.keys():
    print(P_dict[item], end=" : ")
    print(item.to_variable(), end=" / ")
    print()

or_list = []
for symlist in symbol_list:
    or_list.append(Or(*symlist))

final_expression = And(*or_list)

print("Petrick's Method: ")
result = str(simplify_logic(final_expression, form='dnf'))
result = result.replace(" & ", " ")
result = result.replace(" | ", " + ")
print(result)

parts = result.replace('(', '').replace(')', '').split('+')

all_cases = [part.strip().split(' ') for part in parts]
min = 100000
strings = []
for i in range(len(all_cases)):
    result_str = ""
    for j in range(len(all_cases[i])):
        if(j == len(all_cases[i])-1):
            result_str += inverted_P_dict[all_cases[i][j]].to_variable() + " / "
        else:
            result_str += inverted_P_dict[all_cases[i][j]].to_variable() + " + "
    strings.append(result_str)
    if(count_alphabets(result_str) <= min):
        min = count_alphabets(result_str)

print("Minimum SOP: ")
for string in strings:
    if(count_alphabets(string) == min):
        print(string)