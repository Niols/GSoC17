(set-logic QF_S)
(set-info :status sat)
(set-option :strings-exp true)

(declare-fun x () String)
(declare-fun y () String)
(declare-fun z () String)
(declare-fun i () Int)

(assert (>= i 420))
(assert (= x (u16.to.str i)))
(assert (= x (str.++ y "0" z)))
(assert (not (= y "")))
(assert (not (= z "")))



(check-sat)