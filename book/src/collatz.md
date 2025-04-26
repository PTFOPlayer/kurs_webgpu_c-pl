# Collatz

> Ten problem jest rozwiązany jako jeden z przykładów w repozytorium.

Hipoteza collatza jest bardzo prostym problemem. Weźmy dowolną liczbę n, według hipotezy jeśli dla $n$ parzystego $n_{i+1} = \frac{n_i}{2}$ a dla $n$ nieparzystego $n_{i+1} = 3n_i + 1$ jego definicja matematyczna jest następująca:

$$
    \forall n_i > 1 \\
    n_{i+1} = \begin{cases}
        \frac{n_i}{2}, & \in n_i \bmod 2 = 0 \\
        3n_i + 1, & \in n_i \bmod 2 = 1
    \end{cases}
$$

Podpowiedź:

Petla `loop` w tym wypadku jest lepiej optymalizowana niż pętla `for` ponieważ jest "bardziej ogólna" i dzięki temu nie ma znaczenia czy następny przebieg tej pętli jest wykonywany na tym samym rdzeniu.
