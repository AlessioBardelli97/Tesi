files = [
	open('Risultati sperimentali - output_aggregato(filippo, alessio, ottimo).tsv', 'r').read().split('\n'),
	open('Risultati sperimentali - output_aggregato(zero, one, filippo_migl).tsv', 'r').read().split('\n'),
	open('Risultati sperimentali - output_indipendenti.tsv', 'r').read().split('\n')
]

for line in files[2][:-1]:
		line = line.split('\t')
		for i in range(1, 7):
			try:
				line[i] = int(line[i])
			except:
				line[i] = -1
		toPrint = True
		for i in line[1:-1]:
			if i >= line[-1]:
				toPrint = False
		if toPrint:
			print(line)
			

