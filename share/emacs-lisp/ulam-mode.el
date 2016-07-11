;; A weak first cut at an ulam mode for emacs.  Gurus please help!

(defvar ulam-mode-hook nil)

(defvar ulam-mode-map
  (let ((map (make-sparse-keymap)))
     (define-key map [f1]
       (lambda() (interactive) (message "foo")))
     map)
  "Keymap for `ulam-mode'.")

(define-derived-mode ulam-mode java-mode "ulam"
  "A mode to edit ulam source files."

  (c-lang-defconst c-other-decl-kwds
    ulam ("ulam" "use"))

  (c-lang-defconst c-class-decl-kwds
    ulam '("element" "quark" "union"))

  (c-lang-defconst c-type-list-kwds
    ulam '("use" "element" "quark" "union"))

  (c-lang-defconst c-primitive-type-kwds
    ulam '("Int" "Unsigned" "Void" "Bool" "Unary" "Bits"))

  ;; Try to recognize, e.g.,  Int(17) as a type
  ;;   (Doesn't seem to work, though, because I don't understand the
  ;;   engine or the engine just isn't prepared to understand ulam.
  ;;   The engine has a bunch of weird hardcoded stuff about parens
  ;;   and declarations, that seems to interact badly with ulam's use
  ;;   of parens in decls.  Oh well.)
  (c-lang-defconst c-type-decl-prefix-key
    ulam '("([^)]+)"))  ;; Note: '("(") doesn't work either..

  (c-add-style "std2" '("java"
                        (c-basic-offset . 2)
                        (c-hanging-braces-alist
                         ((substatement-open)
                          (block-close . c-snug-do-while)
                          (extern-lang-open after)
                          (inexpr-class-open after)
                          (inexpr-class-close before)))
                        (c-offsets-alist
                         (substatement-open . 0))
                        ))
  (c-set-style "std2")
)

(add-to-list 'auto-mode-alist '("\\.ulam\\'" . ulam-mode))

(provide 'ulam)
