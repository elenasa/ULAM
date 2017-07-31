;; C-x C-k 1
;; wipes from point (usually top of buffer) to the test name, inserts #=
;; wipes to the answer key, inserts #!
;; skips answer key
;; wipes to the first '->add' (which might be commented!  that has to be pre-handled before doing this!)
;; converts to #>
;; leaves point after first file
(fset 'do-test-top
   [?# ?# ?  ?- ?* ?- ?  ?m ?o ?d ?e ?: ?u ?l ?a ?m ?  ?- ?* ?- ?\C-m ?\C-  ?\C-s ?B ?E ?G ?I ?N ?\M-f ?\C-f ?\C-w ?\C-e ?\C-? ?\C-a ?# ?= ?\C-n ?\C-a ?\C-  ?\C-s ?g ?e ?t ?a ?n ?\C-e ?\C-w ?\C-  ?\C-s ?r ?e ?t ?u ?r ?n ?  ?s ?t ?d ?: ?: ?s ?t ?r ?i ?n ?g ?\C-f ?\C-w ?# ?! ?\C-o ?\C-n ?\C-a ?\C-d ?\C-e ?\C-  ?\C-r ?\" ?\C-w ?\C-m ?\C-w ?\C-b ?\C-b ?\C-d ?\C-d ?\C-n ?\C-a ?\C-  ?\C-s ?- ?> ?a ?d ?d ?\C-f ?\C-f ?\C-w ?$ ?\C-? ?# ?> ?\C-s ?\" ?\C-b ?\C-d ?\C-m ?\C-  ?\C-s ?\" ?\C-m ?\C-w ?\C-e ?\C-r ?\" ?\C-  ?\C-k ?\C-n ?\C-a])
(global-set-key  (kbd "C-x C-k 1") 'do-test-top)

;; C-x C-k 2
;; puts a '##' at the beginning of the current line, moves to next line
(fset 'add-cmt
   "\C-a##\C-e\C-f")
(global-set-key  (kbd "C-x C-k 2") 'add-cmt)

;; C-x C-k 3
;; wipes to next fms->add (commented ones must have already been removed),
;; converts to #:
;; leaves point after file
(fset 'do-system
   [?# ?: ?\C-s ?a ?d ?d ?\( ?\" ?\C-m ?\C-w ?\M-f ?\M-f return ?\C-  ?\M-f ?\M-b ?\C-w ?\C-e backspace backspace backspace backspace backspace ?\C-n])
(global-set-key  (kbd "C-x C-k 3") 'do-system)

;; C-x C-k 4
;; inserts #. at point, wipes to end of file, saves
(fset 'finishit
   [?# ?. ?\C-n ?\C-a ?\C-  ?\M-> ?\C-w ?\C-x ?\C-s])
(global-set-key  (kbd "C-x C-k 4") 'finishit)

;; C-x C-k 5
;; replaces all \n's with newlines, 
;; either in the current region (if there is one)
;; or from point to end of buffer (if not)
(fset 'rep-bslash-n
   [?\M-x ?r ?e ?p ?l ?  ?s return ?\\ ?n return ?\C-q ?\C-j return ?\C-x])
(global-set-key  (kbd "C-x C-k 5") 'rep-bslash-n)
