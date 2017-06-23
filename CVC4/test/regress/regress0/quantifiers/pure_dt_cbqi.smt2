; COMMAND-LINE: --cbqi --no-check-models
; EXPECT: sat
(set-logic ALL_SUPPORTED)
(set-info :status sat)
(declare-datatypes () ((nat (Suc (pred nat)) (zero))))
(declare-fun y () nat)
(assert (forall ((x nat)) (not (= y (Suc x)))))
(check-sat)
