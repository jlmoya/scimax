;;    <SciMax, a Scilab toolbox to connect Maxima.>
;;    Copyright (C) <2009>  <Calixte DENIZET>
;;
;;    This program is free software: you can redistribute it and/or modify
;;    it under the terms of the GNU General Public License as published by
;;    the Free Software Foundation, either version 3 of the License, or
;;    (at your option) any later version.
;;
;;   This program is distributed in the hope that it will be useful,
;;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;   GNU General Public License for more details.
;;
;;   You should have received a copy of the GNU General Public License
;;   along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;
;;
;;   Contact : Calixte DENIZET <calixte.denizet@ac-rennes.fr>

(defun sm_getenv (name &optional default)
  #+CMU
  (let ((x (assoc name ext:*environment-list*
		  :test #'string=)))
    (if x (cdr x) default))
  #-CMU
  (or
   #+GCL (sys:getenv name) 
   #+Allegro (sys:getenv name)
   #+CLISP (ext:getenv name)
   #+ECL (si:getenv name)
   #+SBCL (sb-unix::posix-getenv name)
   #+LISPWORKS (lispworks:environment-variable name)
   default))

;; The environment variable SCIMAX_TOOLBOX_PATH is set by Scilab in the file etc/SciMax.start

;; macOS/2027 port: recompile-and-retry if the prebuilt .fasl files will not load.
;;
;; A .fasl is locked to the exact Lisp build that produced it. SBCL refuses one
;; with "is a fasl file compiled with SBCL 2.6.5-85913ede1, and can't be loaded
;; into SBCL 2.6.6" -- so a routine `brew upgrade sbcl` (or a Maxima upgrade that
;; pulls a new SBCL) silently breaks a working scimax install. The only symptom
;; the user ever sees is maxinit.c's "Maxima started but could not load
;; scimax/scimath", which names neither the fasl nor the version skew. That is
;; exactly how this broke here: fasls built 2026-07-12 against 2.6.5 stopped
;; loading once 2.6.6 arrived.
;;
;; The .lisp sources are version independent, so recompiling recovers without
;; any manual step. Compiler chatter is muted because this file is loaded OVER
;; the Scilab<->Maxima pipe, whose framing depends on exact "<EO>" markers --
;; stray warnings on stdout would corrupt the wire, turning a self-heal into a
;; worse failure than the one it fixes. If recompiling is impossible too (a
;; read-only install, say), the original message is still what comes out.
(let ((path (concatenate 'string (sm_getenv "SCIMAX_TOOLBOX_PATH") "/src/lisp/")))
  (handler-case
   (progn
     (load (concatenate 'string path "scimath"))
     (load (concatenate 'string path "scimax")))
   (error ()
     (handler-case
      (progn
        (let ((*standard-output* (make-broadcast-stream))
              (*error-output*    (make-broadcast-stream)))
          (compile-file (concatenate 'string path "scimath.lisp")
                        :verbose nil :print nil)
          (compile-file (concatenate 'string path "scimax.lisp")
                        :verbose nil :print nil))
        (load (concatenate 'string path "scimath"))
        (load (concatenate 'string path "scimax")))
      (error () (format t "Files scimath and scimax cannot be loaded~%<BD>~%"))))))
(format t "Files scimath and scimax loaded~%")

;; macOS/2027 port (Task 12): activate the <EO> wire prompt HERE, at load
;; time, and make it flush.
;;
;; Upstream relied on maxima's "-p loader.lisp" preload: scimax.lisp's
;; redefined macsyma-top-level (whose body installs the <EO> main-prompt)
;; was in place BEFORE cl-user::run invoked the top level, so the REPL that
;; ran was the redefined one. This port cannot use -p (with a piped stdin,
;; Maxima 5.49/SBCL crashes into the Lisp debugger during startup whenever
;; -p is passed -- see src/c/maxinit.c) and loads this file over the pipe
;; instead; a macsyma-top-level redefinition performed by the ALREADY
;; RUNNING stock top level never takes effect, because nothing ever
;; re-enters macsyma-top-level. Net effect: main-prompt stayed stock, the
;; "\n<EO>\n" frame terminator the C reader waits for was never emitted,
;; and maxinit()'s handshake hung forever.
;;
;; So install main-prompt directly, at top level, exactly as the running
;; REPL will call it each cycle. Two deliberate details:
;;   * "~&" (fresh-line), not a hardcoded newline: a scalar result frame
;;     ends with its payload mid-line, and src/c/donnees.c drains the
;;     byte-exact 6-byte terminator "\n<EO>\n" after the payload -- ~&
;;     supplies that leading newline after a payload while not doubling it
;;     when output is already at column 0 (verified byte-for-byte against
;;     recupResult()'s framing arithmetic in an isolated pipe run).
;;   * (finish-output): THIS is the fix for the handshake blocker. SBCL
;;     full-buffers stdout when it is a pipe (only a tty gets line
;;     buffering), so complete responses sat in the child's 4-8KB buffer
;;     forever while the C side blocked in fgets(). Flushing inside
;;     main-prompt pushes each response out at exactly the protocol
;;     boundary (the interpreter only prompts when it is ready for the
;;     next command), which makes the OS-level buffering irrelevant and
;;     needs no pty anywhere -- a previous openpty() attempt on the C side
;;     broke the parent Scilab session's console and was reverted.
;; The prompt string itself is printed here as a side effect and "" is
;; returned for the interpreter's own prompt printing, so the bytes on the
;; wire cannot depend on how (or whether) the caller renders the returned
;; prompt under --very-quiet.
(defun main-prompt ()
  (format t "~&<EO>~%")
  (finish-output *standard-output*)
  "")
(setq $nolabels t)