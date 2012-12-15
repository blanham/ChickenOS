/*
 * Environment garbage collection test
*/

(((lambda (x) (lambda () (define A (lambda (x) #T)))) 1))

(A 1)