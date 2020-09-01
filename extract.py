files = [
	open("output.generalized_test_Filippo.txt", 'r').read().split('\n'),
	open("output.generalized_test_Alessio.txt", 'r').read().split('\n'),
	open("output.generalized_test_Ottimo.txt", 'r').read().split('\n')
]

output, results, string = 0, [], ''

for f in files:
	results.append([])
	for line in f:
		if '  Dimensione di Ls: ' in line:
			results[output].append(line.replace('  Dimensione di Ls: ', ''))
	output += 1
			
# print(*results, sep='\n')

for i in range(len(results[0])):
	string += f'{i} & {results[0][i]} & {results[1][i]} & {results[2][i]} \\\\ \n'

print(string)

