;; A weak first cut at an ulam mode for emacs.  Gurus please help!
(require 'cc-mode)
(require 'cc-langs)

(defvar ulam-mode-hook nil)

(defvar ulam-mode-map
  (let ((map (make-sparse-keymap)))
     (define-key map [f1]
       (lambda() (interactive) (message "foo")))
     map)
  "Keymap for `ulam-mode'.")

(c-add-language 'ulam-mode 'java-mode)

(c-lang-defconst c-other-decl-kwds
  ulam '("ulam" "use"))

;; remove "package"
(c-lang-defconst c-ref-list-kwds
  ulam nil)

(c-lang-defconst c-class-decl-kwds
  ulam '("element" "quark" "union" "transient"))

(c-lang-defconst c-type-list-kwds
  ulam '("use" "element" "quark" "union" "transient"))

(c-lang-defconst c-primitive-type-kwds
  ulam '("Int" "Unsigned" "Void" "Bool" "Unary" "Bits" "String"))

(c-lang-defconst c-typedef-kwds
  ulam '("typedef"))

(c-lang-defconst c-typedef-decl-kwds
  ulam (append (c-lang-const c-typedef-decl-kwds)
               '("typedef")))

(c-lang-defconst c-type-modifier-prefix-kwds
  ulam '("constant" "parameter" "local"))

(c-lang-defconst c-modifier-kwds
  ulam '("virtual" "native" "@Override"))

(c-lang-defconst c-operators
  ulam (append '((left-assoc "as")
                 (left-assoc "is"))
               (c-lang-const c-operators)))

(c-lang-defconst c-primary-expr-kwds
  ulam '("Self" "self" "operator"))

(c-lang-defconst c-overloadable-operators
  ulam (c-lang-const c-overloadable-operators c++))

(c-lang-defconst c-recognize-colon-labels
  ulam nil)

;; Angle brackets for template arguments are hardcoded into CC mode,
;; so we might as well disable this feature.
;; The mode seems to recognize template arguments in parens though,
;; while still correctly identifying class declaration as such, not
;; a function declaration after removing ":" and "+" from
;; `c-block-prefix-disallowed-chars`.
;;
;; Base classes in class template declaration are not highlighted:
;; cc-engine.el handles C++ and Java cases in special way, see
;; `c-guess-basic-syntax`.
;;
(c-lang-defconst c-recognize-<>-arglists
  ulam nil)
(c-lang-defconst c-block-prefix-disallowed-chars
  ulam (cl-set-difference (c-lang-const c-block-prefix-disallowed-chars)
                           '(?: ?+)))

;; remove "enum"
(c-lang-defconst c-brace-list-decl-kwds
  ulam nil)

;; Recognizing template types, e.g. Int(17).
;; Template arguments are limited to comma-separated list of \w characters
;; to avoid matching function calls and definitions:
;; * foo((Type) a);
;; * Type bar(Type a, Type b);
(c-lang-defconst c-opt-type-suffix-key
;; ulam "\\(([^()]+)\\)")
  ulam (concat "\\(("
               "\\w+"
               "\\(,\\s-*\\w+\\)*"
               ")\\)"))

(defconst ulam-font-lock-keywords-1
  (c-lang-const c-matchers-1 ulam))

(defconst ulam-font-lock-keywords-2
  (c-lang-const c-matchers-2 ulam))

(defconst ulam-font-lock-keywords-3
  (c-lang-const c-matchers-3 ulam))

;; Reference types
(c-lang-defconst c-type-decl-prefix-key
  ulam "\\(&\\)")

(define-derived-mode ulam-mode java-mode "ulam"
  "A mode to edit ulam source files."

  (c-initialize-cc-mode t)
  (c-init-language-vars ulam-mode)
  (c-common-init 'ulam-mode)
  (run-mode-hooks 'c-mode-common-hook)
  (c-update-modeline)

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

(provide 'ulam-mode)
