#lang racket/base

;; Code for Advent of Code 2019, day 2: Program Alarm
;; https://adventofcode.com/2019/day/2

(require racket/list)
(require racket/stream)
(require racket/match)
(require racket/file)
(require racket/string)
(require racket/sequence)
(require rackunit)

(define OPCODE-ADD 1)
(define OPCODE-MUL 2)
(define OPCODE-STOP 99)
(define INSTRUCTION-WIDTH 4)

(define (opcode->fn opcode)
  (case opcode
    [(1) +]
    [(2) *]))

;; Processes the opcode at the start of `truncated-list`
;; and applies its result to `output-list`.
(define (process-opcode lst inx)
  (match (drop lst inx)
    [(list opcode lhs-inx rhs-inx out-inx _ ...)
     (let ([new-value (list ((opcode->fn opcode)
                             (list-ref lst lhs-inx)
                             (list-ref lst rhs-inx)))])
       (append (take lst out-inx)
               new-value
               (drop lst (add1 out-inx))))]))

(define (eval-intcode lst [inx 0])
  (if (eq? OPCODE-STOP (list-ref lst inx))
      lst
      (eval-intcode (process-opcode lst inx)
                    (+ inx INSTRUCTION-WIDTH))))

;; Replaces elements 1 and 2 in `lst` with the given `noun` and `verb`.
(define (prepare-program noun verb lst)
  (append
   (list (car lst) noun verb)
   (drop lst 3))) ; 3 being the number of values we replace at the start.

;; Gives the program from my input for a given noun and verb.
(define (program noun verb)
  (prepare-program
   noun verb
   (map string->number
        (string-split
         (string-trim
          (file->string "data/day_2_gravity_assist_input.txt"))
         ","))))

;; Computes the output of my program for a given noun and verb.
(define (program-output noun verb)
  (car (eval-intcode (program noun verb))))

;; Gives a sequence of all possible nouns and verbs [0..99] inclusive.
(define (all-inputs)
  (define possible-input (in-range 100))
  (apply sequence-append
         (sequence->list (sequence-map
                          (λ (noun) (sequence-map
                                     (λ (i) (list noun i))
                                     possible-input))
                          possible-input))))

;; Finds the input noun and verb for a given output value; takes a while to run.
(define (find-input output)
  (stream-first
   (sequence->stream
    (sequence-filter
     (λ (input) (= (apply program-output input) output))
     (all-inputs)))))

(check-equal? (process-opcode '(1 0 0 0) 0) '(2 0 0 0))
(check-equal? (eval-intcode '(2 3 0 3 99))
              '(2 3 0 6 99))
(check-equal? (eval-intcode '(2 4 4 5 99 0))
              '(2 4 4 5 99 9801))
(check-equal? (eval-intcode '(1 1 1 4 99 5 6 0 99))
              '(30 1 1 4 2 5 6 0 99))
(check-equal? (program-output 12 2) 6327510)
; This takes a looong time to run (a few minutes).
(check-equal? (find-input 19690720) '(41 12))