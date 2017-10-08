;;; arc-mfz.el --- archive-mode support for MFZ archives

;; Based on arc-lzh-exe LHa self-extracting .exe by Kevin Ryde
;; Butchering for MFZ by Dave Ackley
;; Modifications copyright 2017 the Regents of the University of New Mexico
;; Modifications licensed under GPLv3+ 
;;
;; Original arc-lzh-exe.el by Kevin Ryde <user42_kevin@yahoo.com.au>
;; Copyright 2006, 2007, 2008, 2009, 2010, 2015 Kevin Ryde
;;
;; arc-mfz.el is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by the
;; Free Software Foundation; either version 3, or (at your option) any later
;; version.
;;
;; arc-mfz.el is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
;; Public License for more details.
;;
;; You can get a copy of the GNU General Public License online at
;; <http://www.gnu.org/licenses/>.

;; This is code for viewing the contents of MFZ MFM archive files.
;;
;; There's no support for modifying such files, because any
;; modifications would invalidate the MFZ signature anyway.
;;

;;; Install:

;; Put arc-mfz.el somewhere in one of your `load-path' directories, and
;; in your .emacs put
;;
;;     (autoload 'archive-mfz-p              "arc-mfz")
;;     (add-to-list 'magic-mode-alist '(archive-mfz-p . archive-mode))
;;
;; There's autoload cookies below for all this if you install via
;; `M-x package-install' or know how to use `update-file-autoloads'.

;;; Emacsen:
;; Designed for Emacs 22+
;;

;;; Code:
(eval-after-load "arc-mode"
  '(unless (eval-when-compile (fboundp 'archive-mfz-summarize))

     (defadvice archive-find-type (around arc-mfz activate)
       "Recognise MFZ files."
       (if (let ((case-fold-search nil))
             (save-restriction
               (widen)
               (save-excursion
                 (goto-char (point-min))
                 (looking-at "MFZ(1.0)"))))
           (setq ad-return-value 'mfz)
         ad-do-it))

     (add-hook 'arc-mfz-unload-hook
               (lambda ()
                 ;; ad-find-advice not autoloaded, require 'advice it in
                 ;; case it was removed by `unload-feature'
                 (require 'advice)
                 (when (ad-find-advice 'archive-find-type 'around 'arc-mfz)
                   (ad-remove-advice   'archive-find-type 'around 'arc-mfz)
                   (ad-activate        'archive-find-type))))

     (defun archive-mfz-summarize ()
       "Summarize contents of an MFZ file for `archive-mode'."
       ;;
       ;; This code uses `archive-zip-summarize' after narrowing the
       ;; buffer to chew off the MFZ header
       (unwind-protect
           (progn
             (goto-char (point-min))
             (search-forward "MFZ(1.0)")
             (forward-char 1)
             (narrow-to-region (point) (point-max))
             (archive-zip-summarize))
         (progn "foo")))

     (defalias 'archive-mfz-extract 'archive-zip-extract)
     ))

;; this is an autoload instead of (require 'arc-mode) because such a require
;; fails in xemacs 21.4 (the file provides 'archive-mode instead of
;; 'arc-mode)
;;
(autoload 'archive-find-type "arc-mode")

;;;###autoload
(defun archive-mfz-p ()
  "Return non-nil if the current buffer is an MFZ archive file.
This is designed for use as a test in `magic-mode-alist', to
select `archive-mode'.
"
  (let (case-fold-search)
    (and (buffer-file-name) ;; oughtn't be nil normally, but watch out for that
         (string-match "\\.\\(mfz\\|MFZ\\)\\'" (buffer-file-name))
         (save-excursion
           (goto-char (point-min))
           ;; same regexp as in archive-find-type for `lzh-exe'
           (looking-at "MFZ(1.0)")))))

;;;###autoload  (add-to-list 'magic-mode-alist '(archive-mfz-p . archive-mode))

;; LocalWords: MFZ zip

(provide 'arc-mfz)

;;; arc-mfz.el ends here
