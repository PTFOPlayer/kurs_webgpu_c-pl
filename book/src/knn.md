# KNN

KNN znany również jako K najbliższych sąsiadów. 

Dany jest zbiór punktów, należy znaleźć K najbliższych sąsiadów, dla punktów 2d dystans może być obliczony z poniższego wzoru.

$$
    d = \sqrt{(x_2-x_1)^2 + (y_2-y_1)^2}
$$

Podpowiedź:

Sortowanie czy też szukanie po stronie GPU jest wysoce skomplikowane. Można użyć GPU do znalezienia dystansów dla każdego punktu a następnie znaleźć te K najbliższych.